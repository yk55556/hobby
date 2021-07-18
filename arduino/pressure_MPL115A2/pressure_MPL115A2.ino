/****************************************************************
  Balometer  -  V.01
    Initial version V.01 on May.27,2021   (c) Yoshiji Kanamoto
  Function:
    Measure air pressure(hPa) and calculate rough altitude.
    Using sensor is MPL115A2.
    Source code is widely cited from the following website.
    http://zattouka.net/GarageHouse/micon/Arduino/Pressure/Pressure2.htm
  Pin connections for MPL115A1 
      P1:VDD
      P2:CAP  1uF to GND !!!!! MANDATORY for keeping precision !!!!! 
      P3:GND
      P4:SHDN VDD        !!!!! MANDATORY !!!!!
      P5:RST  VDD        !!!!! MANDATORY !!!!!
      P6:NC
      P7:SDA  A4 (and PULLUP 4.7k)
      P8:SCL  A5 (and PULLUP 4.7k)
  Revisions:
    V01: Initial version 2021/05/13
  Remarks:
    No restriction related to copyright and modification.
  **************************************************************/

/************************
  UV intensity meter.
************************/


#include <Wire.h>

#define SENSOR_ADRS 0x60                  // MPL115A2のI2Cアドレス
#define AVE_NUM     20                    // 圧力・温度のＡ／Ｄ変換値を平均化する回数
#define H_CORRECT   80                    // 自宅でのセンサと実際の高度差補正値(My自宅の標高は100m)

float a0 , b1 , b2 , c12 ;                // 係数のデータを保存する変数
unsigned long Press , Temp ;              // 圧力および温度の変換値を保存する変数

void setup()
{
     // シリアルモニターの設定
     Serial.begin(9600) ;
     // Ｉ２Ｃの初期化
     Wire.begin() ;                       // マスターとする

     delay(3000) ;                        // 3Sしたら開始
     CoefficientRead() ;                  // メモリーマップから係数を読み出して置く
}
void loop()
{
     int i ;
     float ans ;
     unsigned long p , t ;

     p = t = 0 ;
     for (i=0 ; i < AVE_NUM ; i++) {      // ２０回読み込んで平均化する
          PressureRead() ;                // メモリーマップから圧力および温度のＡ／Ｄ変換値を読み出す
          p = p + Press ;
          t = t + Temp ;
     }
     Press = p / AVE_NUM ;
     Temp  = t / AVE_NUM ;

     ans = PressureCalc() ;               // 気圧値の計算を行う
     Serial.print(ans) ;                  // 気圧値の表示を行う
     Serial.print(" hPa    ") ;
     ans = AltitudeCalc(ans,H_CORRECT) ;  // 高度の計算を行う
     Serial.print(ans) ;                  // 高度の表示を行う
     Serial.println(" m") ;

     delay(1000) ;                        // １秒後に繰り返す
}
// メモリーマップから係数を読み出す処理
int CoefficientRead()
{
     int ans ;
     unsigned int h , l ;

     Wire.beginTransmission(SENSOR_ADRS) ;        // 通信の開始
     Wire.write(0x04) ;                           // 係数の読出しコマンド発行
     ans = Wire.endTransmission() ;               // データの送信と通信の終了
     if (ans == 0) {
          ans = Wire.requestFrom(SENSOR_ADRS,8) ; // 係数データの受信を行う
          if (ans == 8) {
               // ａ０の係数を得る
               h = Wire.read() ;
               l = Wire.read() ;
               a0 = (h << 5) + (l >> 3) + (l & 0x07) / 8.0 ;
               // ｂ１の係数を得る
               h = Wire.read() ;
               l = Wire.read() ;
               b1 = ( ( ( (h & 0x1F) * 0x100 ) + l ) / 8192.0 ) - 3 ;
               // ｂ２の係数を得る
               h = Wire.read() ;
               l = Wire.read() ;
               b2 = ( ( ( ( h - 0x80) << 8 ) + l ) / 16384.0 ) - 2 ;
               // Ｃ１２の係数を得る
               h = Wire.read() ;
               l = Wire.read() ;
               c12 = ( ( ( h * 0x100 ) + l ) / 16777216.0 )  ;
               ans = 0 ;
          } else ans = 5 ;
     }

     return ans ;
}
// メモリーマップから圧力および温度のＡ／Ｄ変換値を読み出す処理
int PressureRead()
{
     int ans ;
     unsigned int h , l ;

     // 圧力および温度の変換を開始させる処理
     Wire.beginTransmission(SENSOR_ADRS) ;        // 通信の開始
     Wire.write(0x12) ;                           // 圧力・温度の変換開始コマンド発行
     Wire.write(0x01) ;
     ans = Wire.endTransmission() ;               // データの送信と通信の終了
     if (ans != 0) return ans ;
     delay(3) ;                                   // 変換完了まで３ｍｓ待つ

     // Ａ／Ｄ変換値を得る処理
     Wire.beginTransmission(SENSOR_ADRS) ;        // 通信の開始
     Wire.write(0x00) ;                           // 圧力のHighバイトから読込むコマンド発行
     ans = Wire.endTransmission() ;               // データの送信と通信の終了
     if (ans == 0) {
          ans = Wire.requestFrom(SENSOR_ADRS,4) ; // Ａ／Ｄ変換値データの受信を行う
          if (ans == 4) {
               // 圧力のＡ／Ｄ変換値を得る
               h = Wire.read() ;
               l = Wire.read() ;
               Press = ( ( h * 256 ) + l ) / 64 ;
               // 温度のＡ／Ｄ変換値を得る
               h = Wire.read() ;
               l = Wire.read() ;
               Temp = ( ( h * 256 ) + l ) / 64 ;
               ans = 0 ;
          } else ans = 5 ;
     }

     return ans ;
}
// 気圧値(hPa)を計算する処理
float PressureCalc()
{
     float ret , f ;

     f = a0 + ( b1 + c12 * Temp ) * Press + b2 * Temp ;
     ret = f * ( 650.0 / 1023.0 ) + 500.0 ;
     return ret ;
}
// 気圧値(hPa)から高度を計算する処理
float AltitudeCalc(float pressure,int Difference)
{
     float h ;

     h = 44330.8 * (1.0 - pow( (pressure/1013.25) ,  0.190263 )) ;
     h = h + Difference ;
     return h ;
}
