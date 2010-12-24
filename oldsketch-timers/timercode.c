#include <avr/interrupt.h>
#include <avr/io.h>

#define INIT_TIMER_COUNT 6
#define RESET_TIMER2 TCNT2 = INIT_TIMER_COUNT

int ledPin = 13;
volatile int int_counter = 0;
volatile int second = 0;
int oldSecond = 0;
long starttime = 0;
//unsigned long m = millis();

// Aruino runs at 16 Mhz, so we have 1000 Overflows per second…
// 1/ ((16000000 / 64) / 256) = 1 / 1000
ISR(TIMER2_OVF_vect) {
    RESET_TIMER2;
    int_counter += 1;
    if (int_counter == 1000) {
	second+=1;
	int_counter = 0;
    }
};

void setup() {
    Serial.begin(9600);
    Serial.println(“Initializing timerinterrupt”);
    // so we just hard code this for now.
    // Timer2 Settings: Timer Prescaler /64,
    TCCR2A |= (1<<CS22);
    TCCR2A &= ~((1<<CS21) | (1<<CS20));
    // Use normal mode
    TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
    // Use internal clock – external clock not used in Arduino
    ASSR |= (0<<AS2);
    // Timer2 Overflow Interrupt Enable
    TIMSK2 |= (1<<TOIE2) | (0<<OCIE2A);

    sei();
    //Serial.println(millis() – starttime, DEC);
    //Serial.println(“.”);
    //digitalWrite(ledPin, HIGH);
    //delay(100);
    //digitalWrite(ledPin, LOW);
    //oldSecond = second;
    //starttime = millis();
}

void loop()                     
{
    Serial.printf("counter = %d, second = %d\n", int_counter, second);
  delay(1000);                  // wait for a second
}
