#include <Wire.h>

#include "bldc.h"
#include "parseCmd.h"
#include "definitions.h"
#define CMD_HEADER 0x24 // ascii "$"
void setup()
{
	Serial.begin(BAUDRATE);
	initBlController();
	initMotorStuff();
	motorPowerOff(0);
  motorPowerOff(1);
  delay(1000);
	sei();
}
byte mode_byts[5];
int ctl_motor_num = 0 ;
float ctl_motor_speed = 0;
int ctl_motor_power = 0;
int parseBytes(){    
	ctl_motor_num = parseInt2(mode_byts[0]) - '0';
	if(ctl_motor_num !=0 && ctl_motor_num != 1) return -1;
	ctl_motor_speed = parseSInt2(mode_byts[1])/16.0f;
	ctl_motor_power = parseInt2(mode_byts[2]);
	return 0;
}
byte check_byte = 0;
void loop()
{
	if(Serial.available() >= 5){
    	if(CMD_HEADER == Serial.read()){       
			Serial.readBytes(mode_byts,4);
			check_byte = 0;
			for(int i=0;i<4;i++){
				check_byte ^= mode_byts[i];
			}
			if(check_byte == 0 && parseBytes() == 0){
//        Serial.print("We got:");
//        Serial.println(ctl_motor_num);
//        Serial.println(ctl_motor_speed);
//        Serial.println(ctl_motor_power);
				setMotorMode(ctl_motor_num,ctl_motor_speed,ctl_motor_power);
			}else{
                //Serial.print('$');
                Serial.println("Failed in the check");
	        }
		}else{
			//Serial.write('$');
		}
	}
	moveMotorWithSpeed();
}

