#include <SoftwareSerial.h>
#include <SD.h>
#include <EEPROM.h>
#include<stdlib.h>

// Liquid Crystal Display
// Define the LCD pins: We'll be using a serial-based LCD display
// which only required +3.3Volts, GND, and a single data line.
// databuff and displaybuff hold the data to be displayed
#define LCDIn 2
#define LCDOut 5      
SoftwareSerial mySerialPort(LCDIn, LCDOut);


// Data Buffers for the LCD
char databuff1[16];  
char databuff2[16];  
char dispbuff[16];  

// GAS SENSORS
// Analog input pin that reads the first gas sensor
const int gasPin1 = A5;    

// Analog input pin that reads the gas sensor
const int gasPin2 = A4;    

// The digital pin that controls the heater of gas sensor 1
const int heaterPin1 = 7; 

// The digital pin that controls the heater of gas sensor 2
const int heaterPin2 = 9;

// LED connected to digital pin 13
const int ledPin = 13;  

// value read from the sensor A5
int gasVal1 = 0;    

// value read from the sensor A4
int gasVal2 = 0;    

long warmup = 180000;           // enter time for heaters to warmup, in milliseconds. 
// 180,000 milliseconds = 3 minutes

long downtime = 360000;       // enter delay between readings, in milliseconds. 
// 360,000 milli seconds = 6 minutes

//EEPROM records require two bytes to store a 1024 bit value.
//Each gas sensor returns a value from 0-1024, taking 2 bytes.
//To store gas sensor data would require a record index, 
//plus two bytes for the first gas sensor, two bytes for the second gas sensor
//For a total of five bytes per record.

// current EEPROM address
int addr =0;        

//EEPROM record number
int record = 0;    

//EEPROM record length
int reclen = 5;    

//switch to tell if an SD card is present
int SDPresent = 1;

void setup() 
{
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 

  pinMode(heaterPin1, OUTPUT);  // sets the digital pins as output 
  pinMode(heaterPin2, OUTPUT);   
  pinMode(LCDOut, OUTPUT);   

  //reset the LCD  
  mySerialPort.begin(9600);
  mySerialPort.write(0xFE); 
  mySerialPort.write(0x01); 
  sprintf(databuff1,"Wakeup Test");
  sprintf(dispbuff,"%-16s",databuff1);
  mySerialPort.print(dispbuff);

  // Set up SD card, let us know if SD card is absent
  pinMode(10, OUTPUT);
  if (!SD.begin(4))
  { 
    SDPresent =0;
    sprintf(databuff2,"NO SD CARD!!!");
    sprintf(dispbuff,"%-16s",databuff2);    
    mySerialPort.print(dispbuff);
    Serial.println("NO SD CARD!!!");
    delay(6000);
  }
  delay(3333);
}

void loop() 
{
  long scratch=0;   // scratch variable

  // set the timer
  unsigned long counter = millis();   

  //turn first heater on
  digitalWrite(heaterPin1, HIGH);

  // wait 3 minutes for heater to heat up
  while(millis() < (counter + warmup))
  { 
    sprintf(databuff1,"Unit1 Activated");
    sprintf(dispbuff,"%-16s",databuff1);
    mySerialPort.print(dispbuff);

    scratch = (int)((counter+warmup - millis())/1000);
    sprintf(databuff2,"Countdown: %3d", scratch);
    sprintf(dispbuff,"%-16s",databuff2);    
    mySerialPort.print(dispbuff);

    Serial.println(scratch);
  }

  // read the analog in value:
  gasVal1 = analogRead(gasPin1);
  sprintf(databuff1,"read unit 1");
  sprintf(dispbuff,"%-16s",databuff1);
  mySerialPort.print(dispbuff);

  // shut off the first heater  
  digitalWrite(heaterPin1, LOW);

  //turn second heater on
  digitalWrite(heaterPin2, HIGH);
  sprintf(databuff2,"turning on unit2");
  sprintf(dispbuff,"%-16s",databuff2);
  mySerialPort.print(dispbuff);

  // wait 3 minutes for heater to heat up
  while(millis() < (counter + warmup + warmup))
  { 
    sprintf(databuff1,"Unit2 Activated");
    sprintf(dispbuff,"%-16s",databuff1);
    mySerialPort.print(dispbuff);

    scratch = (int)((counter+warmup+warmup - millis())/1000);
    sprintf(databuff2,"Countdown: %3d", scratch);
    sprintf(dispbuff,"%-16s",databuff2);    
    mySerialPort.print(dispbuff);

    Serial.println(scratch);
  }

  // read the analog in value:
  gasVal2 = analogRead(gasPin2);
  sprintf(databuff2,"reading unit2");
  sprintf(dispbuff,"%-16s",databuff2);
  mySerialPort.print(dispbuff);

  // shut off the second heater  
  digitalWrite(heaterPin2, LOW);

  //Display on LCD
  sprintf(databuff1,"Gas1:%4d",gasVal1);
  sprintf(dispbuff,"%-16s",databuff1);
  mySerialPort.print(dispbuff);
  sprintf(databuff2,"Gas2:%4d",gasVal2);
  sprintf(dispbuff,"%-16s",databuff2);
  mySerialPort.print(dispbuff);

//write to SD card
  if(SDPresent = 1)
  {
    writeDataToSD(databuff1, databuff2);            
  }

  //Wait downtime and start again
  //to make more frequent measurements, change value of downtime
  while(millis() < (counter +downtime))
  { 
  }
}

void writeDataToSD(String dataString1, String dataString2)
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) 
  {
    Serial.println("Hooray, we have a file!");
    dataFile.print(millis());
    dataFile.print(","); 
    dataFile.print(dataString1);
    dataFile.print(",");
    dataFile.println(dataString2);

    dataFile.close();

    // print to the serial port too:
    Serial.print(millis());
    Serial.print(","); 
    Serial.print(dataString1);
    Serial.print(",");
    Serial.println(dataString2);

    //Print to LCD
    mySerialPort.print("Datafile written");
  }  
}
