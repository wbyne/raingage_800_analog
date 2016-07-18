//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2015-12-13 21:36:49

#include "Arduino.h"
#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"
#include "Wire.h"
#include "Time.h"
#include "DS3232RTC.h"
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "rain_gauge.h"
void setup() ;
void Tip_Int_Handler() ;
void Alarm_Int_Handler() ;
void Send_The_Tip_Count(int RepCnt) ;
void Send_The_Daily_Message() ;
void sleepNow() ;
void sendAllSMS() ;
void turnOnFONA() ;
void turnOffFONA() ;
void delayFONA() ;
void statusMessage() ;
void milsDelay(unsigned int mils)  ;
void syncTime() ;
void Read_the_Analogs(int RepCnt) ;
void Figure_the_Time(int RepCnt) ;
int freeRam ()     ;
void loop() ;


#include "raingage_800_analog.ino"
