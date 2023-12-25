/*
 * Driving.c
 *
 *  Created on: Jun 8, 2016
 *      Author: os
 */
#include "derivative.h" 				// Include peripheral declarations 
# include "TFC.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

/******************************************************************************
* Global variables
******************************************************************************/
extern int turnL;
extern int turnR;
extern int EnCountL;
extern int EnCountR;
extern int dir;
extern int x;
extern int y;
extern double OpenR[2];
extern double OpenL[2];
double TurnAngle;
extern int TurnCount;
int k =0;
//-----------------------------------------------------------------
// Motor config
//-----------------------------------------------------------------
void MotorsConfig (){
	//Motor1 configurations
	PORTC_PCR5 = PORT_PCR_MUX(1); // assign PTC5 as GPIO
	GPIOC_PDDR |=PORT_LOC(5);  // PTC(5) is Output
	PORTC_PCR5 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK ;
	PORTC_PCR6 = PORT_PCR_MUX(1); // assign PTC6 as GPIO
	GPIOC_PDDR |=PORT_LOC(6);  // PTD(6) is Output
	PORTC_PCR6 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK ;
	//GPIOC_PSOR |= 0x60;
	PORTE_PCR23 = PORT_PCR_MUX(3); //assign PTE23 as TPM2_CH1

	
	//Motor2 configurations
	PORTC_PCR7 = PORT_PCR_MUX(1); // assign PTC7 as GPIO
	GPIOC_PDDR |=PORT_LOC(7);  // PTC(7) is Output
	PORTC_PCR7 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK ;
	PORTC_PCR10 = PORT_PCR_MUX(1); // assign PTC10 as GPIO
	GPIOC_PDDR |=PORT_LOC(10);  // PTC(10) is Output
	PORTC_PCR10 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK ;
//	GPIOC_PSOR |= 0x480;
	PORTE_PCR20 = PORT_PCR_MUX(3); //assign PTE20 as TPM1_CH0

	GPIOC_PSOR &= ~0X4E0; //stops the wheels
	
	TPM2_C1V = 0x3000; // motor 1 PWM 100% Right  4180
	TPM1_C0V = 0x3860; // motor 2 PWM 100% Left   4900
}

//-----------------------------------------------------------------
// Encoders Config
//-----------------------------------------------------------------
void EncodersConfig (){
	PORTC_PCR3 = PORT_PCR_MUX(4); //assign PTC3 as TPM0
	TPM0_C2SC = 0;
	TPM0_C2SC |= TPM_CnSC_ELSA_MASK + TPM_CnSC_CHIE_MASK; // TMP0 C2 input capture mode with interrupt
	PORTC_PCR4 = PORT_PCR_MUX(4); //assign PTC4 as TPM0
	TPM0_C3SC = 0;
	TPM0_C3SC |= TPM_CnSC_ELSA_MASK + TPM_CnSC_CHIE_MASK; // TMP0 C3 input capture mode with interrupt

}
/******************************************************************************
* Functions
******************************************************************************/


void Go()
{

	TPM2_C1V = 0x4A00; // motor 1 PWM 100% Right  4180   3000
	TPM1_C0V = 0x6750; // motor 2 PWM 100% Left   4900   3800 
	GPIOC_PCOR |= 0XFFF;
	GPIOC_PSOR |= 0X440;
}

void Reverse ()
{
	TPM2_C1V = 0x4500; // motor 1 PWM 100%
	TPM1_C0V = 0x4C50; // motor 2 PWM 100%
	GPIOC_PCOR |= 0XFFF;
	GPIOC_PSOR |= 0XA0;
}

void Stop()
{
	GPIOC_PCOR |= 0XFFF;
}
void TurnRight()
{
	turnR=1;
	TurnCount=0;
	TurnAngle = 91;
	dir = (dir+1)%4;
	GPIOC_PCOR |= 0XFFF;
	GPIOC_PSOR |= 0X420;
	TPM1_C1V = 0x910; //900
	while (turnR ==1) k++;
}

void TurnLeft()
{
	turnL=1;
	TurnCount=0;
	TurnAngle = 86;
	dir = (dir+3)%4;
	GPIOC_PCOR |= 0XFFF;
	GPIOC_PSOR |= 0XC0;
	TPM0_C4V = 0x700;
	while (turnL ==1) k++;
}

void TurnAngleL (double angle)
{
	turnL=1;
	TurnCount=0;
	TurnAngle = OpenR[1];
	GPIOC_PCOR |= 0XFFF;
	GPIOC_PSOR |= 0XC0;
}

void TurnAngleR (double angle)
{
	turnR=1;
	TurnCount=0;
	TurnAngle = OpenR[1];
	GPIOC_PCOR |= 0XFFF;
	GPIOC_PSOR |= 0X420;
}

void FixRight()
{
	turnR=1;
	TurnCount=0;
	TurnAngle = 91;
	dir = (dir+1)%4;
	GPIOC_PCOR |= 0XFFF;
	GPIOC_PSOR |= 0X420;
	TPM0_C4V = 0xBD0;
	while (turnR ==1) k++;
}

void FixLeft()
{
	turnL=1;
	TurnCount=0;
	TurnAngle = 87;
	dir = (dir+3)%4;
	GPIOC_PCOR |= 0XFFF;
	GPIOC_PSOR |= 0XC0;
	TPM1_C1V = 0x450;
	while (turnL ==1) k++;
}

void GoBackTo0()
{
	if (x>0)
	{
		switch (dir){
			case 0:
				FixLeft();
				waitt(5);
				break;
			case 1:
				FixLeft();
				waitt(5);
				FixLeft();
				waitt(5);
				break;
			case 2:
				FixRight();
				waitt(5);
				break;
			default:
			break;
		}
	}
	if (x<0)
	{
		switch (dir){
			case 2:
				FixLeft();
				waitt(5);
				break;
			case 3:
				FixLeft();
				waitt(5);
				FixLeft();
				waitt(5);
				break;
			case 0:
				FixRight();
				waitt(5);
				break;
			default:
			break;
		}
	}
	while (x != 0) 	Go();
	Stop();
	waitt(3);
	if (dir == 1) FixLeft();
	else FixRight();
	waitt(5);
}
