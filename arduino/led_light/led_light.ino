
/****************************************************************
  RGB LED light -  V.01
    Initial version V.01 on Jul.22,2021   (c) Yoshiji Kanamoto
  Function:
    Simple LED light

  Pin connections:
    Arduino:
      9: Red
      6: Green
      5: Blue
      
  Revisions:
    V01: Initial version 2021/07/22

  Usage:
    Connect 5V.
    
  Note:
    Nothing.

  Remarks:
    Nothing.
  **************************************************************/

#define BLUE  5
#define GREEN 6
#define RED   9

const int delayTime= 100;
const float brightness= 0.05;
float r,g,b;

// frequency factor
float tr=5;
float tg=11;
float tb=31;

// phase shift
float pr=0;
float pg=0.6;
float pb=1.2;

float t=0;


void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
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
  Serial.println(String(r) +"," + String(g) + "," + String(b));

  delay(delayTime);


}
