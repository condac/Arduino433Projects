#include <avr/sleep.h>

#include <RH_ASK.h>

#include <dht.h>


#define LED_PIN 13
#define LED 13


#define BLINK_PIN 2

#define DEVICE_ID 2 // Uniqe id in your network

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
  String out = " {\"id\":"; // make sure first char is a space that we replace with id later
  out += DEVICE_ID;
  out += ",\"a\":";
  out.concat(blinkCounter);
  out += ",\"t\":";
  out.concat(t);
  out += ",\"h\":";
  out.concat(h);
  out += ",\"v\":";
  out.concat(battV);
  
  out += "}\n\0"; // end, also add a \n to find end on reciever side

  // TODO send multiple times? due to the long warmup of the dht after sleep mode we have lots of time to send multiple messages
  Serial.println(out);
  static char *msg = out.c_str();
  byte idchar= DEVICE_ID;
  msg[0] = idchar;
  rf_driver.send((uint8_t *)msg, strlen(msg));
  rf_driver.waitPacketSent();

rf_driver.send((uint8_t *)msg, strlen(msg));
  rf_driver.waitPacketSent();
  rf_driver.send((uint8_t *)msg, strlen(msg));
  rf_driver.waitPacketSent();
  
  //goToSleep();

  // 1. Read DHT
  delay(400); // extra delay for DHT to wake up
  readData = DHT.read22(DHT_DATA);
  t = DHT.temperature + TEMP_CAL;
  h = DHT.humidity;
  powerDown();
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
