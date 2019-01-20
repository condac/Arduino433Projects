#include <avr/sleep.h>

#include <RH_ASK.h>
RH_ASK rf_driver;

#define TX_POWER 10 // Use 2 power pins to spread the load
#define TX_POWER2 11 // the tx use 9-40mA and pins can only deliver 20mA

#define TX_OUT 12 // This cant be changed, hardcoded in RH_ASK driver
#define LED_PIN LED_BUILTIN



#define BLINK_PIN 2

#define DEVICE_ID 123 // Uniqe id in your network

long blinkCounter = 0;
long lastCounter = 0;
long lastTime = -1000000000;

float watts = 0.0;

void setup() {
  Serial.begin(9600);
  Serial.println("Setup");


  pinMode (TX_POWER, INPUT); // Power pin is left as INPUT to have it floating when not used
  digitalWrite (TX_POWER, LOW); // If it is set to OUTPUT it will shortout when one of the pins go HIGH
  pinMode (TX_POWER2, INPUT);
  digitalWrite (TX_POWER2, LOW);

  pinMode (BLINK_PIN, INPUT);
  digitalWrite (BLINK_PIN, HIGH); 

  pinMode(LED_BUILTIN, OUTPUT);
  
  rf_driver.init();

  attachInterrupt(digitalPinToInterrupt(BLINK_PIN), interuptFunction, FALLING);
}


void loop() {

  if (blinkCounter>lastCounter) {
    lastCounter = blinkCounter;
    loopHandle();
  }

}

void loopHandle() {
  Serial.println("loopHandle");
  // Main function when we see a blink on the power meter
  // Steps to do:
  // 1. Power on transmitter, so it can warm up while we do other stuff
  // 2. Calculate time since last blink
  // 3. Add to counter for number of blinks seen
  // 4. Measure battery voltage
  // 5. Send all data
  // 6. Power off

  
  // 1. Power on transmitter
  pinMode (TX_POWER, OUTPUT); 
  pinMode (TX_POWER2, OUTPUT);
  digitalWrite (TX_POWER, LOW); // GND of TX is connected to pins so pulling it LOW power the TX on
  digitalWrite (TX_POWER2, LOW); 

  pinMode (LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW);

  // 2. Calculate time since last blink
  long currentTime = millis();
  long deltaTime = currentTime - lastTime;
  lastTime = currentTime;
  watts = 3600000.0/deltaTime; //TODO
  Serial.println(deltaTime);
  Serial.println(millis());
  // 3. Add to counter for number of blinks seen
  // moved to interupt function
  

  
  // 4. Measure battery voltage
  long battV = vccVoltage();
  
  // 5. Send all data
  String out = "{\"id\":";
  out += DEVICE_ID;
  out += ",\"b\":";
  out.concat(blinkCounter);
  out += ",\"w\":";
  out.concat(watts);
  out += ",\"v\":";
  out.concat(battV);
  
  out += ",}"; // end

  // TODO send multiple times?
  Serial.println(out);
  static char *msg = out.c_str();
    
  rf_driver.send((uint8_t *)msg, strlen(msg));
  rf_driver.waitPacketSent();

  // 6. Power off
  pinMode (TX_POWER, INPUT); 
  pinMode (TX_POWER2, INPUT);
  digitalWrite (TX_POWER, LOW); // GND of TX is connected to pins so pulling it LOW power the TX on
  digitalWrite (TX_POWER2, LOW); 
  
  digitalWrite (LED_PIN, HIGH);
  pinMode (LED_PIN, OUTPUT);
}
void interuptFunction() {
  blinkCounter++;
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
