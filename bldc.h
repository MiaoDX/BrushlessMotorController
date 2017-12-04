#include <Arduino.h>
#include <avr\interrupt.h>
#include "definitions.h"

// Hardware Abstraction for Motor connectors,
// DO NOT CHANGE UNLESS YOU KNOW WHAT YOU ARE DOING !!!
//aktivering av pwm(OCR = Output Compare Register)

#define PWM_A_MOTOR1 OCR2A
#define PWM_B_MOTOR1 OCR1B
#define PWM_C_MOTOR1 OCR1A

#define PWM_A_MOTOR0 OCR0A
#define PWM_B_MOTOR0 OCR0B
#define PWM_C_MOTOR0 OCR2B

#define N_SIN 256

uint8_t pwm_a_motor0 = 128;
uint8_t pwm_b_motor0 = 128;
uint8_t pwm_c_motor0 = 128;

uint8_t pwm_a_motor1 = 128;
uint8_t pwm_b_motor1 = 128;
uint8_t pwm_c_motor1 = 128;

uint16_t freq_1_count = 0; 
uint16_t freq_0_count = 0;
volatile uint16_t motor_0_freq = 256;
volatile uint16_t motor_1_freq = 96;
volatile int motor_0_update = 0;
volatile int motor_1_update = 0;
static int8_t pwmSinMotor[N_SIN];

/*Initialize motorcontroller
Setter pinner og velger frekvens*/
void initBlController()
{
	pinMode(3, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);

#ifdef PWM_8KHZ_FAST
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM10);
	TCCR0B = _BV(CS01);
	TCCR1A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	TCCR2A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(CS21);
#endif

#ifdef PWM_32KHZ_PHASE
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);
	TCCR0B = _BV(CS00);
	TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(CS10);
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20);
	TCCR2B = _BV(CS20);
#endif

#ifdef PWM_4KHZ_PHASE
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);
	TCCR0B = _BV(CS01);
	TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(CS11);
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20);
	TCCR2B = _BV(CS21);
#endif

	// enable Timer 1 interrupt
	TIMSK1 |= _BV(TOIE1);

	// disable arduino standard timer interrupt
	TIMSK0 &= ~_BV(TOIE1);

	sei();

	// Enable Timer1 Interrupt for Motor Control
	OCR2A = 0;  //11  APIN
	OCR2B = 0;  //D3
	OCR1A = 0;  //D9  CPIN
	OCR1B = 0;  //D10 BPIN
	OCR0A = 0;  //D6
	OCR0B = 0;  //D5 
}

void calcSinusArray()
{
    int i;
	for (i = 0; i<N_SIN; i++)
	{
		pwmSinMotor[i] = sin(2.0 * i / N_SIN * 3.14159265) * 127.0;
	}
}

void initMotorStuff()
{
	cli();
	motor_0_freq = CC_FACTOR * 1000 / MOTORUPDATE_FREQ;
	motor_1_freq = CC_FACTOR * 1000 / MOTORUPDATE_FREQ;
	calcSinusArray();
	sei();
}

void MoveMotorPosSpeed(uint8_t motorNumber, int MotorPos, uint16_t maxPWM)
{
	uint16_t posStep;
	int16_t pwm_a;
	int16_t pwm_b;
	int16_t pwm_c;

	// fetch pwm from sinus table
	posStep = MotorPos & 0xff;
	pwm_a = pwmSinMotor[(uint8_t)posStep];
	pwm_b = pwmSinMotor[(uint8_t)(posStep + 85)];
	pwm_c = pwmSinMotor[(uint8_t)(posStep + 170)];

	// apply power factor
	
	pwm_a = maxPWM * pwm_a;
	pwm_a = pwm_a >> 8;
	pwm_a += 128;
		
	pwm_b = maxPWM * pwm_b;
	pwm_b = pwm_b >> 8;
	pwm_b += 128;
		
	pwm_c = maxPWM * pwm_c;
	pwm_c = pwm_c >> 8;
	pwm_c += 128;

	// set motor pwm variables
	if (motorNumber == 0)
	{
		pwm_a_motor0 = (uint8_t)pwm_a;
		pwm_b_motor0 = (uint8_t)pwm_b;
		pwm_c_motor0 = (uint8_t)pwm_c;
	}

	if (motorNumber == 1)
	{
		pwm_a_motor1 = (uint8_t)pwm_a;
		pwm_b_motor1 = (uint8_t)pwm_b;
		pwm_c_motor1 = (uint8_t)pwm_c;
	}
}

void motorPowerOff(int motorNum) 
{
	/*Sl�r av power og setter farten/pos til 0*/
	MoveMotorPosSpeed(motorNum, 0, 0);
}

/* intrrupt */
ISR(TIMER1_OVF_vect)
{
	if(!motor_0_update) freq_0_count++;
	if (freq_0_count == motor_0_freq)
	{
		PWM_A_MOTOR0 = pwm_a_motor0;
		PWM_B_MOTOR0 = pwm_b_motor0;
		PWM_C_MOTOR0 = pwm_c_motor0;

		freq_0_count = 0;
		motor_0_update = 1;
	}

	if (!motor_1_update) freq_1_count++;
	if (freq_1_count == motor_1_freq)
	{
		PWM_A_MOTOR1 = pwm_a_motor1;
		PWM_B_MOTOR1 = pwm_b_motor1;
		PWM_C_MOTOR1 = pwm_c_motor1;

		freq_1_count = 0;
		motor_1_update = 1;
	}
}

const float Width = 2048.0f; 
static float motor0_pos = 0;
static float motor1_pos = 0;

static float motor0_speed = 0;
static float motor1_speed = 0;

static int motor0_power = 0;
static int motor1_power = 0;

void setMotorMode(int ctl_motor_num,float ctl_motor_speed,int ctl_motor_power){
	if(ctl_motor_num == 0){
		motor0_speed = ctl_motor_speed;
		motor0_power = ctl_motor_power;
	}
	if(ctl_motor_num == 1){
		motor1_speed = ctl_motor_speed;
		motor1_power = ctl_motor_power;
	}
}
void moveMotorWithSpeed(){
	if(motor_0_update){
		motor_0_update = 0;
		motor0_pos += motor0_speed;		
		if (motor0_pos >= Width) motor0_pos -= Width;
		else if (motor0_pos < 0) motor0_pos += Width;

        if(motor0_power){
		    MoveMotorPosSpeed(0,(int)motor0_pos, motor0_power);
        }else{
            motorPowerOff(0);
        }
	}
	if(motor_1_update){
		motor_1_update = 0;
		motor1_pos += motor1_speed;
		if(motor1_pos >= Width) motor1_pos -= Width;
		else if(motor1_pos < 0) motor1_pos += Width;

		if(motor1_power){
            MoveMotorPosSpeed(1,(int)motor1_pos, motor1_power);
        }else{
            motorPowerOff(1);
        }
	}
}
