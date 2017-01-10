#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/io.h>

#define LED_BUILTIN 1 // builtin LED is on port 1
#define WDT_MAX 8 // WDT counter
int wdt_counter;

ISR(WDT_vect) {
  wdt_reset();
}

/** Blink number of times:
 *  ., .., ..., ...., -., -.., -..., -....
 */
void blink(int num) {
  if (num>8) num = 8;
  digitalWrite(LED_BUILTIN, HIGH);
  if (num > 4) {
    delay(3);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1);
    num -= 4;
  }
    
  while(num > 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1);
    digitalWrite(LED_BUILTIN, LOW);  
    delay(1);
    num--; 
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  wdt_counter = WDT_MAX;
  slowDown();
  blink(7);
}

void loop() {
  powerDownComponents();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Power down everything, wake up from WDT
  sleep_enable();
  setupWatchDog();
  sleep_mode();
  wdt_counter--;
  if (!wdt_counter) {
    wdt_disable();
    wdt_counter = WDT_MAX;
    blink(1);
  }
}

void slowDown()
{
 CLKPR=0x80; // Program clock
 // CLKPR=0x03; // Clock division factor = 8 2^3 
 CLKPR=0x08; // Clock division factor = 256 2^8
 // this slows down everything including delay(x)
 // delay(1) is suddenly delay(256)
 // The clock speed is down from 8MHz to 32Khz
}

void powerDownComponents()
{
  // PORT PINS
  // Digital Input Disable - for all non digital input pins
  DIDR0 = 0x1f; // All pins are either output or analog
  // A/D Disable
  ADCSRA=0x00;
  // Analog comparator disable
  ACSR = 0x80;
  // Modules
  PRR = 0x7;
  // BOD dectector
  MCUCR = 0x84;
  MCUCR = 0x80;
  sleep_bod_disable(); 
}

void setupWatchDog()
{
  cli();
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCR |= (1<<WDCE) | (1<<WDE);
  WDTCR = 0x21 | (1<<WDIE) | (1<<WDE);
  sei();
}

