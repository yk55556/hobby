
/****************************************************************
  RGB LED light -  V.01
    Initial version V.01 on Jul.22,2021   (c) Yoshiji Kanamoto
  Function:
    Simple LED light

  Pin connections:
    Arduino:
      3: Red
      5: Green
      6: Blue
      
  Revisions:
    V01: Initial version 2021/07/22

  Usage:
    Connect 5V.
    
  Note:
    Nothing.

  Remarks:
    Nothing.
  **************************************************************/


// for arduino Uno pin 
#define RED   3
#define GREEN 5
#define BLUE  6

#define BRT   8   // brightness change.
#define BMAX  9   // brightness to max
#define BMIN 10   // brightness to min
const int delayTime= 1000;
float brightness= 0.1;
float r,g,b;

// frequency factor
float tr=5;
float tg=7;
float tb=11;

// phase shift factor
float pr=0;
float pg=0.6;
float pb=1.2;

float t=0;

int f_pressed= HIGH;
int f_direction=1;


void setup() {
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BRT, INPUT_PULLUP);
  pinMode(BMAX, INPUT_PULLUP);
  pinMode(BMIN, INPUT_PULLUP);

  Serial.begin(115200);
}


void loop() {
  r=sin(t*tr+pr)*0.5+0.5;
  g=sin(t*tg+pg)*0.5+0.5;
  b=sin(t*tb+pb)*0.5+0.5;

  // normalize each color value
  float v=r+g+b;
  r= r/v;
  g= g/v;
  b= b/v;
  
  analogWrite(RED,   r * 255 * brightness);
  analogWrite(GREEN, g * 255 * brightness);
  analogWrite(BLUE,  b * 255 * brightness);
  
  t=t+0.001;

  f_pressed=!digitalRead(BRT);
  if(f_pressed){
    //brightness= brightness+0.05*f_direction;
    if(brightness >= 1){
      f_direction= -1;
    }else if(brightness<= 0.1){
      f_direction= 1;
    }
    brightness= brightness+0.05*f_direction;
  }


  if(!digitalRead(BMAX)){
    brightness=1.0;
  }
  
  if(!digitalRead(BMIN)){
    brightness=0.1;
  }
  
  delay(delayTime);    
  //Serial.println(String(r) +"," + String(g) + "," + String(b));
}
