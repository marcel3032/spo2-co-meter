float measure(int light_port, int input_port);
void write_measurment(float IR, float RED);
float compute_BPM();
bool is_peak();
float avg_ago(int index);
void change_max_min_for_R(float RED, float IR);

//constants and global variables
int T=20, sensorPin=A0, IR_LED=10, RED_LED=13;
const int max_measurments_size=50, min_inserted_finger=200;
float RED_measurments[max_measurments_size], IR_measurments[max_measurments_size];
int index=0, measurments_size=0;
float sum_bpms=0, last_peak=0;
const int max_bpms=5, avg_const=3, diff=0.3, min_diff_between_peak=200;
int bpms=0, index_bpm=0;
float bpm[max_bpms]={0,0,0,0,0};
float REDmin=30000, REDmax=0, IRmin=30000, IRmax=0, R=0;
//end constants and global variables

void setup() {
  Serial.begin(9600);
  Serial.flush();
  Serial.println("beat, R, SPO2, BPM");
  
  pinMode(sensorPin,INPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(IR_LED,OUTPUT);

  digitalWrite(RED_LED,LOW);
  digitalWrite(IR_LED,LOW);  
}

void loop() {
  float s=millis();
  float IR = measure(IR_LED, sensorPin);
  float RED = measure(RED_LED, sensorPin);
  if(IR<min_inserted_finger || RED<min_inserted_finger)
    return;
  write_measurment(IR, RED);

  change_max_min_for_R(RED, IR);

  
  if(measurments_size==max_measurments_size)
  {
    compute_BPM();
    
  }
      Serial.println();
}

float measure(int light_port, int input_port)
{
  digitalWrite(light_port, HIGH);
  float start=millis();
  double res=0;
  int n=0;
  while(millis()<start+T)
  {
    res+=analogRead(input_port);
    n++;
  }
  digitalWrite(light_port, LOW);
  return res/n;
}

void write_measurment(float IR, float RED)
{
  RED_measurments[index]=RED;
  IR_measurments[index]=IR;
  index++;
  
  if(measurments_size<max_measurments_size)
    measurments_size++;
  index%=max_measurments_size;
}

bool is_peak()
{
  float now =avg_ago(index);
  float prev=avg_ago(index-3);
  float last=avg_ago(index-5);
  //Serial.print(now);
  //Serial.print(",");
  if(last<prev && last>now && last+diff<=prev && prev>=now+diff)
  {
    Serial.print(100);
    return true;
  }
  Serial.print(0);
  return false;
}

float avg_ago(int index)
{
  float res=0;
  for(int i=avg_const-1;i>=0;i--)
  {
    res+=RED_measurments[(index-i+max_measurments_size)%max_measurments_size];
    //res+=IR_measurments[(index-i+max_measurments_size)%max_measurments_size];
  }
  return res/(avg_const);
}

float compute_BPM()
{
  float avg=0;
  if(is_peak() && millis()- last_peak>min_diff_between_peak)
  {
    sum_bpms-=bpm[index_bpm];
    sum_bpms+=bpm[index_bpm]=millis()- last_peak;
    last_peak=millis();
    index_bpm++;
    index_bpm%=max_bpms; 
    R=(((REDmax-REDmin)*IRmin) / ((IRmax-IRmin)*REDmin));
    REDmin=30000, REDmax=0, IRmin=30000, IRmax=0;
  }
  for(int i=0;i<max_bpms;i++)
    {
      avg+=bpm[i];
    }
  Serial.print(",");
  Serial.print(R);
  Serial.print(",");
  Serial.print(-19*R+112);
  Serial.print(",");
  Serial.print(60000/(avg/max_bpms));
  /*Serial.print(",");
  Serial.print((RED_measurments[index]-850)*5);
  Serial.print(",");
  Serial.print((IR_measurments[index]-850)*5);*/
}

void change_max_min_for_R(float RED, float IR)
{
  REDmax=max(RED, REDmax);
  REDmin=min(RED, REDmin);
  IRmax=max(IR, IRmax);
  IRmin=min(IR, IRmin);
}
