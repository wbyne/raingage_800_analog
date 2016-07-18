/*  WRITE THE HEADER AND DESCRIPTION
 * DO THE CALCULATIONS FOR ANALOG
 * IF STATION # > 40, SKIP THE ANALOG EXCEPT 3231 READ
 */
#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"
#include "Wire.h"
#include "Time.h"
#include "DS3232RTC.h"

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include "rain_gauge.h"

#define i2cBegin Wire.begin
#define i2cBeginTransmission Wire.beginTransmission
#define i2cEndTransmission Wire.endTransmission
#define i2cRequestFrom Wire.requestFrom
#define i2cRead Wire.read
#define i2cWrite Wire.write

//static const char STATION[] = "GRU";						// Station ID
unsigned long int STATION = 8000;								// Station ID, unsigned gives us 1-65
static const char PARAMETER[] = "RAIN";						// Parameter in Tip Count Message
static char sendto[] = "7062540102";						// Set the receiving phone number here
static const uint16_t keyTime = 2000;						// keytime is used to power GSM on and off

// Global variable declarations
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);	// Software Serial object named fonaSS
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);				// GSM Module object named fona
DS3232RTC rtc = DS3232RTC();								// Real Time Clock object
volatile int Tip_Count = 0;                                 // Counter for the rain gauge tips
volatile int Tip_Count_Copy[DataPts_to_Store] = {0};                            // Storage to make a copy of the rain gauge tips
volatile bool Set_Interval_Alarm = false;                   // If true, the interval alarm needs to be set
volatile bool Rtc_Alarm = false;							// If true, an RTC alarm (ALARM_1 or ALARM_2) caused an interrupt

tmElements_t tm;											// Time in year, month, day, etc
volatile time_t t;											// Time in seconds since 1970

volatile unsigned long startdelay = 0;						// startdelay = millis(); instead of using delay()
volatile int16_t storedsms = 0;								// Number of SMS messages in storage

//analog globals
int RepCnt=0;
unsigned long DateTimelong[DataPts_to_Store] = {0};
unsigned int rain[DataPts_to_Store] = {0};  //remember arrays are sized by the number but COUNT from 0.
unsigned int light[DataPts_to_Store] = {0};
unsigned int depth[DataPts_to_Store] = {0};
unsigned int temp[DataPts_to_Store] = {0}; //raw reading, not saving.  stored in realtemp as a float
//float realtemp[DataPts_to_Store] = {0.0};
unsigned int temp_RTC[DataPts_to_Store] = {0};
double realtemp = 0.0;

//for watching the freeram
unsigned int freeramresults = 0;



void setup()
{
//	char replybuffer[128] = { 0 }; not used?
	analogReference(EXTERNAL); //reads off the AREF pin to set the upper limit.  Currently around 2.5-3.4 V on the different revs.
	pinMode(DEPTH_READ,INPUT); //A0
	pinMode(LIGHT_READ,INPUT); //A1
	//pinMode(A2_Read,INPUT); //A2, used on the Sim900 for shield power verification
	pinMode(TEMP_READ,INPUT); //A3
    pinMode(DEPTH_PWR,OUTPUT); //D11
	// First, turn off peripheral devices that are not needed and set the GPIO pins to input with pull-down resistors enabled
	// Disable Watchdog and brown-out in fuse settings
/*
	// turn OFF analog parts
	ACSR = ACSR & 0b11110111 ;		// clearing ACIE prevent analog interrupts happening during next command
	ACSR = ACSR | 0b10000000 ;		// set ACD bit powers off analog comparator
	ADCSRA = ADCSRA & 0b01111111 ;	// clearing ADEN turns off analog digital converter
	ADMUX &= B00111111;				// Comparator uses AREF/GND and not internally generated references
*/
	// power_adc_disable();			// ADC converter - ADC was turned off above
	//power_spi_disable();			// SPI
	//power_usart0_disable();		// Serial (USART)
	//power_timer0_disable();		// Timer 0
	//power_timer1_disable();		// Timer 1
	//power_timer2_disable();		// Timer 2 - need timer 2
	// power_twi_disable();			// TWI (I2C)

	// configure gpio as input with pull down
	/*
	for (byte i = 0; i <= A5; i++)
	{
		pinMode (i, INPUT);
		digitalWrite (i, LOW);
	}
	*/

	pinMode(13, OUTPUT);						// For testing only.

	// Configure the FONA pins
	pinMode(FONA_KEY, OUTPUT); //D9
	digitalWrite(FONA_KEY, HIGH);

	pinMode(FONA_PS, INPUT); //D10
	digitalWrite(FONA_PS, HIGH);

	pinMode(REED_SWITCH_PIN, INPUT); //D2           // Set the reed switch pin as input
	digitalWrite(REED_SWITCH_PIN, HIGH);        // Enable the internal pullup resistor on the reed switch pin

	pinMode(PULSE_PER_SECOND_PIN, INPUT); //D3      // Set the real time clock pulse per second pin as input
	digitalWrite(PULSE_PER_SECOND_PIN, HIGH);  // Enable the internal pullup resistor on the pulse per second pin

	turnOnFONA(); //jumps to turn on, then to milsdelay to wait for about 10 seconds

	while (!Serial)									// Ensures a serial connection before moving on.
	{
		;
	}
	Serial.begin(19200); //hardware serial D0,D1
	fonaSS.begin(19200); //fona software serial start. I upped this from 4800 (Adafruits recommendation) based on problems
						 // where the hardware would outrun the software buffer using the stock uno's with Sim900s.
						 // I had no problems with these speeds on the Unos/900's with narcoleptic.

	if(fona.begin(fonaSS)) //I'm not sure what this is checking.  True if it succeeds, false if turning off echo fails.
	{
		;
	}

	delayFONA();										// waits for network registration before continuing

	// Configure the FONA settings
	// A one second pause allows for completion of the command before entering a new one.
	// Two seconds are given for the delete command to complete.  These delays seem to be sufficient
	// but 500 was sometimes not long enough.
	fona.mySerial->println("AT+CMGF=1");				// sets fona to sms mode
	milsDelay(1000);
	fona.mySerial->println("AT+CPMS=\"SM\",\"SM\"");	// sets preferred message storage to sim card
	milsDelay(1000);
	fona.mySerial->println("AT+CLTS=1");				// enables the fona onboard rtc
	milsDelay(1000);
	fona.mySerial->println("AT+CMGD=1,4");				// deletes all stored messages from every memory location
	milsDelay(2000);									// TODO: Check this delay because it may not be long enough
	fona.mySerial->println("AT&W");						// writes current settings to memory so they will be the same when the fona powers down and up
	milsDelay(1000);

	syncTime();
	milsDelay(1000);

	// Attach the Tip interrupt
	attachInterrupt(INTERRUPT_0, Tip_Int_Handler, LOW);

	// Configure the RTC alarms (ALARM_1 and ALARM_2)
	rtc.alarmInterrupt(ALARM_1, false);		// Disable the ALARM_1 interrupt
											//    ALARM_1 is used for the Send Tip Count Message interrupt
											//    This alarm is enabled as needed in void loop()
	if (rtc.alarm(ALARM_1));				// Clear pending ALARM_1 interrupt from RTC



	rtc.alarmInterrupt(ALARM_2, false);		// Disable the RTC ALARM_2 interrupt
											//   ALARM_2 is used for the Daily Message Interrupt
	if(rtc.alarm(ALARM_2));					// Clear pending ALARM_2 interrupt from RTC

	//SMS on startup should have zero messages because they have all been cleared on startup with AT+CMGD=1,4
	statusMessage();
	milsDelay(3000);
/*fwb-30Aug15
	// Set the time for the daily update interrupt
	tm.Day = 0;			// Day does not matter since the alarm match will be on hours and minutes (or just minutes for testing)
	tm.Hour = 6;		// Set the hour and minute.  For example, 2:23 PM
	tm.Minute = 55;
	tm.Second = 0;		// Seconds does not matter since Alarm 2 does not use seconds


	//rtc.setAlarm(ALM2_MATCH_MINUTES, tm.Second, tm.Minute, tm.Hour, tm.Day);	// Set the alarm to match only Minutes.  Interrupt once per hour
	rtc.setAlarm(ALM2_MATCH_HOURS, tm.Second, tm.Minute, tm.Hour, tm.Day);		// Set the alarm to match Hours and Minutes.  Interrupt once per day
	rtc.alarmInterrupt(ALARM_2, true);											// Enable the RTC ALARM_2 interrupt
*/
	//moved the ALARM1 setting to here.  Basically, use the alarm to wake up every 5 minutes-fwb-30Aug15
	// Read the current time at a time_t variable.  This is an unsigned long containing time as the number of seconds since 1970
	//    Add the appropriate number of seconds to the variable (e.g., for 5 minutes, add 300 seconds)
	//    Convert the time_t variable to a tmElements_t variable and write to RTC ALARM_1.
	t = rtc.get();							// Get the current time
	Serial.println (t);
	t += INTERVAL;							// Add in the interval.  INTERVAL is defined in rain_gauge.h
	Serial.println(t);
	breakTime(t, tm);						// Format the time for setting the alarm.  Command found in time.cpp
	Serial.print(tm.Day);
	Serial.print(":");
	Serial.print(tm.Hour);
	Serial.print(":");
	Serial.print(tm.Minute);
	Serial.print(":");
	Serial.print(tm.Second);
	Serial.print("\n");
	if(rtc.alarm(ALARM_1));					// Clear the RTC ALARM_1 interrupt flag
	rtc.alarmInterrupt(ALARM_1, false);		// Disable the RTC ALARM_1 interrupt

	rtc.setAlarm(ALM1_MATCH_DATE, tm.Second, tm.Minute, tm.Hour, tm.Day);	// Set the alarm to match Date, Hour, Minute, Second

	rtc.alarmInterrupt(ALARM_1, true);		// Enable the RTC ALARM_1 interrupt

	// Attach the RTC alarm interrupt
	// Need to interrupt on LOW level (instead of edge trigger) because a LOW level interrupt will wake the processor from
	// power-down sleep mode.

	attachInterrupt(INTERRUPT_1, Alarm_Int_Handler, LOW);		//  Attach RTC alarm interrupt to the ATmega328p Interrupt_1 pin

	// Done with fona for now so it is powered off and the module is put to sleep until an interrupt sets a flag
	turnOffFONA();
	Serial.println("Got Here");
}

//   An INT_0 interrupt is generated by the rain gauge reed switch.
void Tip_Int_Handler()
{
	int level = 0;

	Tip_Count += 1;

	digitalWrite(13, HIGH);
	delay(100);
/* not really using Set_Interval alarm anymore -fwb-30Aug15
	// When the first tip in an interval occurs set the RTC alarm.
	if (Tip_Count == 1)
	{
		Set_Interval_Alarm = true;		// Set the flag to true.  The main loop will service this flag
	}
*/
	while(level == 0)
	{
		level = digitalRead(REED_SWITCH_PIN);
	}

	digitalWrite(13, LOW);
}


//  An INT_1 interrupt is generated by the RTC ALARM_1 or RTC ALARM_2 or both.
//    When an interrupt occurs, set the Rtc_Alarm flag to true.  The flag will is serviced
//    in the main loop
void Alarm_Int_Handler()
{
	detachInterrupt(INTERRUPT_1);
	Rtc_Alarm = true;

	digitalWrite(13, HIGH);		// For Testing Only.  Turn on LED for either alarm (send tip count or send daily message)
	delay(1000);
}


void Send_The_Tip_Count(int RepCnt)
{
	Serial.println ("RepCnt: ");
	Serial.println (RepCnt);
	delay(1000);
	if (RepCnt == 0 ) {
	  turnOnFONA();
	  delayFONA();
	  delay(5000);
	}
	char message[75] = {0}; //too short and you'll buffer overflow and reset the board. wasn't there an article about buffer overflows recently.....

	// Format the message: D1,A0,A1,A2,A3, Rain, Light, Temp, Depth, A3 (reserved)
	sprintf(message, "%5lu:%010lu:%04d:%04u:%04u:%04u:%04u:%04u\n", STATION,DateTimelong[RepCnt], Tip_Count_Copy[RepCnt],light[RepCnt],temp[RepCnt],depth[RepCnt],0,temp_RTC[RepCnt]);
//	sprintf(message, "%s,%s,%02u,%02u,%02u,%02u,%02u,%03u,%s,%04u,%s,%03u,%s,%03u,%s,%04u\n", STATION, PARAMETER, tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second, Tip_Count_Copy[RepCnt],":",temp[RepCnt],":",light[RepCnt],":",depth[RepCnt],":",000);
	//sprintf(message, "%s,%02u,%02u,%02u,%02u,%02u,%03u,%s,\n", STATION, tm.Month, tm.Day, tm.Year, tm.Hour, tm.Minute, tm.Second,":", Tip_Count_Copy);
///	sprintf(message, "%s\n","SPAM");
	/*
	// This stores the message in SIM card memory before it is sent real-time.  The stored messages are sent and deleted later in a daily update.
	fona.writeSMS(sendto, message);
	milsDelay(1000);
	*/
	// Send the message
	fona.sendSMS(sendto, message);
	milsDelay(3000);
	Serial.println("managed to get here");
	milsDelay(1000);
//	Tip_Count_Copy = 0; gets zeroed in loop after the call to this sub

/* will reinstate later, need to figure out how to store more in EEPROM and send
 * from there
	// initiates a daily message if the number of stored messages reaches 20
	if(fona.getNumSMS() < 0)
	{
		milsDelay(1000);
		fona.getNumSMS();
	}
	if (fona.getNumSMS() < 20)
	{
		turnOffFONA();
	}

	else
	{
		Send_The_Daily_Message();
	}
 */
	if (RepCnt == 2) {
		turnOffFONA();
	}
}

void Send_The_Daily_Message()
{
	// Synchronize the real time clock with the cell tower (or GPS?) time
	// Format and send the daily update message
	turnOnFONA();
	delayFONA();
	syncTime();
	milsDelay(1000);
	statusMessage();

	turnOffFONA();
}

// Put the controller in a low power state
// Set the sleep mode
// Valid settings are:
//     SLEEP_MODE_IDLE       � least power savings
//     SLEEP_MODE_ADC
//     SLEEP_MODE_PWR_SAVE
//     SLEEP_MODE_STANDBY
//     SLEEP_MODE_PWR_DOWN   � most power savings

void sleepNow()
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	// Set sleep enable (SE) bit:
	sleep_enable();

	sleep_mode();

	// Execute sleep_disable() when the processor exits sleep mode
	sleep_disable();
}

// A hard limit of 30 SMS messages is set for the daily to prevent a thousand messages from being sent in an hour
//  I went with 30 because that is the number of messages that can be stored on the fona.  Other hardware may give more storage.
void sendAllSMS()
{
	byte maxcount = 30;
	delayFONA();

	for (uint8_t smsn = 1; smsn <= storedsms; smsn++)
	{
		// Removed the hundreds digit because there should never be a need for it
		char sendbuff[11] = "AT+CMSS=00";
		//sendbuff[8] = (smsn / 100) + '0';
		//smsn %= 100;
		sendbuff[8] = (smsn / 10) + '0';
		smsn %= 10;
		sendbuff[9] = smsn + '0';
		fona.mySerial->println(sendbuff);
		fona.mySerial->println();
		milsDelay(3000);

		if(!fona.deleteSMS(smsn))
		{
			milsDelay(1000);
			startdelay = millis();
			while(!fona.deleteSMS(smsn))
			{
				milsDelay(1000);
				if(fona.deleteSMS(smsn))
				{
					break;
				}

				if(startdelay + 5000 > millis())
				{
					break;
				}
			}
		}
		maxcount --;
		if(maxcount < 1)
		{
			break;
		}
	}
	fona.mySerial->println("AT+CMGD=1,4");	// one last delete to clear everything.  The messages should have already been deleted individually.
	storedsms = 0;								// This resets the number of stored sms to zero as it should be after everything has been deleted
	turnOffFONA();							// This is redundant but I'm keeping it for now
}

void turnOnFONA()
{

	if (!digitalRead(FONA_PS)) //if there is no voltage on the power supply pin, i.e.-it's off
	{
		digitalWrite(FONA_KEY, LOW);				// pull Key pin LOW
		unsigned long KeyPress = millis();
		while (KeyPress + keyTime >= millis())		// wait two seconds
		{
			;
		}
		digitalWrite(FONA_KEY, HIGH);				// return PS pin to HIGH
		milsDelay(10000);							// delay for every boot should be as long as possible before delayFONA
													// loop because nothing can happen there
													// upped from 5s to 10s here to try to give them more time to connect
	}
}

void turnOffFONA()
{
	if (digitalRead(FONA_PS))
	{
		digitalWrite(FONA_KEY, LOW);
		unsigned long KeyPress = millis();
		while (KeyPress + keyTime >= millis())
		{
			;
		}
		digitalWrite(FONA_KEY, HIGH);
	}
}

// This function checks for a valid network registration before it moves on
void delayFONA()
{
	startdelay = millis();
	while(fona.getNetworkStatus() == 0)
	{
		milsDelay(1000);
		if(fona.getNetworkStatus() != 0)
		{
			break;
		}
		if(startdelay + 30000 >= millis()) // shuts down fona if registration takes longer than 30 seconds.
			//switching this value from 25 to 30 seconds forced the FONA to stay on during boot while running
			// at 1Mhz. Not sure why.
		{
			turnOffFONA();
			break;
		}

	}
}

// This function formats a status message for startup and daily updates.  It takes the battery life and number of stored messages from the fona
// and the time and temperature from the RTC
void statusMessage()
{
	t = rtc.get();
	breakTime(t, tm);
	char stat[75] = {0};

	startdelay = millis();

	if(fona.getNumSMS() > 0)
	{
		milsDelay(1000);
		storedsms = fona.getNumSMS();
	}

	 // initialize to 0.  this will be reset if there are more messages.
	// -1 is returned when there is a communication error with the fona.  This loop will
	// retry the getNumSMS function until it returns a value.  If getNumSMS does not
	// succeed in 10 seconds the function will exit without sending a message.

	if (storedsms < 0)
	{
		storedsms = fona.getNumSMS();
		milsDelay(1500);
		while( storedsms < 0 )
		{
			milsDelay(1500);
			storedsms = fona.getNumSMS();
			if (startdelay + 15000 >= millis())
			{
				storedsms = 0;
			}
		}
	}
	Figure_the_Time(0);
//               Station:MODDYYHHMM:D1:A0:A1:A2:A3, Rain, Light, Temp, Depth
	STATION=STATION+999; //tells the receiving program that it's a system reset
	//reset stamp is Station(999 extension), DateTimelong, lat, long, sig_Strength, timestamp
	sprintf(stat, "%5lu:%010lu:%04u:%04u:%04u:%04u", STATION, DateTimelong[0], 0,0,0,0);
	STATION=STATION-999; //put the station number back right
//	sprintf(stat, "%s,%s,%02u,%02u,%02u,%02u,%d", STATION, "STAT", tm.Month, tm.Day, tm.Hour, tm.Minute, storedsms);
	DateTimelong[0]=0; //reset the time for the l
	if(fona.sendSMS(sendto, stat))
	{
		;
	}
	milsDelay(3000);


 	if(storedsms > 0)
 	{
 		sendAllSMS();
 	}
}

// Using millis() is better than delay() for this because it allows other parts of the program to run while waiting
void milsDelay(unsigned int mils) //by design, int mils can only be from -32000 to 32000 roughly.  Changed to uint
{
	startdelay = millis(); //startdelay is a volatile ulong
	while (startdelay + mils >= millis())
	{
		;
	}
}

void syncTime()
{
	char gsmtime[64] = {0};
	if (!fona.getTime(gsmtime, 64))
	{
		startdelay = millis();
		milsDelay(1000);
		while(!fona.getTime(gsmtime, 64))
		{
			if(fona.getTime(gsmtime, 64))
			{
				break;
			}
			if (startdelay + 25000 > millis()) //upped this from 10s to 25s to give it more of a chance to sync.
			{
				break;
			}
		}
	}
	// Set the RTC using the GSM time
	String buffer_string = gsmtime;		// Declare a String object from the replybuffer char array
	// Put the date/time from the GSM string into the tmElements_t structure (tm)
	// tm.Year = buffer_string.substring(1,2).toInt();
	Serial.print("SyncTime tm.Year: ");
	Serial.println(tm.Year);
	int shortyear = buffer_string.substring(1, 3).toInt();
	Serial.print("ShortYear: ");
	Serial.println(shortyear);
	tm.Year = shortyear+30; //buffer_string.substring(1, 3).toInt(); //tm.Year is an offset from 1970
	tm.Month = buffer_string.substring(4, 6).toInt();
	tm.Day = buffer_string.substring(7, 9).toInt();
	tm.Wday = 0;
	tm.Hour = buffer_string.substring(10, 12).toInt();
	tm.Minute = buffer_string.substring(13, 15).toInt();
	tm.Second = buffer_string.substring(16, 18).toInt();


	// Write the date/time to the RTC
	rtc.write(tm);

	// clear the tm variable
	tm.Year = 0;
	tm.Month = 0;
	tm.Day = 0;
	tm.Wday = 0;
	tm.Hour = 0;
	tm.Minute = 0;
	tm.Second = 0;

}

void Read_the_Analogs(int RepCnt) {
	//********** Power pin for hopefully both Temp & Depth
	digitalWrite(DEPTH_PWR,HIGH);
    delay(500); //time to warm up
	//********** Temperature external
	unsigned int temp2;
	temp[RepCnt] = analogRead(TEMP_READ);
    temp2 = map(temp[RepCnt],0,1023,0,2800); //map the analog reading to 2.8V, the approx value on AREF pin using a diode
    realtemp = ((temp2/10.)) - 50; //temp in C.  the formula is 100*temp2 - 50, but since I'm scaling temp2 to 2800 instead of 2.8V, I just dropped the extra calc
    realtemp = ((realtemp * 9./5.) + 32.); //temp in F
    temp[RepCnt]=int(realtemp);
    Serial.print(F("The Temperature is: "));
    Serial.println(temp[RepCnt]);
//	temp[RepCnt] = analogRead(TEMP_READ);
//********** Light level reading
//  light[RepCnt] = analogRead(LIGHT_READ);
    light[RepCnt] = 0; //set to 0 b/c no new ones had a reading.
  Serial.print(F("The light level is :"));
  Serial.println(light[RepCnt]);
  delay(1000);
//*********** Depth
  depth[RepCnt] = analogRead(DEPTH_READ);
  temp2 = map(depth[RepCnt],180,915,0,1163); //see note below
  //183 is 0.5V for the probe based on a mapping of 0.5V = 0 psi, and a voltage range of 0 to 2.8V
  // the probe reads from 0.5V-2.5V.  2.5V is therefore about 913
  //In practice, the value of 183 is a little higher than it reads typically, which is ~180 because the voltage varies around 2.8-2.83.
  //so, 180 is 0.5V which is 0 psi or 0 ft of water.  913 is 2.5V, which is 5 psi or 1163 hundredths of a foot of water.
  depth[RepCnt] = temp2;
  digitalWrite(DEPTH_PWR,LOW);
  delay(500); //time to warm up
  Serial.print(F("The depth is :"));
  Serial.println(depth[RepCnt]);
//*********** RTC Temperature
  temp_RTC[RepCnt] = rtc.temperature();
  Serial.print("temp_RTC_C_x4: ");
  Serial.println(temp_RTC[RepCnt]);
  delay(500); //time to warm up
  realtemp = (((temp_RTC[RepCnt]/4.)*9./5.)+32.);
  temp_RTC[RepCnt] = int(realtemp);
  Serial.print("temp_RTC_F: ");
  Serial.println(temp_RTC[RepCnt]);
  if (int(STATION/1000) >= 25) { //It's a board with no analog inputs
	  temp[RepCnt] = 0;
	  light[RepCnt]=0;
	  depth[RepCnt]=0;
  }
}

void Figure_the_Time(int RepCnt) {
	t = rtc.get();							// Get the current time
	breakTime(t, tm);						// Format the time for setting the alarm.  Command found in time.cpp
	DateTimelong[RepCnt] = int(tm.Month) * 100000000;
    DateTimelong[RepCnt] = DateTimelong[RepCnt] + int(tm.Day) * 1000000;
    unsigned long int shortyear = tm.Year-30; //Year is the offset from 1970 converted to +30.  i don't know why.//this is for the adafruit version%100;
    Serial.print("tm.Year: ");
    Serial.println(tm.Year);
    Serial.print("Short Year: ");
    Serial.println(shortyear);
    DateTimelong[RepCnt] = DateTimelong[RepCnt] + shortyear * 10000;
    DateTimelong[RepCnt] = DateTimelong[RepCnt] + int(tm.Hour) * 100;
    DateTimelong[RepCnt] = DateTimelong[RepCnt] + int(tm.Minute) * 1;
	Serial.print("DateTimeLong: ");
	Serial.println(DateTimelong[RepCnt]);

}
	int freeRam ()
    {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

void loop()
{
	Serial.println("Entered Loop");
	Serial.print("RTC_Alarm: ");
	Serial.println(Rtc_Alarm);
	Serial.print("Set_Interval_Alarm: ");
	Serial.println(Set_Interval_Alarm);
	delay(1000);
/* removed in favor of waking from ALARM1 every 5 minutes (useful for logging analog parameters at the location)-fwb30Aug15
	if (Set_Interval_Alarm)
	{
		// Read the current time at a time_t variable.  This is an unsigned long containing time as the number of seconds since 1970
		//    Add the appropriate number of seconds to the variable (e.g., for 5 minutes, add 300 seconds)
		//    Convert the time_t variable to a tmElements_t variable and write to RTC ALARM_1.
		t = rtc.get();							// Get the current time

		t += INTERVAL;							// Add in the interval.  INTERVAL is defined in rain_gauge.h

		breakTime(t, tm);						// Format the time for setting the alarm.  Command found in time.cpp

		if(rtc.alarm(ALARM_1));					// Clear the RTC ALARM_1 interrupt flag
		rtc.alarmInterrupt(ALARM_1, false);		// Disable the RTC ALARM_1 interrupt

		rtc.setAlarm(ALM1_MATCH_DATE, tm.Second, tm.Minute, tm.Hour, tm.Day);	// Set the alarm to match Date, Hour, Minute, Second

		rtc.alarmInterrupt(ALARM_1, true);		// Enable the RTC ALARM_1 interrupt

		Set_Interval_Alarm = false;				// The alarm is set so reset the flag to false
	}
*/


	if (Rtc_Alarm)
	{
		// Determine if RTC ALARM_1 caused the interrupt
		if (rtc.alarm(ALARM_1))				// Get the ALARM_1 status and clear the ALARM_1 interrupt flag, if set
		{
			Figure_the_Time(RepCnt); //construct the DateTimeLong timestring
		    Read_the_Analogs(RepCnt); //Read the analog pins, including the internal RTC temperature
			Tip_Count_Copy[RepCnt] = Tip_Count;     // Make a copy of Tip_Count and then clear Tip_Count so that
			Tip_Count=0;                  //    Tip_Count start counting while the SMS message is sent
			RepCnt++; //a little screwy, but it works
			Serial.print("RepCnt: ");
			Serial.println(RepCnt);
			if (RepCnt > 2) {
				for (int i=0;i<DataPts_to_Store;i++) {
					Send_The_Tip_Count(i);
				}
				RepCnt=0;
				for (int i=0;i<DataPts_to_Store;i++) {
				        	rain[i]=0;
				        	light[i]=0;
				        	temp[i]=0;
				        	DateTimelong[i]=0;
				        	depth[i]=0;
				        	Tip_Count_Copy[i]=0;
				}
			}
			// Read the current time at a time_t variable.  This is an unsigned long containing time as the number of seconds since 1970
			//    Add the appropriate number of seconds to the variable (e.g., for 5 minutes, add 300 seconds)
			//    Convert the time_t variable to a tmElements_t variable and write to RTC ALARM_1.
			t = rtc.get();							// Get the current time
			Serial.println (t);

			t += INTERVAL;							// Add in the interval.  INTERVAL is defined in rain_gauge.h
			Serial.println(t);

			breakTime(t, tm);						// Format the time for setting the alarm.  Command found in time.cpp

			if(rtc.alarm(ALARM_1));					// Clear the RTC ALARM_1 interrupt flag
			rtc.alarmInterrupt(ALARM_1, false);		// Disable the RTC ALARM_1 interrupt

			Serial.print("The next alarm is: ");
			breakTime(t, tm);						// Format the time for setting the alarm.  Command found in time.cpp
			Serial.print(tm.Day);
			Serial.print(":");
			Serial.print(tm.Hour);
			Serial.print(":");
			Serial.print(tm.Minute);
			Serial.print(":");
			Serial.print(tm.Second);
			Serial.print("\n");
			delay(2000);

			rtc.setAlarm(ALM1_MATCH_DATE, tm.Second, tm.Minute, tm.Hour, tm.Day);	// Set the alarm to match Date, Hour, Minute, Second

			rtc.alarmInterrupt(ALARM_1, true);		// Enable the RTC ALARM_1 interrupt

			attachInterrupt(INTERRUPT_1, Alarm_Int_Handler, LOW);	// Re-attach the RTC alarm interrupt to the ATmega328p INTERRUPT_1
			digitalWrite(13, LOW);
			delay(1000);
			Rtc_Alarm = false;
		}
/*Not really worried about ALARM2 now, we're going to use an if construct to tell when to send the Daily Message
 * For now, I'm not going to restore the Daily Message, because I need to work out the timings first.  I'll also
 * clear the subroutine since Send_The_Tip_Count will call it if the number of stored messages exceeds 20.
		// Determine if RTC ALARM_2 (also) caused the interrupt.
		if (rtc.alarm(ALARM_2))				// Get the ALARM_2 status and clear the ALARM_2 interrupt flag, if set
		{
			Send_The_Daily_Message();

		}
		attachInterrupt(INTERRUPT_1, Alarm_Int_Handler, LOW);	// Re-attach the RTC alarm interrupt to the ATmega328p INTERRUPT_1
		digitalWrite(13, LOW);
		delay(1000);
		Rtc_Alarm = false;
	}
*/
	}

	// If none of the flags are set, enter sleep mode
	if (!Set_Interval_Alarm & !Rtc_Alarm)
	{
		sleepNow();
	}

}

