/****************************************************************
  IR  remote controller  -  V.01
    Initial version V.01 on Jun.19,2021   (c) Yoshiji Kanamoto
  Function:
    IR signal send/receiver.
      receive:

      send:
        Drive IR LED with 2SC1815
          base resistance
          collector resistance:
  Pin connections:
    Arduino:
      A1: Input voltage signal from UV sensor(4th pin)
      D2: SDA for I2C LCD.
      D3: SCL for I2C LCD.
  Revisions:
    V01: Initial version 2021/05/13
  Remarks:
    No restriction related to copyright and modification.
  **************************************************************/

#include <Arduino.h>

/*
 * Define macros for input and output pin etc.
 */
#include "PinDefinitionsAndMore.h"
#include <IRremote.h>

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


void setup() {
  // the followings are dead copy from irremote sendDemo::setup(). do not modify until you understand what it is.
    //Serial.begin(115200);
    Serial.begin(9600);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin

    Serial.print(F("Ready to send IR signals at pin "));
#if defined(ARDUINO_ARCH_STM32) || defined(ESP8266)
    Serial.println(IR_SEND_PIN_STRING);
#else
    Serial.println(IR_SEND_PIN);
#endif

#if !defined(SEND_PWM_BY_TIMER) && !defined(USE_NO_SEND_PWM) && !defined(ESP32) // for esp32 we use PWM generation by ledcWrite() for each pin
    /*
     * Print internal signal generation info
     */
    IrSender.enableIROut(38);

    Serial.print(F("Send signal mark duration is "));
    Serial.print(IrSender.periodOnTimeMicros);
    Serial.print(F(" us, pulse correction is "));
    Serial.print(IrSender.getPulseCorrectionNanos());
    Serial.print(F(" ns, total period is "));
    Serial.print(IrSender.periodTimeMicros);
    Serial.println(F(" us"));
#endif
}

/*
 * Set up the data to be sent.
 * For most protocols, the data is build up with a constant 8 (or 16 byte) address
 * and a variable 8 bit command.
 * There are exceptions like Sony and Denon, which have 5 bit address.
 */
uint16_t sAddress;
uint8_t sCommand;
uint8_t sRepeats = 0;



void loop() {
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
    delay(100);
}
