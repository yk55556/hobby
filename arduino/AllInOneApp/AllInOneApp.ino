/****************************************************************
  All in One app Proto V.01
    Initial version V.01 on June 27, 2021  (C) Yoshiji Kanamoto
  Function:
    Multiple controller including:
    - 2.8 inch SPI LCD panel for display
    - iR remote controller sender
    - iR remote controller receiver(under construction)
    - CO2 monitor
    - balometer-thermometer

  Pin connections:
    LCD:
      A5:        CS
      A4:        RESET
      A3         DC
      MOSI(SDI): D11
      SCK:       D13
      LED:       A0     // FIXME this is not used. -1 and connect gnd or vcc?
      MISO(SDO): D12
//            T_IRQ  T_DO  T_DIN  T_CS  T_CLK
//Arduino Uno  6      4      5     2      3->7 beause of iRRemote use D3!
      
    iR Send
      D3:        send signal
    BME280
      CSB        D10
    MH-Z18C
      DATAIN     A1

   websites:
     Good example of LCDWIKI
       https://a-tomi.hatenablog.com/entry/2021/05/24/105516

  Revisions:
    V01: Initial version 2021/06/30
  Remarks:
    No restriction related to copyright and modification.
  **************************************************************/

#include <Arduino.h>

// LCD
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <LCDWIKI_TOUCH.h> //touch screen library

// IrRemote
#include "PinDefinitionsAndMore.h"
#include <IRremote.h>

// BME280
#include "SparkFunBME280.h"

// MHZ19C
#include <MsTimer2.h> // for interrupt mode

////////////////////////////////////////////////////////////
//paramters define

// LCD controller
#define MODEL ILI9341

// PIN assign for LCD
#define CS   A5
#define CD   A3
#define RST  A4
#define LED  A0   //if you don't need to control the LED pin,
// you should set it to -1 and set led pin of "LCD" to 3.3V.
// NOTE: backlight blinks. why?
//touch screen paramters define

#define TCS   2
#define TCLK  7
#define TDOUT 4
#define TDIN  5
#define TIRQ  6

// BME280
#define SPI_CS_PIN  10

// MHZ19C
#define pwmPin A1        // UV voltage output value
#define LedPin 13       // just indicator...

////////////////////////////////////////////////////////////

// define some colour values
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// irRemote command definitions
#define TV_ADDR     0x2684
#define TV_ON       0x18
#define TV_UP       0x0F    // volume up
#define TV_DN       0x0B    // volume down
#define TV_BS       0x39    // BS
#define TV_DG       0x38    // Ground digital

#define TV_ST       0xA1    // subtitle in mainly mute mode
#define TV_MT       0x1A    // MUTE

#define TV_C1       0x00
#define TV_C2       0x01
#define TV_C3       0x02
#define TV_C4       0x03
#define TV_C5       0x04
#define TV_C6       0x05
#define TV_C7       0x06
#define TV_C8       0x07
#define TV_C9       0x08

////////////////////////////////////////////////////////////
// instances and variables
LCDWIKI_SPI mylcd(MODEL, CS, CD, RST, LED); //model,cs,dc,reset,led
LCDWIKI_TOUCH my_touch(TCS,TCLK,TDOUT,TDIN,TIRQ); //tcs,tclk,tdout,tdin,tirq
uint16_t wW;
uint16_t wH;
BME280 sensor;
int senseCount = 0;   
int senseCountMax=199;  // 100mm x 200= 20sec for BME280 and CO2 monitor

// for irremote
uint16_t sAddress;
uint8_t sCommand;
uint8_t sRepeats = 0;

//for MHZ19C and CO2 monitor
int prevVal = LOW;
uint32_t th, tl, h, l, ppm, ppm_now;
uint32_t ppm_befor = 0L;
uint32_t tt = 0L;
uint32_t CO2_data = 0L;
uint8_t  data_count = 0L;

// co2 graph drawing coordinate variable
#define CO2MIN 450
#define CO2MAX 1500

// rectangular coordinates
uint16_t co2graph_min_x;
uint16_t co2graph_max_x;
uint16_t co2graph_min_y;
uint16_t co2graph_max_y;

// current bar graph x position
uint16_t currentXpos;

// led is lit: HIGH
int f_pressed= HIGH;
int f_pressed_old= HIGH;
bool f_state=true;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void setup() {

  ////////////////////////////////////////
  // 1. LCDWiki initialization
  // Be careful wW and wH is swapped becase of rotate 90!
  wW = mylcd.Get_Display_Height();
  wH = mylcd.Get_Display_Width();
  mylcd.Init_LCD();
  mylcd.Fill_Screen(BLACK);

  // rotation from 0:0degree, 1:90degree, 3:270degree
  mylcd.Set_Rotation(1); // 90 dgree rotation for landscape displaying

  // fill title bar, because ratete90, wH is used insted of wW
  mylcd.Set_Draw_color(16, 16, 96);

  mylcd.Set_Text_Mode(1);
  mylcd.Set_Text_Size(1);
  mylcd.Set_Text_colour(255, 255, 255);
  mylcd.Set_Text_Back_colour(0);

  // header
  mylcd.Fill_Rectangle(0, 0, wW - 1, 15); // draw header bar
  mylcd.Print_String("Welcome to MM world!", CENTER, 3);

  // hooter
  mylcd.Fill_Rectangle(0, wH - 15, wW - 1, wH - 1); // draw footer
  mylcd.Print_String("(C) Yoshiji Kanamoto, 2021.", CENTER, wH - 12);

  // main part
  mylcd.Set_Text_Mode(0);
  mylcd.Set_Text_colour(YELLOW);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(1);

  // co2 graph area
  mylcd.Set_Draw_color(255,255,255);
  co2graph_min_x= 30;
  co2graph_max_x= wW-1;
  co2graph_min_y= 100;
  co2graph_max_y= wH-20;
  mylcd.Draw_Rectangle(co2graph_min_x, co2graph_min_y, co2graph_max_x, co2graph_max_y); 

  // scale of graph
  uint16_t tmpx, tmpy;
  tmpx= 0;
  tmpy= map(450, CO2MIN, CO2MAX, co2graph_max_y, co2graph_min_y);
  mylcd.Print_String(" 450" , tmpx, tmpy-4);
  tmpy= map(1500, CO2MIN, CO2MAX, co2graph_max_y, co2graph_min_y);
  mylcd.Print_String("1500" , tmpx, tmpy-4);
  tmpy= map(1000, CO2MIN, CO2MAX, co2graph_max_y, co2graph_min_y);
  mylcd.Print_String("1000" , tmpx, tmpy-4);

  currentXpos= co2graph_min_x;

  ////////////////////////////////////////
  // 2. irRemote initialization

  // the followings are dead copy from irremote sendDemo::setup().
  // do not modify until you understand what it is.
  Serial.begin(115200);
  //Serial.begin(9600);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
  delay(4000); // To be able to connect Serial monitor after reset
  // or power up and before first print out.
  // Do not wait for an attached Serial Monitor!
#endif
  // Just to know which program is running on my Arduino
  //  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

  // Specify send pin and enable feedback LED at default feedback LED pin
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);

  //  Serial.print(F("Ready to send IR signals at pin "));
#if defined(ARDUINO_ARCH_STM32) || defined(ESP8266)
  //  Serial.println(IR_SEND_PIN_STRING);
#else
  //  Serial.println(IR_SEND_PIN);
#endif

  // for esp32 we use PWM generation by ledcWrite() for each pin
#if !defined(SEND_PWM_BY_TIMER) && !defined(USE_NO_SEND_PWM) && !defined(ESP32)

  // Print internal signal generation info
  IrSender.enableIROut(38);

  /*
    Serial.print(F("Send signal mark duration is "));
    Serial.print(IrSender.periodOnTimeMicros);
    Serial.print(F(" us, pulse correction is "));
    Serial.print(IrSender.getPulseCorrectionNanos());
    Serial.print(F(" ns, total period is "));
    Serial.print(IrSender.periodTimeMicros);
    Serial.println(F(" us"));
  */
#endif

  ////////////////////////////////////////
  // 3. BME280
  sensor.beginSPI(SPI_CS_PIN);

  ////////////////////////////////////////
  // 4. MHZ19C
  pinMode(pwmPin, INPUT);
  pinMode(LedPin, OUTPUT);
  MsTimer2::set(1, flash);                            // 1ms毎に割り込みを発生させる
  MsTimer2::start();

  // switch for LED
  pinMode(8, INPUT_PULLUP);

} // end of setup


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void loop() {
  irControl();
  
  if(senseCount==senseCountMax){
    senseBME280();
    readCO2Value();
    senseCount=0;
  }else{
    senseCount++;
  }

  f_pressed=digitalRead(8);
  if((f_pressed==LOW) && (f_pressed_old==HIGH)){ //押されていない状態から押された状態の時
    f_state=!f_state;
  }
  f_pressed_old=f_pressed;

  if(f_state==HIGH){
   digitalWrite(LED, HIGH);
  }else{
    digitalWrite(LED,LOW);
  }
  
  
  delay(100);
}

/////////////////////////////////////////////////////////////


void irControl() {
  if (Serial.available() > 0 )     //受信したデータが存在した場合以下を実行
  {
    char val = Serial.read();      //char文字として受信データの読み込み
    switch (val) {
      // TV
      case 'p': IrSender.sendNEC(TV_ADDR, TV_ON, 0); break;
      case 'u': IrSender.sendNEC(TV_ADDR, TV_UP, 0); break;
      case 'd': IrSender.sendNEC(TV_ADDR, TV_DN, 0); break;
      case 'g': IrSender.sendNEC(TV_ADDR, TV_DG, 0); break; // 地デジ
      case 'b': IrSender.sendNEC(TV_ADDR, TV_BS, 0); break; // BS

      case 's': IrSender.sendNEC(TV_ADDR, TV_ST, 0); break; // 字幕
      case 'm': IrSender.sendNEC(TV_ADDR, TV_MT, 0); break; // ミュート

      case '1': IrSender.sendNEC(TV_ADDR, TV_C1, 0); break;
      case '2': IrSender.sendNEC(TV_ADDR, TV_C2, 0); break;
      case '3': IrSender.sendNEC(TV_ADDR, TV_C3, 0); break;
      case '4': IrSender.sendNEC(TV_ADDR, TV_C4, 0); break;
      case '5': IrSender.sendNEC(TV_ADDR, TV_C5, 0); break;
      case '6': IrSender.sendNEC(TV_ADDR, TV_C6, 0); break;
      case '7': IrSender.sendNEC(TV_ADDR, TV_C7, 0); break;
      case '8': IrSender.sendNEC(TV_ADDR, TV_C8, 0); break;
      case '9': IrSender.sendNEC(TV_ADDR, TV_C9, 0); break;

      // DVD Player
      case 'x': IrSender.sendPanasonic(0xb, 0x3d, 0); break;

      // Experimental
      default: ;
    }
  }
}


void senseBME280() {

    float t = sensor.readTempC();
    float h = sensor.readFloatHumidity();
    float p = sensor.readFloatPressure() / 100.0;
    /*
      Serial.print("Temp: ");  Serial.print(t, 2);
      Serial.print(" °C, Humidity: "); Serial.print(h, 2);
      Serial.print(" %, Pressure: ");   Serial.print(p, 1);  Serial.println(" hPa");
    */

    String a = "TEMP: ";
    a.concat(String(t-1.0));    // correction factor -1.0
    //a.concat(String(t));
    mylcd.Print_String(a , 0, 20);
    String b = "HUMD: ";
    b.concat(String(h+5.5));    // correction factor +5.0
    mylcd.Print_String(b , 0, 30);
    String c = "BALO: ";
    c.concat(p);
    mylcd.Print_String(c, 0, 40);
}



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


void readCO2Value() {
    uint32_t    CO2_data_ave;
    CO2_data_ave = CO2_data / data_count;               // 取得した回数分の平均
    // ppm: 即値、ppm_now: フィルタした値、CO2_data_ave: サンプリングした値の平均値
    Serial.println("PPM = " + String(CO2_data_ave) + "," + String(ppm_now) + "," + String(ppm));
    Serial.println(String(600));

    String c = "CO2:  ";
    c.concat(ppm);
    c.concat("  ");
    mylcd.Print_String(c, 0, 50);

    // for graph
    int tmpCO2posInGraph=map(ppm,CO2MIN, CO2MAX, co2graph_max_y, co2graph_min_y);
    if(ppm<=CO2MIN){
      tmpCO2posInGraph=co2graph_max_y-1;
    }else if(ppm>= CO2MAX){
      tmpCO2posInGraph=co2graph_min_y;
    }
    
    mylcd.Set_Draw_color(150,50,35);
    mylcd.Draw_Line(currentXpos+1, co2graph_max_y-1,currentXpos+1, tmpCO2posInGraph);
    mylcd.Set_Draw_color(0,0,0); 
    mylcd.Draw_Line(currentXpos+1, tmpCO2posInGraph-1, currentXpos+1, co2graph_min_y+1);
    
    mylcd.Set_Draw_color(0,0,0);
    
    mylcd.Fill_Triangle(currentXpos-3, co2graph_min_y-6,
      currentXpos+3, co2graph_min_y-6,
      currentXpos, co2graph_min_y-1);
    
    mylcd.Set_Draw_color(255,255,255);
    mylcd.Fill_Triangle(currentXpos-3+1, co2graph_min_y-6,
      currentXpos+3+1, co2graph_min_y-6,
      currentXpos+1, co2graph_min_y-1);
      
    currentXpos++;

    String t="Xpos: ";
    t.concat(currentXpos);
    t.concat("  ");
    mylcd.Print_String(t, 0, 60);
    t="Max X:";
    t.concat(co2graph_max_x);
    t.concat("   ");
    mylcd.Print_String(t, 0, 70);
    
    
    if(currentXpos== co2graph_max_x-1){ // -1 is a point! 2021/7/15
      // erase triangle
      mylcd.Set_Draw_color(0,0,0);
      currentXpos= co2graph_min_x;
      mylcd.Fill_Rectangle(co2graph_max_x-10, co2graph_min_y+1,co2graph_max_x, co2graph_min_y-10);
    }

    CO2_data = 0L;
    data_count = 0L;

}
