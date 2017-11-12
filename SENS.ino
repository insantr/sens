#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <SPI.h>
#include "RF24.h"
#include <printf.h>
#include "printf.h"
#include <stdarg.h>
#include <Arduino.h>
#include "LowPower.h"


#define VIN_HTU21 5
#define BATT_PIN A0
#define FULL_BATT_VOLT 3.3
#define MAX_TEMP 100.0
#define MIN_TEMP -100.0
#define MAX_HUMD 100.0
#define MIN_HUMD 0.0
#define DEBUG false
#define DELAY_READ_HTU21 50 //20-27
#define SENSOR_ID 0x01
#define MAX_REPEAT 100
#define TIMEOUT 600 // in sec
#define TIMEOUT_AFTER_SEND_FAIL 60 // in sec
byte pipe[6] = "1NODE";

HTU21D myHumidity;
RF24 radio(9, 10);

struct dataStruct{
  float humd;
  float temp;
  float batt;
  unsigned long _time;
  char id;
}myData;

unsigned int loop_number = 0;
bool last_send_result = true;
bool first_run = true;

void initRF24();

void setup()
{
  #if DEBUG
  Serial.begin(9600);
  printf_begin();
  Serial.println("HELLO WORLD");
  //pinMode(LED_BUILTIN, OUTPUT);
  #endif

  
  digitalWrite(VIN_HTU21, LOW);
  pinMode(VIN_HTU21, OUTPUT);
  digitalWrite(VIN_HTU21, HIGH);
  myHumidity.begin();
  digitalWrite(VIN_HTU21, LOW);

  initRF24();
  radio.startListening();
  radio.powerDown();
  
  #if DEBUG
  radio.printDetails();
  #endif
}

void initRF24(){
  radio.begin();
  radio.setPALevel(RF24_PA_MAX); // RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio.failureDetected = 0;
  radio.setDataRate(RF24_250KBPS); // RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setChannel(125);
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  radio.openWritingPipe(pipe);
  radio.openReadingPipe(1,0xF0F0F0F066);
}

void readHTU21(){
  digitalWrite(VIN_HTU21, HIGH);
  delay(DELAY_READ_HTU21); 
  myData.temp = myHumidity.readTemperature();
  while(myData.temp > MAX_TEMP || myData.temp < MIN_TEMP){
    delay(DELAY_READ_HTU21); 
    myData.temp = myHumidity.readTemperature();
  }
  myData.humd = myHumidity.readHumidity();
  while(myData.humd > MAX_HUMD || myData.humd < MIN_HUMD){
    delay(DELAY_READ_HTU21); 
    myData.humd = myHumidity.readTemperature();
  }
  digitalWrite(VIN_HTU21, LOW);
}

void readBattVoltage(){
  myData.batt = (float)(analogRead(BATT_PIN) * (FULL_BATT_VOLT / 1023.0));
}

void loop()
{
  
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  loop_number++;
  if (first_run || (last_send_result && (loop_number * SLEEP_8S > TIMEOUT)) || (!last_send_result && (loop_number * SLEEP_8S > TIMEOUT_AFTER_SEND_FAIL))){
    loop_number = 0;
    first_run = false;
  
    memset (&myData,0,sizeof(myData));
  
    myData.id = SENSOR_ID;
    myData._time = millis();
    readHTU21();
    readBattVoltage();
  
    radio.powerUp(); // This will take up to 5ms for maximum compatibility
    LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF); 
    
    initRF24();
    LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF); 
    radio.stopListening();
    last_send_result = radio.write(&myData,sizeof(myData));
    
    unsigned int repeat = 0;
    while(!last_send_result && repeat < MAX_REPEAT){
      repeat++;
      last_send_result = radio.write(&myData,sizeof(myData));
    }

    radio.startListening();
    
    #if DEBUG
    char deb_log[256];
    char h[15];
    char t[15];
    char v[15];
    dtostrf(myData.humd,4, 1, h);
    dtostrf(myData.temp,4, 1, t);
    dtostrf(myData.batt,5, 2, v);
    sprintf(deb_log, "T: %sC; H: %s%%; B: %sV; Res: %d; Rep: %d; T: %lu;", t, h, v, last_send_result, repeat, myData._time);
    Serial.println(deb_log);
    #endif
    
    radio.powerDown();
  }
}
