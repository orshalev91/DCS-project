/*
 * Clocks.c
 *
 *  Created on: Jun 8, 2016
 *      Author: os
 */


#include "derivative.h" 				// Include peripheral declarations 
# include "TFC.h"
#include "mcg.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define MUDULO_REGISTER  0x927C

/******************************************************************************
* Global variables
******************************************************************************/
extern int Allow;
extern int turnL;
extern int turnR;
extern int EnCountL;
extern int EnCountR;
int TurnCount;
extern double Wheels[4];
extern double ADCMeas[3];
int TOF1; //Motor 1 TOF counter
int TOF2; // Motor 2 TOF counter
extern int dir;
extern int x;
extern int y;
extern double TurnAngle;
int TempCount; // TPM clicks count for speed calc
int EncDef;



//-----------------------------------------------------------------
// TPMx - Clock Setup
//-----------------------------------------------------------------
void ClockSetup(){
	    
	    pll_init(8000000, LOW_POWER, CRYSTAL,4,24,MCGOUT); //Core Clock is now at 48MHz using the 8MHZ Crystal
		
	    //Clock Setup for the TPM requires a couple steps.
	    //1st,  set the clock mux
	    //See Page 124 of f the KL25 Sub-Family Reference Manual
	    SIM_SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK;// We Want MCGPLLCLK/2=24MHz (See Page 196 of the KL25 Sub-Family Reference Manual
	    SIM_SOPT2 &= ~(SIM_SOPT2_TPMSRC_MASK);
	    SIM_SOPT2 |= SIM_SOPT2_TPMSRC(1); //We want the MCGPLLCLK/2 (See Page 196 of the KL25 Sub-Family Reference Manual
		//Enable the Clock to the TPM0 and PIT Modules
		//See Page 207 of f the KL25 Sub-Family Reference Manual
		SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK + SIM_SCGC6_TPM1_MASK + SIM_SCGC6_TPM2_MASK;
	    // TPM_clock = 24MHz , PIT_clock = 24MHz
	    
}

//-----------------------------------------------------------------
// TPMx - Initialization
//-----------------------------------------------------------------
void InitTPM(){ 
// TMP0
		TPM0_SC = 0; // to ensure that the counter is not running
		TPM0_SC |= TPM_SC_PS(4); //Prescaler =64, up-mode, counter-disable
		TPM0_MOD = MUDULO_REGISTER; // PWM frequency of 40Hz = 24MHz/(16x37,500)
		TPM0_C1SC |= TPM_CnSC_MSB_MASK + TPM_CnSC_ELSB_MASK;// + TPM_CnSC_CHIE_MASK;
		TPM0_C1V = 0;
		TPM0_C4SC |= TPM_CnSC_MSB_MASK + TPM_CnSC_ELSB_MASK ;//+ TPM_CnSC_CHIE_MASK; //Servo 2
		TPM0_C4V = 0xBD0;  // Servo 2 PWM   RIGHT on 0 degrees CE4  354  C95
		TPM0_CONF = 0; 
		TPM0_SC |= TPM_SC_CMOD(1); // enable TPM0
		enable_irq(INT_TPM0-16);
		set_irq_priority(INT_TPM0-16,0);  // Interrupt priority = 0 = max
		
//TMP1
		TPM1_SC = 0; // to ensure that the counter is not running
		TPM1_SC |= TPM_SC_PS(4); //Prescaler =64, up-mode, counter-disable
		TPM1_MOD = MUDULO_REGISTER; // PWM frequency of 40Hz = 24MHz/(16x37,500)
		TPM1_C0SC |= TPM_CnSC_MSB_MASK + TPM_CnSC_ELSB_MASK ;//+ TPM_CnSC_CHIE_MASK;  //Motor 2
		TPM1_C0V = 0x493E; //Motor 2 PWM on 50%
		TPM1_C1SC |= TPM_CnSC_MSB_MASK + TPM_CnSC_ELSB_MASK ;//+ TPM_CnSC_CHIE_MASK;
		TPM1_C1V = 0x450;  // Servo 1 PWM  LEFT on 180 degrees   CE4   385
		TPM1_CONF = 0; 
		TPM1_SC |= TPM_SC_CMOD(1); // enable TPM1 

//TMP2
		TPM2_SC = 0; // to ensure that the counter is not running
		TPM2_SC |= TPM_SC_PS(4); //Prescaler =64, up-mode, counter-disable
		TPM2_MOD = MUDULO_REGISTER; // PWM frequency of 40Hz = 24MHz/(16x37,500)
		//TPM2_C0SC |= TPM_CnSC_MSB_MASK + TPM_CnSC_ELSB_MASK ;//+ TPM_CnSC_CHIE_MASK;
		//TPM2_C0V = 0x1250; 
		TPM2_C1SC |= TPM_CnSC_MSB_MASK + TPM_CnSC_ELSB_MASK ;//+ TPM_CnSC_CHIE_MASK; //Motor 1
		TPM2_C1V = 0x493E;  // Motor 1 PWM on 50%
		TPM2_CONF = 0;
		TPM2_SC |= TPM_SC_CMOD(1); // enable TPM2
		
}
//-----------------------------------------------------------------
// PIT - Initialisation
//-----------------------------------------------------------------
void InitPIT(){
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK; //Enable the Clock to the PIT Modules
	// Timer 0
	PIT_MCR = PIT_MCR_MDIS_MASK; // Turn off PIT
	PIT_LDVAL0 = 0x17700; // setup timer 0 for 4msec counting period
	PIT_TCTRL0 = PIT_TCTRL_TIE_MASK; //enable PIT0 and its interrupt
	PIT_LDVAL1 = 0x16E360; // setup timer 1 for 60msec counting period
	//PIT_MCR |= PIT_MCR_FRZ_MASK; // stop the pit when in debug mode
	enable_irq(INT_PIT-16); //  //Enable PIT IRQ on the NVIC
	set_irq_priority(INT_PIT-16,0);  // Interrupt priority = 0 = max
	PIT_MCR = 0;

}

void delay (int i){
	PIT_MCR = PIT_MCR_MDIS_MASK; // Turn off PIT
	PIT_LDVAL0 = i* 0x5dc0; // setup timer 0 for i*1msec counting period
	PIT_MCR = 0;
	PIT_TCTRL0 = PIT_TCTRL_TEN_MASK;
	while (!PIT_TFLG0);
	PIT_CVAL0 = 0;
	PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK; 
}
void waitt (int i)
{
	int wait;
	for (wait = 0;wait<100000*i;wait++);

}
/******************************************************************************
* ISRs
******************************************************************************/


//------------------------------------------------------------
// PIT - ISR
//-------------------------------------------------------------
void PIT_IRQHandler(){
	if (PIT_TFLG0){
		Allow =  1;
		PIT_TFLG0 = PIT_TFLG_TIF_MASK; //Turn off the Pit 0 Irq flag 
		PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK & ~PIT_TCTRL_TIE_MASK; // Turn off PIT0
		PIT_CVAL0 = 0;
		}

}


//------------------------------------------------------------
// TPM0 - ISR
//-------------------------------------------------------------

void FTM0_IRQHandler(void){
	if(TPM0_STATUS & 0x02){
		TPM0_C1V = 0;
		TPM0_C1SC |= TPM_CnSC_CHF_MASK;
	}
	if(TPM0_STATUS & 0x04){ //Motor 1   Right Encoder
		if (turnR !=1 && turnL !=1) EnCountR= (EnCountR+1);//%8000;
		else TurnCount++;
		
		if (TurnCount >= (int)(TurnAngle*2.5) && turnR==1){  // Turning
			GPIOC_PCOR |= 0xFFFF;
			turnR = 0;
		}


		TPM0_C2SC |= TPM_CnSC_CHF_MASK;
	}
	if(TPM0_STATUS & 0x08){ //Motor 2  Left Encoder
		if (turnR !=1 && turnL !=1) EnCountL= (EnCountL+1);//%8000;
		
		if (TurnCount >= (int)(TurnAngle*3.25) && turnL==1){  // Turning
			GPIOC_PCOR |= 0xFFFF;
			turnL = 0;
		}
		if (turnR !=1 && turnL !=1){ // place in arena calculation
			switch (dir){
				case 0:
					y++;	
					break;
				case 1:
					x++;
					break;
				case 2:
					y--;
					break;
				case 3:	
					x--;
					break;
			}
		}
		TPM0_C3SC |= TPM_CnSC_CHF_MASK;

	}
}
