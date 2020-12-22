#include <LiquidCrystal.h>

//constants and global variables
  //LCD
    const int rs = 12, en = 7, d4 = 5, d5 = 4, d6 = 8, d7 = 2;
    const int lcd_red=3, lcd_green=11, lcd_blue=6;
    const int n_r=123, n_g=123, n_b=123;
    const int o_r=0, o_g=255, o_b=0;
    const int c_r=255, c_g=123, c_b=0;
    const int w_r=255, w_g=0, w_b=0;
    enum farba {neutral, ok, caution, warning};
    LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
  //MQ7
    int MQ7_output = A1, sensorValue = 0, MQ7_control=9;
  //IR and RED measurment
    int T=20, sensorPin=A0, IR_LED=10, RED_LED=13;
    const int max_measurments_size=50, min_inserted_finger=200;
    float RED_measurments[max_measurments_size], IR_measurments[max_measurments_size];
    int index=0, measurments_size=0;
  //BPM
    float sum_bpms=0, last_peak=0;
    const int max_bpms=5, avg_const=4, diff=0.23, min_diff_between_peak=300;
    int bpms=0, index_bpm=0;
    float bpm[max_bpms];
  //R and SPO2
    float REDmin=30000, REDmax=0, IRmin=30000, IRmax=0, R=0, sum_Rs=0;
    float Rs[max_bpms];
//end constants and global variables

float measure(int light_port, int input_port);
void write_measurment(float IR, float RED);
float compute_BPM();
bool is_peak();
float avg_ago(int index);
void change_max_min_for_R(float RED, float IR);
void lcd_backlight(int r, int g, int b);
void lcd_backlight(enum farba);

void setup() {
  Serial.begin(9600);
  Serial.flush();
  Serial.println("beat, R, SPO2, BPM");
  
  pinMode(sensorPin,INPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(IR_LED,OUTPUT);
  
  pinMode(MQ7_control, OUTPUT);

  pinMode(en, OUTPUT);
  pinMode(lcd_red, OUTPUT);
  pinMode(lcd_green, OUTPUT);
  pinMode(lcd_blue, OUTPUT);

  lcd_backlight(neutral);
  
  digitalWrite(RED_LED,LOW);
  digitalWrite(IR_LED,LOW);  

  lcd.begin(16, 2);

  for(int i=0;i<max_bpms;i++)
    bpm[i]=Rs[i]=0;
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
  float prev=avg_ago(index-1);
  float prevprev=avg_ago(index-2);
  float last=avg_ago(index-4);
  
  if(last+diff<prevprev && prevprev+diff<prev && prev>now+diff)
    return true;
  return false;
}

float avg_ago(int index)
{
  float res=0;
  for(int i=avg_const-1;i>=0;i--)
  {
    res+=RED_measurments[(index-i+max_measurments_size)%max_measurments_size];
    res+=IR_measurments[(index-i+max_measurments_size)%max_measurments_size];
  }
  return res/(2*avg_const);
}

float compute_BPM()
{
  if(is_peak() && millis()- last_peak>min_diff_between_peak)
  {
    Serial.print(100);
    sum_bpms-=bpm[index_bpm];
    sum_Rs-=Rs[index_bpm];
    
    sum_bpms+=(bpm[index_bpm]=millis()- last_peak);
    sum_Rs+=(Rs[index_bpm] = (((REDmax-REDmin)*IRmin) / ((IRmax-IRmin)*REDmin)));

    REDmin=30000, REDmax=0, IRmin=30000, IRmax=0;    
    last_peak=millis();
    
    index_bpm++;
    index_bpm%=max_bpms; 
  }
  else
    Serial.print(0);

  if(sum_Rs/max_bpms>=92)
    lcd_backlight(ok);
  if(sum_Rs/max_bpms<=88)
    lcd_backlight(warning);
  else if(sum_Rs/max_bpms<92)
    lcd_backlight(caution);


  Serial.print(",");
  Serial.print(sum_Rs/max_bpms);
  Serial.print(",");
  Serial.print(-19*(sum_Rs/max_bpms)+112);
  Serial.print(",");
  Serial.print(60000/(sum_bpms/max_bpms));
}

void change_max_min_for_R(float RED, float IR)
{
  REDmax=max(RED, REDmax);
  REDmin=min(RED, REDmin);
  IRmax=max(IR, IRmax);
  IRmin=min(IR, IRmin);
}

void lcd_backlight(int r, int g, int b)
{
  analogWrite(lcd_red, r);
  analogWrite(lcd_green, g);
  analogWrite(lcd_blue, b);
}
void lcd_backlight(farba f)
{
  if(f==neutral)
    lcd_backlight(n_r, n_g, n_b);
  if(f==ok)
    lcd_backlight(o_r, o_g, o_b);
  if(f==caution)
    lcd_backlight(c_r, c_g, c_b);
  if(f==warning)
    lcd_backlight(w_r, w_g, w_b);
}
