#include <avr/sleep.h>


#define TX_POWER 7 // Use 2 power pins to spread the load
#define TX_POWER2 8 // the tx use 9-40mA and pins can only deliver 20mA

#define BLINK_PIN 2

#define DEVICE_ID 123 // Uniqe id in your network

long blinkCounter = 0;
long lastTime = -1000000000;

void setup() {
  for (byte i = 0; i <= A5; i++)  {
    pinMode (i, INPUT);  // set all pins to the most power saving state
    digitalWrite (i, LOW);  
  }

  pinMode (TX_POWER, INPUT); // Power pin is left as INPUT to have it floating when not used
  digitalWrite (TX_POWER, LOW); // If it is set to OUTPUT it will shortout when one of the pins go HIGH
  pinMode (TX_POWER2, INPUT);
  digitalWrite (TX_POWER2, LOW);

  pinMode (BLINK_PIN, INPUT);
  digitalWrite (BLINK_PIN, LOW); 
  

}

void loop() {
  // disable ADC
  ADCSRA = 0;  
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
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


void interuptFunction() {
  // Main function when we see a blink on the power meter
  // Steps to do:
  // 1. Power on transmitter, so it can warm up while we do other stuff
  // 2. Calculate time since last blink
  // 3. Add to counter for number of blinks seen
  // 4. Measure battery voltage
  // 5. Send all data


  // 1. Power on transmitter
  pinMode (TX_POWER, OUTPUT); 
  pinMode (TX_POWER2, OUTPUT);
  digitalWrite (TX_POWER, LOW); // GND of TX is connected to pins so pulling it LOW power the TX on
  digitalWrite (TX_POWER2, LOW); 


  // 2. Calculate time since last blink
  long currentTime = millis();
  long deltaTime = currentTime - lastTime;
  lastTime = currentTime;
  long watts = 123; //TODO

  
  // 3. Add to counter for number of blinks seen
  blinkCounter++;

  
  // 4. Measure battery voltage
  long battV = vccVoltage();
  
  // 5. Send all data
  String out = "{\"id\":";
  out += DEVICE_ID;
  out += ",\"b\":";
  out.concat(blinkCounter);
  out += ",\"w\":";
  out.concat(watts);
  out += ",}";

  // TODO send multiple times?
  Serial.println(out);
  
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
