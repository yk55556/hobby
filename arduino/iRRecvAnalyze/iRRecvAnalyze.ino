/****************************************************************
  IR signal analyzer -  V.01
    Initial version V.01 on May.28,2021   (c) Yoshiji Kanamoto
  Function:
    Receuve IR signal, decode into RAW format and HEX format.
    https://qiita.com/dokkozo/items/25c6e17fcc2e655d5d42

    RAW signal can be send and control successfully using:
    https://iot-gym.com/how-to-send-ir-signals-by-using-m5atom/
    https://kyonta1022.hatenablog.com/entry/2019/10/24/002919

  Pin connections:
    Arduino:
      D2: IR Sensor IN
  Revisions:
    V01: Initial version 2021/05/28

  Usage:
    Activate this sketch, open serial monitor 115200 bps.
    Send IR signal to sensor, wait several second.
    Confirm output of serial monitor.
  Note:
    This sketch is clever enough to discard noise signal.
    For this purpose displaying analyzed result takes several seconds.
    Please wait to get analyzed result on serial monitor.
  CAUTION:
    Please do not turn off TV when using this program.
    TV emits lots of noisy IR light. This deteriorates recgonition rates.
    
  Remarks:
    No restriction related to copyright and modification.
  **************************************************************/

#define maxLen 800 // 信号の最大長
//#define maxLen 600
#include <stdio.h>

// リモコン信号を読むためのコード
// 参考 https://gist.github.com/chaeplin/a3a4b4b6b887c663bfe8

// 受光モジュールはGPIO2に接続
// 受信内容はシリアルモニタで確認

volatile unsigned int irBuffer[maxLen]; // stores timings - volatile because changed by ISR
volatile unsigned int x = 0; // 入ってきた信号の長さ Pointer thru irBuffer - volatile because changed by ISR

void setup() {
  Serial.begin(115200);
  attachInterrupt(0, rxIR_Interrupt_Handler, CHANGE);
}

void loop() {
  delay(5000);
  if (x) { // same as if x >= 1
    detachInterrupt(0);
    for (int i = 1; i < x; i++) {
      irBuffer[i - 1] = irBuffer[i] - irBuffer[i - 1];
    }
    x--;

    // 9000usより長い信号があった場合は, その次からデコードする
    // たとえばエアコンのリモコンの場合, 13000usのインターバルをはさんで2回信号が送信されているが, 後半だけあればエアコンを操作できるようなので
    // 前半は表示しない. 
    int startpoint = 0;
    for (int i = 0; i < x; i++) {
      if(irBuffer[i]>9000){
        startpoint = i+1;
      }
    }

    if (x - startpoint < 30){ //短すぎる信号はノイズなのでカット
      attachInterrupt(0, rxIR_Interrupt_Handler, CHANGE);
      return;
    }

    //raw array
    Serial.print(F("Raw: ("));
    Serial.print((x-startpoint));
    Serial.print(F(")"));
    Serial.print(F("["));
    for (int i = startpoint; i < x; i++) {
      Serial.print(irBuffer[i]);
      Serial.print(F(", "));
    }
    Serial.println(F("]"));

    // hex array. 8T 4T { } T
    Serial.print(F("hex: ("));
    Serial.print((x - startpoint - 3)/2); // length of signal [bit]
    Serial.print(F(") {"));

    int cnt = 0; // digit
    int hexval = 0; // sum of 8digits C_0 C_1 ... C_7

    for (int i = startpoint + 2; i < x - 1; i++) {
      if ((i - startpoint)%2 == 1){
        if (irBuffer[i]> 900){ // T=450usならおよそ2Tを閾値としていることになる
          //Serial.print(1);
          hexval += 1 << cnt;
        }else{
          //Serial.print(0);
        }
        cnt++;
        if (cnt == 8){
          if(hexval < 16){ // padding
            Serial.print(0);
          }
          Serial.print(hexval, HEX);
          Serial.print(F(" "));
          cnt = 0;
          hexval = 0;
        }
      }
    }
    x = 0;
    Serial.println(F("}"));

    attachInterrupt(0, rxIR_Interrupt_Handler, CHANGE);
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void rxIR_Interrupt_Handler() {
  if (x > maxLen) return; //ignore if irBuffer is already full
  irBuffer[x++] = micros(); //just continually record the time-stamp of signal transitions
}
