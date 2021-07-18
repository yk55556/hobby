
/****************************************************************
  CO2 Monitor(interrupt mode) -  V.01
    Initial version V.01 on Jun.30,2021   (c) Yoshiji Kanamoto
  Function:
    Measure CO2 density using MH Z19C sensor.
    Density signal is red as a PWM value, converted into PPM.
    Original information is from the following site(Thank you!)

    This skeetch measures CO2 dnsity by interrupt mode.
    Measuring procedure does not occupy entire main loop,
    simply can be used as a function call !
    // https://qiita.com/kaz19610303/items/2dbfe18e248a503fb203
    // http://toccho.xsrv.jp/2018/10/10/esp-wroom02-arduino-mh-z19-pwm/

  Pin connections:
    Arduino:
      A0: CO2 density value as a PWM.
      A4: SDA
      A5: SCL
  Revisions:
    V01: Initial version 2021/06/30

  Usage:
    Activate this sketch, open serial monitor 9600 bps.
    Or activate serial plotter to display graph.
    CO2 values are sampled in every 5 second intervals.
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

/*



*/

#include <MsTimer2.h>
#include <Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

/* Switch on LED on and off each half second */

#define LedPin 13                               /* LED to pin 13 */
#define pwmPin A0
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4

/* CO2 data input to pin A0 */

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
boolean  prevVal = LOW;

uint32_t CO2_data = 0L;
uint8_t  data_count = 0L;


uint32_t th, tl, h, l, ppm, ppm_now;

uint32_t ppm_befor = 0L;
uint32_t tt = 0L;



void flash()
{
  uint8_t myVal = digitalRead(pwmPin);

  tt++ ;
  if (myVal == HIGH) {
    digitalWrite(LedPin, HIGH);
    if (myVal != prevVal) {
      tl = tt ;
      prevVal = myVal;
      tt = 0L;
    }
  }  else {
    digitalWrite(LedPin, LOW);
    if (myVal != prevVal) {
      th = tt;
      prevVal = myVal;
      ppm = 5000 * (th - 2) / (th + tl - 4);         // 取得したデータを二酸化炭素データ(ppm)に変換
      ppm_now = ppm_befor * 0.8 + ppm * 0.2 ;        // 差分方程式のディジタルフィルタ処理
      ppm_befor = ppm_now;
      data_count++ ;
      CO2_data += ppm_now;
      tt = 0L;
    }
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(pwmPin, INPUT);
  pinMode(LedPin, OUTPUT);
  Wire.begin();




  MsTimer2::set(1, flash);                            // 1ms毎に割り込みを発生させる
  MsTimer2::start();
  delay(5000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void loop()
{
  uint32_t    CO2_data_ave;
  CO2_data_ave = CO2_data / data_count;               // 取得した回数分の平均
  // ppm: 即値、ppm_now: フィルタした値、CO2_data_ave: サンプリングした値の平均値
  Serial.println("PPM = " + String(CO2_data_ave) + "," + String(ppm_now) + "," + String(ppm));


  display.clearDisplay();
  display.setTextSize(2);
  // 出力する文字の色
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("CO2 METER");
  display.println("PPM: " + (String)ppm);
  display.display();

  CO2_data = 0L;
  data_count = 0L;

  delay(5000);
}
