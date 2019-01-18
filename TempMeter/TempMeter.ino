#include <avr/sleep.h>

#include <RH_ASK.h>


#define LED_PIN 13
#define LED 13


#define BLINK_PIN 2

#define DEVICE_ID 2 // Uniqe id in your network

#define RX_PIN 3
#define TX_PIN 7

// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>


// Create Amplitude Shift Keying Object
RH_ASK rf_driver(2000,RX_PIN,TX_PIN);

long blinkCounter = 0;
long lastTime = -1000000000;

float watts = 0.0;

void setup() {
  Serial.begin(9600);
  Serial.println("Setup");
  for (byte i = 0; i <= A5; i++)  {
    pinMode (i, INPUT);  // set all pins to the most power saving state
    digitalWrite (i, LOW);  
  }

  pinMode (5, OUTPUT);
  digitalWrite (5, LOW); 
  pinMode (6, OUTPUT);
  digitalWrite (6, HIGH);  

  pinMode (LED_PIN, INPUT);
  digitalWrite (LED_PIN, LOW); 
  
  rf_driver.init();
}

void goToSleep() {
  pinMode (LED, OUTPUT);
  digitalWrite (LED, HIGH);
  delay (50);
  digitalWrite (LED, LOW);
  delay (50);
  pinMode (LED, INPUT);
  // disable ADC
  ADCSRA = 0;  

  /* SLEEP_MODE_IDLE: 15 mA
   SLEEP_MODE_ADC: 6.5 mA
   SLEEP_MODE_PWR_SAVE: 1.62 mA
   SLEEP_MODE_EXT_STANDBY: 1.62 mA
   SLEEP_MODE_STANDBY : 0.84 mA
   SLEEP_MODE_PWR_DOWN : 0.36 mA
   */
  //set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  set_sleep_mode (SLEEP_MODE_STANDBY);  
  sleep_enable();

  // Do not interrupt before we go to sleep, or the
  // ISR will detach interrupts and we won't wake.
  noInterrupts ();
  
  // will be called when pin D2 goes low  
  attachInterrupt (0, interuptFunction, FALLING);
  EIFR = bit (INTF0);  // clear flag for interrupt 0
 
  // turn off brown-out enable in software
  // BODS must be set to one and BODSE must be set to zero within four clock cycles
  MCUCR = bit (BODS) | bit (BODSE);
  // The BODS bit is automatically cleared after three clock cycles
  MCUCR = bit (BODS); 
  
  // We are guaranteed that the sleep_cpu call will be done
  // as the processor executes the next instruction after
  // interrupts are turned on.
  interrupts ();  // one cycle
  sleep_cpu ();   // one cycle
}
void loop() {
  delay(1000);
  Serial.println("loop");

  // Main function 
  // Steps to do:
  // 1. Read temp
  // 2. send
  // 3. sleep



  // 1. Power on transmitter
  

  // 2. Calculate time since last blink
  long currentTime = millis();
  long deltaTime = currentTime - lastTime;
  lastTime = currentTime;
  watts = deltaTime/3600.0; //TODO
  Serial.println(deltaTime);
  Serial.println(millis());
  // 3. Add to counter for number of blinks seen
  blinkCounter++;

  
  // 4. Measure battery voltage
  long battV = vccVoltage();
  
  // 5. Send all data
  String out = " {\"id\":"; // make sure first char is a space that we replace with id later
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
  byte idchar= DEVICE_ID;
  msg[0] = idchar;
  rf_driver.send((uint8_t *)msg, strlen(msg));
  rf_driver.waitPacketSent();

  delay(50);
  //goToSleep();
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
