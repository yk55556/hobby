/****************************************************************
  CO2 Monitor -  V.01
    Initial version V.01 on Jun.2,2021   (c) Yoshiji Kanamoto
  Function:
    Measure CO2 density using MH Z19C sensor.
    Density signal is red as a PWM value, converted into PPM.
    Original information is from the following site(Thank you!)
    // http://toccho.xsrv.jp/2018/10/10/esp-wroom02-arduino-mh-z19-pwm/

  Pin connections:
    Arduino:
      A0: CO2 density value as a PWM.
      A4: SDA
      A5: SCL
  Revisions:
    V01: Initial version 2021/06/02

  Usage:
    Activate this sketch, open serial monitor 9600 bps.
    Or activate serial plotter to display graph.
    CO2 values are sampled in every 10 second intervals.
  Note:
    Other MH Z19 sensor variants have not been tested.

  Info:
    http://toccho.xsrv.jp/2018/05/05/esp-wroom02-mh-z19-measurement/
    Relationship between CO2 density and environmental coditions.
    350-500ppm    outside, clean air
    500-1000ppm   indoor, noamal condition
    1000-2000ppm  indoor, bad air circulation
    2000-2500ppm  someone start to feel sleepy
    2500-3000ppm  someone start to feel shoulder pain, headache
    3000ppm-      loose concentration
    
  Remarks:
    No restriction related to copyright and modification.
  **************************************************************/

#include <Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

#define pwmPin A0        // UV voltage output value
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int prevVal = LOW;
int i=0;
 
long th, tl, h, l, ppm;
 
void setup() {
  Serial.begin(9600);
  pinMode(pwmPin, INPUT);
  Wire.begin();
  delay(3000);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

 
void loop() {

//  delay(500);
  long tt = millis();
 
  //電圧を取得
  int myVal = digitalRead(pwmPin);
 
  //パルス長の計測
  if (myVal == HIGH) {
    if (myVal != prevVal) {
      h = tt;
      tl = h - l;
      prevVal = myVal;
    }
  }  else {
    if (myVal != prevVal) {
      l = tt;
      th = l - h;
      prevVal = myVal;
      ppm = 5000 * (th - 2) / (th + tl - 4);

      if(i==10){ 
        Serial.println("PPM:" + String(ppm));
        Serial.println("BORDER: " + String(600));

        display.clearDisplay();
        display.setTextSize(2);
        // 出力する文字の色
        display.setTextColor(WHITE);
        
        display.setCursor(0, 0);
        display.println("CO2 METER");
        display.println("PPM: " +(String)ppm);
        display.display();
        i=0;
        
      }else{
        i++;
      }

      
    }
  }

}
