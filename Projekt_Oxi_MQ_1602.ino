float measure(int light_port, int input_port);
void write_measurment(int IR, int RED);
float compute_BPM();
bool is_peak();
float avg_ago(int index);

//constants and global variables
int T=20, sensorPin=A0, IR_LED=10, RED_LED=13;
const int max_measurments_size=50;
float RED_measurments[max_measurments_size], IR_measurments[max_measurments_size];
int index=0, measurments_size=0;
float sum_bpms=0, last_peak=0;
const int max_bpms=5, avg_const=3;
  int diff=0.4;
int bpms=0, index_bpm=0;
float bpm[max_bpms]={0,0,0,0,0};
//end constants and global variables

void setup() {
  Serial.begin(9600);
  Serial.flush();
  //Serial.println("IR, RED");
  
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
  write_measurment(IR, RED);

  Serial.print(IR);
  Serial.print(",");
  Serial.print(RED);
  Serial.print(",");

  
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

void write_measurment(int IR, int RED)
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
  //return true;
  /*float now =(RED_measurments[(index-0+max_measurments_size)%max_measurments_size]+RED_measurments[(index-1+max_measurments_size)%max_measurments_size]);
  float prev=(RED_measurments[(index-1+max_measurments_size)%max_measurments_size]+RED_measurments[(index-2+max_measurments_size)%max_measurments_size]);
  float last=(RED_measurments[(index-2+max_measurments_size)%max_measurments_size]+RED_measurments[(index-3+max_measurments_size)%max_measurments_size]);*/
  float now =avg_ago(index);
  float prev=avg_ago(index-1);
  float last=avg_ago(index-2);
  Serial.print(now);
  Serial.print(",");
  if(last<prev && last>now && last+diff<=prev && prev>=now+diff)
  {
    Serial.print(760);
    return true;
  }
  Serial.print(725);
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
  if(is_peak())
  {
    sum_bpms-=bpm[index_bpm];
    sum_bpms+=bpm[index_bpm]=millis()- last_peak;
    last_peak=millis();
    index_bpm++;
    index_bpm%=max_bpms;
    for(int i=0;i<max_bpms;i++)
    {
      //Serial.print(bpm[i]);
      //Serial.print(",");
    }
    //Serial.println();
  }
}
