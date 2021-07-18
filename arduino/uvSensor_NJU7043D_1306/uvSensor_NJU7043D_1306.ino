/****************************************************************
  UV meter  -  V.01
    Initial version V.01 on May.13,2021   (c) Yoshiji Kanamoto
  Function:
    Measure UV energy in mW/cm2(lambda=365nm, 25C) scale. See below for details.
    https://www.data.jma.go.jp/gmd/env/uvhp/3-50uvindex_manual.html
  Pin connections:
    Arduino:
      A1: Input voltage signal from UV sensor(4th pin)
      A4: SDA for I2C LCD(Uno)
      A5: SCL for I2C LCD(Uno)
      D2: SDA for I2C LCD(nano)
      D3: SCL for I2C LCD(nano)
  Revisions:
    V01: Initial version 2021/05/13
  Remarks:
    No restriction related to copyright and modification.
  **************************************************************/

/************************
  UV intensity meter.
************************/

#include <Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4

String uv_strength_str = "";        // L:low(0-3), M(3-6), H(6-8), VH(8-11), DANGER(11-)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*
   convert voltaaage to UV intensity value(mW/cm2).
*/
float convertToUVintensity(float y) {
  /* from data sheet
     y=ax+b, x=(y-b)/a
     a: 15.0/(2.8-1.0)
     b: 1.0 */
  float a = (2.8 - 1.0) / 15;
  float b = 1.0;
  float x = (y - b) / a;
  return x;
}

void getUVString(float uvLevel) {
  if (uvLevel < 3) {  // Low level
    uv_strength_str = "LOW  ";
  }
  else if (3 <= uvLevel && uvLevel < 6) {
    uv_strength_str = "MID   ";
  }
  else if (6 <= uvLevel && uvLevel < 8) {
    uv_strength_str = "HIGH  ";
  }
  else if (8 <= uvLevel && uvLevel < 11) {
    uv_strength_str = "V-HIGH";
  }
  else if (11 <= uvLevel) {
    uv_strength_str = "DANGER";
  }
  else {
    uv_strength_str = " undef";
  }
}


void setup() {
  Serial.begin(112500);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}


void loop() {
  display.clearDisplay();
    display.setTextSize(2);
  // 出力する文字の色
  display.setTextColor(WHITE);
  //  display.setCursor(0,1);
  
  int val = analogRead(1);                        // read out value of UV signal in analog scale(0-1023)
  float voltage = 5.0 * val / 1023;;              // calculated voltage value(0-5V)
  float uvLevel = convertToUVintensity(voltage);  // intensity value of UV
  String uv_str1 = "";                            // display string of UV information(1st line)
  String uv_str2 = "";                            // display string of UV information(2nd line)
  
  Serial.print("analog val: " + (String)val);
  Serial.print(" Volgate: " + (String)voltage);
  Serial.println(" UV intensity: " + (String)uvLevel);

  getUVString(uvLevel);

  //uv_str1 = "S: " + (String)val + "   V: " + (String)voltage;
  uv_str1 = "E: " + (String)voltage;
  //uv_str2 = "U: " + (String)uvLevel + uv_strength_str;
  uv_str2 = "U: " + (String)uvLevel;

  display.setCursor(0, 0);
  display.println(uv_str1);
  //display.setCursor(0,16);
  display.println(uv_str2);
  display.println("S : " + uv_strength_str);
  display.display();

  delay(10000);

}
