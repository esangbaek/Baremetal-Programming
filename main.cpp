#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include "motordriver.h"
#include "DHT22.h"
#include <stdio.h>

#define uAs 0.004291
#define uB 0.004065
#define C 0.003831
#define Cs 0.00361
#define D 0.003412
#define Ds 0.003215
#define E 0.003039
#define F 0.002865
#define Fs 0.00271
#define G 0.002557
#define Gs 0.002409
#define A 0.002272
#define As 0.002145
#define B 0.002028
#define CC 0.001912
#define CCs 0.001805
#define DD 0.001703
#define DDs 0.001607
#define EE 0.001517
#define FF 0.001432
#define FFs 0.001353
#define GG 0.001277
#define GGs 0.001204
#define AA 0.001136

float melody1[]={Fs,0,Fs,CCs,B,0,A,0,Gs,0,Gs,Gs,B,0,A,Gs,Fs,0,Fs,AA,GGs,AA,GGs,AA,Fs,0,Fs,AA,GGs,AA,GGs,AA};	//0.2
	
float melody2[]={DD,CC,As,A,As,CC,DD,A,As,A,G,F,G,A,F,D,DD,CC,As,A,As,CC,DD,A,F,G,D,G,A,0};
int melody2bit[]={1,1,1,1,1,1,4,2,1,1,1,1,1,1,4,2,1,1,1,1,1,1,2,2,2,5,1,4,1,1};	//*0.2
	
float melody3[]={uB,Ds,E,G,B,A,Fs,A,G,Fs,E,Ds,E,G,Fs,E,Ds,E,G,Fs,E,uB,uAs,uB,Cs,Ds,G,Ds,Fs,E};	
int melody3bit[]={6,2,2,2,5,1,6,2,2,2,2,3,1,6,6,6,2,2,2,5,1,6,2,2,2,2,3,1,6,6};	//*0.2
	
//Actuator
Motor mt(D11, PC_8);
DHT22 sensor (PB_2);
DigitalIn motion(D5);
volatile int detect = 0;
	
class I2CPreInit : public I2C{
public:
	I2CPreInit(PinName sda, PinName scl) : I2C(sda,scl){
		frequency(400000);
		start();
	};
};
I2C myI2C(I2C_SDA, I2C_SCL);
Adafruit_SSD1306_I2c OLED(myI2C, D13, 0x78, 64, 128);

PwmOut sound(PC_9);
InterruptIn prev(PA_14);
InterruptIn stop_or_play(PB_7);
InterruptIn next(PC_4);

//Interrupt
int playlist=0;	//music order
int playing=0;	//play or stop
Timeout stopfive;	//stop after 5 seconds
Ticker thermo_check;	//Thermometer

void prev_music(){
	mt.backward(0.3);
	wait(0.3);
	mt.stop();
	playlist = playlist==0 ? 2 : playlist-1;
}

void next_music(){
	mt.forward(0.3);
	wait(0.3);
	mt.stop();
	playlist = (playlist+1)%3;
}

void music_stop(){
	sound.period(0);
	playing=0;
}

void gostop(){
	if(playing){
		stopfive.attach(&music_stop,5.0);
	}
	else{
		playing=1;
	}
}

void check(){
	float h=0.0f, c=0.0f;
	int temp, humid;
	int detect = motion;
	sensor.sample();
	
	c=sensor.getTemperature();
	h=sensor.getHumidity();
	temp = ((int)c)/10;
	humid = ((int)h)/10;
	OLED.setTextCursor(0,0);
	OLED.clearDisplay();
	OLED.printf("Temp:%d Humid:%d\r\n",temp,humid);
	OLED.display();
	
	if(detect){
		OLED.printf("Human detected\r\n",temp,humid);
		OLED.display();
	}else{
		OLED.printf("No human\r\n",temp,humid);
		OLED.display();
	}
}

//melody collection
void song1(){
	for(int i=0;i<32;i++){
		if(playing==1 && playlist==0){
				sound.period(melody1[i]);
				sound=0.5;
				wait(0.2);
		}
		else
			break;
	}
}

void song2(){
	for(int i=0;i<30;i++){
		if(playing==1 && playlist==1){
				sound.period(melody2[i]);
				sound=0.5;
				wait(melody2bit[i]*0.2);
		}
		else
			break;
	}
}

void song3(){
	for(int i=0;i<30;i++){
		if(playing==1 && playlist==2){
				sound.period(melody3[i]);
				sound=0.5;
				wait(melody3bit[i]*0.2);
		}
		else
			break;
	}
}

//main function
int main(){
	//Interrupt setup
	prev.rise(&prev_music);
	stop_or_play.rise(&gostop);
	next.rise(&next_music);
	//Ticker setup
	thermo_check.attach(&check, 2.0);
	
	while(1){
		if(playing){
			switch(playlist){
				case 0:	
					song1();
					break;
				case 1:
					song2();
					break;
				case 2:
					song3();
					break;
			}
		}
	}
}