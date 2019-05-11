/*
 * DigitalClock.c
 * Completed: 4/11/2019 16:00
 * Author : Ayan Sengupta and Achuthan Panikath
 
 * Course: ECE231, UMass Amherst
 *
 * Usage:
 *  Enter the current time in initial time as a 24 hour clock
 *	Run the program .
 *	Watch result on board LED
 *
 * Revision history:
 * 1.0 Initial Version setup files
	1.1 Made main method, initializer and delay functions
	1.2 Updated auxiliary functions - send, digitselect, sequenceselect and delay
	1.3 Removed delay to implement interrupt flag logic
	1.4 Bugs addressed- Accidental RTC count reinitialization
					  - Incorrect bit sequence for digits
					  - Using hours and minutes without initialization
	1.5 Documentation added
 *
 * 
 *
 * Note: 11 April 2019 - tested and ready for demo
 *
 *
 *
 * References: This program utilizes the Atmega 817 datasheet
 */ 

#include <avr/io.h>
//method used to initialize SPI and CTRLA
void initializer(){
	//CTRLA and CNT are bit masked to be set up as per datasheet
	//Prescaled to 1Hz
	RTC.CTRLA = RTC.CTRLA | 0b11111001;
	RTC.CNT = RTC_CNT & 0b00000000;
	PORTA.DIR = 0xff; //All ports are enabled on

	//SPI is setup
	SPI0.CTRLA = 0b01100111; //enabled, LSB first and Master
	SPI0.CTRLB = 0b10000100; //enables a buffer
	SPI0.INTCTRL = 0b11110001; //enables interrupt control
 
}

//Method that uses interrupt flags to delay until data is fully passed
void interrupt(){
	//while loop is true as long as bits are being passed
	while(!(SPI0.INTFLAGS & (1<<6))){}
		SPI0.INTFLAGS |= SPI_TXCIF_bm;//resets flag
}
//method to select the digit on the clock display and the value passed to it
//the data is pushed in but not activated
void digitselect(int n){
	switch(n){//n is used to select the digit to display number
		case 1:
		SPI0.DATA = 0b01000000;
		break;
		case 2:
		SPI0.DATA = 0b00100000;
		break;
		case 3:
		SPI0.DATA = 0b00001000;
		break;
		case 4:
		SPI0.DATA = 0b00000100;
		break;
	}
}
//function to push sequence for the given digit as per datasheet
void sequenceselect(int value){
	switch(value){
		case 0:
		SPI0.DATA = 0b11111100;
		break;
		case 1:
		SPI0.DATA = 0b01100001;
		break;
		case 2:
		SPI0.DATA = 0b11011011;
		break;
		case 3:
		SPI0.DATA = 0b11110010;
		break;
		case 4:
		SPI0.DATA = 0b01100111;
		break;
		case 5:
		SPI0.DATA = 0b10110111;
		break;
		case 6:
		SPI0.DATA = 0b10111111;
		break;
		case 7:
		SPI0.DATA = 0b11100001;
		break;
		case 8:
		SPI0.DATA = 0b11111111;
		break;
		case 9:
		SPI0.DATA = 0b11100111;
		break;
	}
}
//method that implements the repeated and sequential update of digits
void send(int ht,int ho,int mt,int mo){
	//each clock digit is updated sequentially
	PORTA.OUTCLR = PIN4_bm;
	digitselect(1); //digit 1 is selected
	interrupt();//waits for completion
	sequenceselect(ht);//selecting segments
	interrupt();
	//for(int i=0;i<50;i++){} tested with delays
	PORTA.OUTTGL= PIN4_bm;//rising clock edge
	//for(int i=0;i<50;i++){}
	
	PORTA.OUTCLR = PIN4_bm;
	digitselect(2);
	interrupt();
	sequenceselect(ho);
	interrupt();
//	for(int i=0;i<50;i++){}
	PORTA.OUTTGL= PIN4_bm;
	//for(int i=0;i<50;i++){}

	PORTA.OUTCLR = PIN4_bm;
	digitselect(3);
	interrupt();
	sequenceselect(mt);
	interrupt();
	//for(int i=0;i<50;i++){}
	PORTA.OUTTGL= PIN4_bm;
	//for(int i=0;i<50;i++){}
		
	PORTA.OUTCLR = PIN4_bm;
	digitselect(4);
	interrupt();  
	sequenceselect(mo);
	interrupt();
	//for(int i=0;i<50;i++){}
	PORTA.OUTTGL= PIN4_bm;
	//for(int i=0;i<50;i++){}
}
//code to pass the digit selection bits to the SPI0


int main(void)
{
	initializer();//initializes setup
	int initialTime = 1801;//enter the current time here
	
	//breaks down entered time into digits
	int minutes=initialTime%100;
	int minute_tens= (int)(minutes/10);
	int minute_ones= minutes%10;
	
	int hours=(int)initialTime/100;
	int hour_tens = (int)(hours/10);
	int hour_ones = hours%10;
	
	//loop that controls the repeated and sequential update
	while (1)
	{		
		//if block words when each round of 60 seconds has passed
		if (RTC.CNT==60){
			RTC.CNT = 0x00;//reinitialized
			minutes= minutes+1;
			minute_ones = minutes%10;
			minute_tens = (int) minutes/10;
		}
		if(minutes==60){//to update hours
			minutes=0;
			hours=hours+1;
			hour_ones = hours%10;
			hour_tens = (int)hours/10;
		}
		if(hours==24){//special case when clock needs to be reset at the end of 24 hours
			hours=0;
			minutes=0;
			hour_ones = 0;
			hour_tens = 0;
			minute_ones = 0;
			minute_tens = 0;
		}
		//repeatedly sends the data in each clock cycle
		send(hour_tens, hour_ones, minute_tens, minute_ones);
	}
}

