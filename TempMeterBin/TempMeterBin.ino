#include <avr/sleep.h>

#include <RH_ASK.h>

#include <dht.h>


#define LED_PIN 13
#define LED 13


#define BLINK_PIN 2

#define DEVICE_ID 4 // Uniqe id in your network

#define RX_PIN 5 // not used
#define TX_PIN 4

#define DHT_DATA 8

#define VCC_PIN 3 // power for radio
#define GND_PIN 2

#define VCC_PIN2 9 // power for DHT22
#define GND_PIN2 7

#define TEMP_CAL -0.5 // Calibrate temperature offset

// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>

typedef union {
 float number;
 uint8_t bytes[4];
} FLOATUNION_t;

typedef union {
 int number;
 uint8_t bytes[2];
} INTUNION_t;

typedef union {
 long number;
 uint8_t bytes[4];
} LONGUNION_t;

FLOATUNION_t fdata;
INTUNION_t idata;
LONGUNION_t ldata;

// Create Amplitude Shift Keying Object
RH_ASK rf_driver(1000,RX_PIN,TX_PIN);

dht DHT;

long blinkCounter = 0;
long lastTime = -1000000000;

float watts = 0.0;

int readData = 0;
float t;
float h;

void setup() {
  for (byte i = 0; i <= A5; i++)  {
    pinMode (i, INPUT);  // set all pins to the most power saving state
    digitalWrite (i, LOW);  
  }
  powerOn();
  setupSleep8();
  Serial.begin(9600);
  Serial.println("Setup");


  pinMode (LED_PIN, INPUT);
  digitalWrite (LED_PIN, LOW); 
  
  rf_driver.init();
}


void loop() {
  
  Serial.println("loop");

  // Main function 
  // Steps to do:
  // 1. Read temp
  // 2. send
  // 3. sleep

  powerOn();

  // 3. Add to counter 
  blinkCounter++;

  
  // 4. Measure battery voltage
  long battV = vccVoltage();
  
  // 5. Send all data

  // new binary format
  uint8_t binmsg[24];
  uint8_t buflen = sizeof(binmsg);

  byte x = 0;
  byte idchar= DEVICE_ID;
  binmsg[0] = idchar;
  
  binmsg[1] = 4; // number of variables
  
  x = 2;
  binmsg[x] = 'l';
  ldata.number = blinkCounter;
  binmsg[x+1] = ldata.bytes[0];
  binmsg[x+2] = ldata.bytes[1];
  binmsg[x+3] = ldata.bytes[2];
  binmsg[x+4] = ldata.bytes[3];
  x=x+5;
  
  binmsg[x] = 'f';
  fdata.number = t;
  binmsg[x+1] = fdata.bytes[0];
  binmsg[x+2] = fdata.bytes[1];
  binmsg[x+3] = fdata.bytes[2];
  binmsg[x+4] = fdata.bytes[3];
  x=x+5;
  
  binmsg[x] = 'f';
  fdata.number = h;
  binmsg[x+1] = fdata.bytes[0];
  binmsg[x+2] = fdata.bytes[1];
  binmsg[x+3] = fdata.bytes[2];
  binmsg[x+4] = fdata.bytes[3];
  x=x+5;

  binmsg[x] = 'l';
  ldata.number = battV;
  binmsg[x+1] = ldata.bytes[0];
  binmsg[x+2] = ldata.bytes[1];
  binmsg[x+3] = ldata.bytes[2];
  binmsg[x+4] = ldata.bytes[3];
  
  binmsg[x+5] = '\n';
  binmsg[x+6] = '\0';
  //Serial.println(binmsg);
  
  rf_driver.send((uint8_t *)binmsg, buflen);
  rf_driver.waitPacketSent();
  delay(100);
  rf_driver.send((uint8_t *)binmsg, buflen);
  rf_driver.waitPacketSent();
  delay(100);
  rf_driver.send((uint8_t *)binmsg, buflen);
  rf_driver.waitPacketSent();
  
  //goToSleep();

  // 1. Read DHT
  delay(200); // extra delay for DHT to wake up
  readData = DHT.read22(DHT_DATA);
  t = DHT.temperature + TEMP_CAL;
  h = DHT.humidity;
  powerDown();
  sleep8s();
  __asm__ __volatile__ ("nop\n\t");
  sleep8s();
  //delay(8000);
}

void powerDown() {
  pinMode (GND_PIN, OUTPUT);
  digitalWrite (GND_PIN, LOW); 
  pinMode (VCC_PIN, OUTPUT);
  digitalWrite (VCC_PIN, LOW);   
  
  pinMode (GND_PIN2, OUTPUT);
  digitalWrite (GND_PIN2, LOW); 
  pinMode (VCC_PIN2, OUTPUT);
  digitalWrite (VCC_PIN2, LOW); 

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW); 
}

void powerOn() {
  pinMode (GND_PIN, OUTPUT);
  digitalWrite (GND_PIN, LOW); 
  pinMode (VCC_PIN, OUTPUT);
  digitalWrite (VCC_PIN, HIGH);   
  
  pinMode (GND_PIN2, OUTPUT);
  digitalWrite (GND_PIN2, LOW); 
  pinMode (VCC_PIN2, OUTPUT);
  digitalWrite (VCC_PIN2, HIGH); 

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, HIGH); 
  
}

void interuptFunction() {
  // cancel sleep as a precaution
  sleep_disable();
  // precautionary while we do other stuff
  detachInterrupt (0);

  // When this is done the loop() function will be called and we do everything there
  
}

long vccVoltage() {
  // Function to read battery voltage from Vcc
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}
