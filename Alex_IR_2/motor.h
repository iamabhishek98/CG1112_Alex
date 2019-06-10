#define LF                  0b01000000   // Left forward pin 6 port d
#define LR                  0b00100000   // Left reverse pin 5 port d
#define RF                  0b00000100  // Right forward pin 10 port b
#define RR                  0b00000010  // Right reverse pin 9 port b
void setupMotors()
{
  DDRB |= (RF | RR);
  DDRD |= (LF | LR);
  TCNT0 = 0;
  OCR0A = 0;
  OCR0B = 0;
  TIMSK0 |= 0b110; // OCIEA = 1 OCIEB = 1
  TCCR0B = 0b00000011;
  /* Our motor set up is:  
   *    A1IN - Pin 5, PD5, OC0B
   *    A2IN - Pin 6, PD6, OC0A
   *    B1IN - Pin 10, PB2, OC1B
   *    B2In - pIN 11, PB3, OC2A //Change to pin 9 oc1A
   */
   TCNT1 = 0;
   OCR1A = 0;
   OCR1B = 0;
   TIMSK1 |= 0b110; // OCIEA = 1 OCIEB = 1
   TCCR1B = 0b00000011;
}
/*
 * Alex's motor drivers.
 * 
 */
static volatile int LFval;
static volatile int LRval;
static volatile int RFval;
static volatile int RRval;
 ISR(TIMER0_COMPA_vect)
{
OCR0A = LFval;
}
ISR(TIMER0_COMPB_vect)
{
OCR0B = LRval;
}
 ISR(TIMER1_COMPA_vect)
{
OCR1A = RRval;
}
ISR(TIMER1_COMPB_vect)
{
OCR1B = RFval;
}

// Set up Alex's motors. Right now this is empty, but
// later you will replace it with code to set up the PWMs
// to drive the motors.


// Start the PWM for Alex's motors.
// We will implement this later. For now it is
// blank.
void startMotors()
{
  
}

// Convert percentages to PWM values

int pwmVal(float speed)
{
  if(speed < 0.0)
    speed = 0;

  if(speed > 100.0)
    speed = 100.0;

  return (int) ( (speed / 100.0) * 255.0 );
}

// Move Alex forward "dist" cm at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// move forward at half speed.
// Specifying a distance of 0 means Alex will
// continue moving forward indefinitely.
void forward(float dist, float speed)
{
  if(dist > 0)
    deltaDist = dist;
  else
    deltaDist = 9999999;

  newDist = forwardDist + deltaDist;
  
  dir = FORWARD;
  int val = pwmVal(speed);

  // For now we will ignore dist and move
  // forward indefinitely. We will fix this
  // in Week 9.

  // LF = Left forward pin, LR = Left reverse pin
  // RF = Right forward pin, RR = Right reverse pin
  // This will be replaced later with bare-metal code.
  TCCR0A = 0b10000001;
  PORTD &= ~LR; //off LR
  LFval = val;

  TCCR1A = 0b00100001;
  PORTB &= ~RR; //off RR
  RFval = val;
  
}

// Reverse Alex "dist" cm at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// reverse at half speed.
// Specifying a distance of 0 means Alex will
// continue reversing indefinitely.
void reverse(float dist, float speed)
{
  if(dist > 0)
    deltaDist = dist;
  else
    deltaDist = 9999999;

  newDist = reverseDist + deltaDist;
  dir = BACKWARD;
  int val = pwmVal(speed);

  // For now we will ignore dist and 
  // reverse indefinitely. We will fix this
  // in Week 9.

  // LF = Left forward pin, LR = Left reverse pin
  // RF = Right forward pin, RR = Right reverse pin
  // This will be replaced later with bare-metal code.
  TCCR0A = 0b00100001;
  PORTD &= ~LF; //off LF
  LRval = val;

  TCCR1A = 0b10000001;
  PORTB &= ~RF; //off RF
  RRval = val;
}

// Turn Alex left "ang" degrees at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// turn left at half speed.
// Specifying an angle of 0 degrees will cause Alex to
// turn left indefinitely.
void left(float ang, float speed)
{
  dir = LEFT;
  int val = pwmVal(speed);

  // For now we will ignore ang. We will fix this in Week 9.
  // We will also replace this code with bare-metal later.
  // To turn left we reverse the left wheel and move
  // the right wheel forward.
  TCCR0A = 0b00100001;
  PORTD &= ~LF; //off LF
  LRval = val;

  TCCR1A = 0b00100001;
  PORTB &= ~RR; //off RR
  RFval = val;

}

// Turn Alex right "ang" degrees at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// turn left at half speed.
// Specifying an angle of 0 degrees will cause Alex to
// turn right indefinitely.
void right(float ang, float speed)
{
  dir = RIGHT;
  int val = pwmVal(speed);

  // For now we will ignore ang. We will fix this in Week 9.
  // We will also replace this code with bare-metal later.
  // To turn right we reverse the right wheel and move
  // the left wheel forward.
  TCCR0A = 0b10000001;
  PORTD &= ~LR; //off LR
  LFval = val;

  TCCR1A = 0b10000001;
  PORTB &= ~RF; //off RF
  RRval = val;

}

// Stop Alex. To replace with bare-metal code later.
void stop()
{
  dir = STOP;
  TCCR0A = 0b00000001;
  PORTD &= (~LR & ~LF); //off LR & LF

  TCCR1A = 0b00000001;
  PORTB &= ( ~RR & ~ RF ); //off RF & RF

}
