#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

enum period_t
{
  SLEEP_15MS,
  SLEEP_30MS,
  SLEEP_60MS,
  SLEEP_120MS,
  SLEEP_250MS,
  SLEEP_500MS,
  SLEEP_1S,
  SLEEP_2S,
  SLEEP_4S,
  SLEEP_8S,
  SLEEP_FOREVER
};

volatile bool watchdogActivated = false;

// Define watchdog timer interrupt.
ISR(WDT_vect)
{
  // Set the watchdog activated flag.
  // Note that you shouldn't do much work inside an interrupt handler.
  watchdogActivated = true;
}

void setupSleep8() {
  // Setup the watchdog timer to run an interrupt which
  // wakes the Arduino from sleep every 8 seconds.
  
  // Note that the default behavior of resetting the Arduino
  // with the watchdog will be disabled.
  
  // This next section of code is timing critical, so interrupts are disabled.
  // See more details of how to change the watchdog in the ATmega328P datasheet
  // around page 50, Watchdog Timer.
  noInterrupts();
  
  // Set the watchdog reset bit in the MCU status register to 0.
  MCUSR &= ~(1<<WDRF);
  
  // Set WDCE and WDE bits in the watchdog control register.
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  // Set watchdog clock prescaler bits to a value of 8 seconds.
  WDTCSR = (1<<WDP0) | (1<<WDP3);
  
  // Enable watchdog as interrupt only (no reset).
  WDTCSR |= (1<<WDIE);
  
  // Enable interrupts again.
  interrupts();
}

void sleep8s() {
  // Put the Arduino to sleep.

  // ADC off
  ADCSRA &= ~(1 << ADEN);

  wdt_enable(SLEEP_8S);
  WDTCSR |= (1 << WDIE);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  cli();
  sleep_enable();   
  sleep_bod_disable(); 
  sei();
  sleep_cpu();      
  sleep_disable();    
  sei();

  ADCSRA |= (1 << ADEN);


}
