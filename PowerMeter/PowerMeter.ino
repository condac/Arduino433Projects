#include <avr/sleep.h>

#include <RH_ASK.h>



#define VCC_PIN 6 // power for radio
#define GND_PIN 5

#define RX_PIN 5 // not used
#define TX_PIN 7
#define LED_PIN LED_BUILTIN

#define TX_TIMER 10000 // how often we send information 

#define BLINK_PIN 2

#define DEVICE_ID 3 // Uniqe id in your network


// Create Amplitude Shift Keying Object
RH_ASK rf_driver(1000,RX_PIN,TX_PIN);


long blinkCounter = 0;
long lastCounter = 0;
long lastTime = -1000000000;
long txDelay = 0;
long battV;
bool trigger = false;
float watts = 0.0;
long heartbeat = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Setup");


  powerDown();

  pinMode (BLINK_PIN, INPUT);
  digitalWrite (BLINK_PIN, LOW); 

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(4,OUTPUT);
  digitalWrite (4, LOW);
  pinMode(3,OUTPUT);
  digitalWrite (3, LOW);
  rf_driver.init();
  randomSeed(analogRead(4));

  attachInterrupt(digitalPinToInterrupt(BLINK_PIN), interuptFunction, FALLING);
}


void loop() {

  if (trigger) {
    trigger = false;
    lastCounter = blinkCounter;
    
    loopHandle();
    delay(100);
    digitalWrite (LED_PIN, LOW); 
    //pinMode (BLINK_PIN, OUTPUT);
    //digitalWrite (BLINK_PIN, HIGH); 
    //pinMode (BLINK_PIN, INPUT);
    trigger = false;
    attachInterrupt(digitalPinToInterrupt(BLINK_PIN), interuptFunction, RISING);
  }
  if (txDelay<millis()) {
    txDelay = TX_TIMER+millis() + random(1000);
    sendRadio();
  }
}

void loopHandle() {
  //Serial.println("loopHandle");
  
  // 2. Calculate time since last blink
  long currentTime = millis();
  long deltaTime = currentTime - lastTime;
  
  if (deltaTime>120) {
    lastTime = currentTime;
    blinkCounter++;
    watts = 3600000.0/deltaTime;
  }
   //TODO
  //Serial.println(watts);
  // 3. Add to counter for number of blinks seen
  // moved to interupt function
  
  // 4. Measure battery voltage
  //battV = vccVoltage();
  
  
}
void interuptFunction() {
  detachInterrupt(digitalPinToInterrupt(BLINK_PIN));
  trigger = true;
  digitalWrite (LED_PIN, HIGH); 
  
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

void sendRadio() {
  powerOn();
  heartbeat++;
  // 5. Send all data
  String out = " {\"id\":";
  out += DEVICE_ID;
  out += ",\"b\":";
  out.concat(blinkCounter);
  out += ",\"w\":";
  out.concat(watts);
  out += ",\"h\":";
  out.concat(heartbeat);
  out += ",\"x\":";
  out.concat(heartbeat);
  
  out += "}\n\0"; // end, also add a \n to find end on reciever side

  // TODO send multiple times?
  //Serial.println(out);
  static char *msg = out.c_str();
  byte idchar= DEVICE_ID;
  msg[0] = idchar;
  String out2 = " {\"id\":0}\n\0";
  static char *msg2 = out2.c_str();
  msg2[0] = 1;
  rf_driver.send((uint8_t *)msg2, strlen(msg2));
  rf_driver.waitPacketSent();
  rf_driver.send((uint8_t *)msg, strlen(msg));
  rf_driver.waitPacketSent();
  // 6. Power off
  powerDown();
}
void powerDown() {
  pinMode (GND_PIN, OUTPUT);
  digitalWrite (GND_PIN, LOW); 
  pinMode (VCC_PIN, OUTPUT);
  digitalWrite (VCC_PIN, LOW);   
  
  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW); 
}

void powerOn() {
  pinMode (GND_PIN, OUTPUT);
  digitalWrite (GND_PIN, LOW); 
  pinMode (VCC_PIN, OUTPUT);
  digitalWrite (VCC_PIN, HIGH);   
  
  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, HIGH); 
  
}
