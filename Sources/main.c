/******************************************************************************
*
* This code has been written for the Freescale FRDM-KL25Z board and 
* demonstrates how to read the acceleration data from the MMA8451Q 
* using an interrupt technique.
* 
* The MMA8451Q is set to achieve the best noise performance: 
* ODR = 1.56Hz, High Resolution Mode, LNOISE bit set 
******************************************************************************/
#include "derivative.h" 				// Include peripheral declarations 
# include "TFC.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

/******************************************************************************
* Constants and macros
******************************************************************************/

#define SpeedCalc             1884955.592  //2*PI*R * 24M / 408 pre calculation for the speed
#define MUDULO_REGISTER  0x927C
#define PI 3.14159265

/******************************************************************************
* Global variables
******************************************************************************/

int Menu = 0;
uint8_t Temp;
int Allow = 1; //for 4ms counting stop
double IRDist[14] = { 2.47, 1.99, 1.57, 1.28, 1.08, 0.93, 0.82, 0.74, 0.68, 0.63, 0.58, 0.53, 0.51, 0.47 };
double ADCMeas[3]; // 0-IRLeft, 1-IRRight, 3-Battery
float RealMeas[7]; //0-IRLeft, 1-IRright, 2-Battery , 3-USLeft, 4- USRight, 5- Left Wheel Speed, 6- Right Wheel Speed
int DTMFInput; 
int readyy =0;
int turnL = 0;
int turnR = 0;
int EnCountL = 0;
int EnCountR = 0;
int SerRotate=0;
int ArenaX = 22; // arena width
int ArenaY = 0; // arena length
int x = 5060;
int y = 230;
int dir = 0; //0 foward (+y), 1 right (+x) , 2 backwards (-y), 3 left (-x) 
char Map [42][58];
int xmap;
int ymap;
int exe;
int UartInt;
char recived[4];
extern double samp[2][23];
extern double OpenR[2];
extern double OpenL[2];
extern int EncDef;
int status= 1; // 1 scaning, 2 passing the circle, 3 reaching to the end
int TempDist;
int TurnDist;
int TurnClock=0;
int a;
int name = 0;
int extractfile = -1;
struct FileLib Lib;


/******************************************************************************
* Functions
******************************************************************************/

void MCU_Init(void);
void Accelerometer_Init (void);
void Calibrate(void);

/******************************************************************************
* Main
******************************************************************************/  

int main (void){
  	ClockSetup();
  	InitTPM();
  	InitPIT();
  	InitUARTs();  	  
  	InitGPIO();
  	MotorsConfig ();
  	EncodersConfig ();
  	ServosConfig ();
  	DistSensConfig ();
  	ColorSensConfig ();
  	BattLedConfig ();
  	DTMFConfig();
  	adc_init();
	int k=0;
	int place;
	recived[3] = '$';
	ADC0_SC1A = ADC_SC1_ADCH(7) | ADC_SC1_AIEN_MASK;
	k=1;   //disable k=1 and status =4 for the car to drive in the arena
	status = 4;
	while (k==0){   //car doesnt start untill the left IR sensor see something 30cm ahead
		if(ADCMeas[0]>=1.8) k=1;
	}
	while(k==1){
		Go();
		while( status == 1){  //searching for opening
			ScanForOpen();
			waitt(5);
			TurnAngleR();  //Turning right to the open in the wall
			waitt(8);
			status=2;
		}
		while(status==2){  //exiting the circle
			EnCountR=0;
			waitt(2);
			for (a=0; a<13; a++){  //wait loop to pass the half circle
				Go();
				Map[(int)((5060+EnCountR*sin(OpenR[1]*PI/180))/230)][(int)((230+EnCountR*cos(OpenR[1]*PI/180))/230)] = 'V';
				waitt(5);
			}
			Map[(int)((5060+EnCountR*sin(OpenR[1]*PI/180))/230)][(int)((230+EnCountR*cos(OpenR[1]*PI/180))/230)] = 'V';
			Stop();
			waitt(2);
			TurnAngleL();  //turning left to the exit direction
			waitt(7);  
			x= 5060+EnCountR*sin(OpenR[1]*PI/180); //set the location after the circle
			y= 230+EnCountR*cos(OpenR[1]*PI/180);
			EnCountR=0;
			EnCountL=0;
			status=3 ;
		}
		while(status==3){ //passing objects
			Map[(int)(x/230)][(int)(y/230)] = 'V';
			Go();
			if((ADCMeas[0]>=2.3 || ADCMeas[1]>=2.3) && x>=5060){
				Map[(int)(x/230)][(int)(y/230)+2] = '#';
				Stop();
				waitt(5);
				TurnLeft();
				waitt(7);
				TurnDist = ADCMeas[1];
				Go();
				while(ADCMeas[1]>=1.8){
					Map[(int)(x/230)][(int)(y/230)+2] = '#';
					Map[(int)(x/230)][(int)(y/230)] = 'V';
					TurnClock++;
					if (TurnClock==500){ // getting further or closer to the object 
						if (TurnDist>ADCMeas[1]) TPM2_C1V -= 0x50; 
						else TPM2_C1V += 0x50; 
						TurnClock=0;
					}
					TurnClock=0;
				}
				waitt(5);
				Map[(int)(x/230)][(int)(y/230)] = 'V';
				waitt(5);
				Map[(int)(x/230)][(int)(y/230)] = 'V';
				waitt(5);
				Stop();
				waitt(5);
				FixRight();
				waitt(8);
			}
			if((ADCMeas[0]>=2.3 || ADCMeas[1]>=2.3) && x < 5060){
				Stop();
				Map[(int)(x/230)][(int)(y/230)+2] = '#';
				MapUpdate ((int)(x/230), (int)(y/230+2), '#');
				waitt(5);
				TurnRight();
				waitt(7);
				TurnDist = ADCMeas[0];
				Go();
				while(ADCMeas[0]>=1.8){
					TurnClock++;
					Map[(int)(x/230)][(int)(y/230)+2] = '#';
					MapUpdate ((int)(x/230), (int)(y/230+2), '#');
					Map[(int)(x/230)][(int)(y/230)] = 'V';
					MapUpdate ((int)(x/230), (int)(y/230), 'V');
					if (TurnClock==500){  // getting father or closer to the object 
						if (TurnDist>ADCMeas[0]) TPM2_C1V += 0x50; 
						else TPM2_C1V -= 0x50; 
						TurnClock=0;
					}
					TurnClock=0;
				}
				waitt(5);
				Map[(int)(x/230)][(int)(y/230)] = 'V';
				waitt(5);
				Map[(int)(x/230)][(int)(y/230)] = 'V';
				waitt(5);
				Stop();
				waitt(5);
				FixLeft();
				waitt(8);
			}
			if (y>= 14000){  //End of the area
				status=4;
				Stop();
				for (ymap=57; ymap>=0;ymap--){
					for (xmap=0; xmap<=41; xmap++){
					   while(!(UART0_S1 & UART_S1_TDRE_MASK));
					   UART0_D = Map[xmap][ymap];
					}
					 UARTprintf(UART0_BASE_PTR,"\r\n");
				}
			}
			if (EnCountR==450){
				if (EnCountR-EnCountL>50) 	TPM2_C1V -= 0x30;//= TPM2_C1V*0.995; // motor 1 PWM 100%
				if (EnCountL-EnCountR>50)	TPM2_C1V += 0x30;//= TPM2_C1V*1.01;
				EnCountR=0;
				EnCountL=0;
			}
			if(Temp==5) status=4;
		}

		if (recived[3] != '$'){
				exe = Decode(recived);	
				switch (exe){
						case 's'+'r'+'0'+'0':
							RED_LED_ON;
							exe = 0;
							break;
						case 's'+'b'+'0'+'0':
							BLUE_LED_ON;
							exe = 0;
							break;
						case 's'+'g'+'0'+'0':
							GREEN_LED_ON;
							exe = 0;
							break;
						case 'r'+'o'+'0'+'0':
							RED_LED_OFF;
							exe = 0;
							break;
						case 'g'+'o'+'0'+'0':
							GREEN_LED_OFF;
							exe = 0;
							break;
						case 'b'+'0'+'0'+'0':
							BLUE_LED_OFF;
							exe = 0;
							break;
						case 'l'+'o'+'0'+'0':
							RGB_LED_OFF;
							exe = 0;
							break;
						case 'r'+'t'+'0'+'0':
							__thumb_startup();
							break;
						case 'p'+'r'+'0'+'0':
							for (ymap=57; ymap>=0;ymap--){
								for (xmap=0; xmap<=41; xmap++){
								   while(!(UART0_S1 & UART_S1_TDRE_MASK));
								   if (Map[xmap][ymap]=='V') MapUpdate (xmap, ymap, 'V');
								   if (Map[xmap][ymap]=='#') MapUpdate (xmap, ymap, '#');
								}
							}
						break;
						default:
							BLUE_LED_ON;
							if (exe >= 1000) delay (exe);
							PIT_TFLG0 = PIT_TFLG_TIF_MASK; //Turn off the Pit 0 Irq flag 
							BLUE_LED_OFF;
							exe = 0;
							break;
				}			
			}
		if (UartInt <0 ) {
			UartInt *= -1;
			Uart0_Br_Sbr(48000000/2/1000, UartInt);
			UartInt = 0;
		}
		/*else if (UartInt > 0){
			Lib.Filenum[Lib.numoffiles].text = (char *) malloc(UartInt);
			Lib.Filenum[Lib.numoffiles].size = UartInt;
			UartInt = 0;
		}*/
		/*if (extractfile >= 0){
			int size;
			for (size =0; size <Lib.Filenum[extractfile].size; size++){
				UARTprintf(UART0_BASE_PTR,"&");
				UARTprintf(UART0_BASE_PTR,Lib.Filenum[extractfile].text);
			}
			extractfile = 0;

		}*/
		switch (DTMFInput){
				case 1:
					ScanForOpen();
					break;
				case 2:
					Go();
					break;
				case 5:
					Stop();
					break;
				case 4:
					TurnLeft();
					break;
				case 6:
					TurnRight();
					break;
				case 7:
					FixLeft();
					Temp=0;
					break;
				case 8:
					 Reverse();
					break;
				case 9:
					FixRight();
					Temp=0;
					break;
				default:
					break;
			}
			DTMFInput = 0;
			switch (Temp){ 
				case '1':
					ScanForOpen();
					waitt(4);
					Temp=0;
					break;
				case '2':
					Go();
					waitt(110);
					Stop();
					waitt(5);
					x= x+3150*sin(PI/6);
					Temp = 0;
					break;
				case '3':
					for (ymap=57; ymap>=0;ymap--){
						for (xmap=0; xmap<=41; xmap++){
						   while(!(UART0_S1 & UART_S1_TDRE_MASK));
						   UART0_D = Map[xmap][ymap];
						}
						 UARTprintf(UART0_BASE_PTR,"\r\n");
					}
					Temp=0;
					break;
				case '4':
					UARTprintf(UART0_BASE_PTR,"\r\n");
					PrintDouble(EnCountR);
					PrintDouble(EnCountL);
					PrintDouble(TPM2_C1V);
					PrintDouble(TPM1_C0V);
					Temp = 0;
					break;
				case '6':
					Menu = 0;
					RealMeas[2] = (ADCMeas[2]-0.1437931034)*(6.904761905); 
					UARTprintf(UART0_BASE_PTR,"\r\nBattery Voltage:  ");
					PrintDouble(RealMeas[2]);
					PrintDouble(ADCMeas[2]);
					Temp = 0;
					break;
				case '7':
					for (ymap=57; ymap>=0;ymap--){
						for (xmap=0; xmap<=41; xmap++){
						   while(!(UART0_S1 & UART_S1_TDRE_MASK));
						   if (Map[xmap][ymap]=='V') MapUpdate (xmap, ymap, 'V');
						   if (Map[xmap][ymap]=='#') MapUpdate (xmap, ymap, '#');
	 					}
					}
					Temp=0;
					break;
				case '8':
					Go();
					Temp=0;
					break;
				case '5':
					Stop();
					Temp=0;
					break;
				case '9':
					FixRight();
					Temp=0;
					break;
			}

	}
	
}		
		
		  

//------------------------------------------------------------
// UART0 - ISR
//-------------------------------------------------------------
void UART0_IRQHandler(){
	static int start = 0; 
	static int z;
	static int file = 0;
	static int Time = 0;
	if(UART0_S1 & UART_S1_RDRF_MASK){ // RX buffer is full and ready for reading
		
		Temp = UART0_D;

	}
	/*if (Temp == (char)0x01 || file == 1 || file == 2){
		switch (Temp){
			case 0x01:
				file =1;
				UartInt = 0;
				Temp = 0;
				break;
			case 0x02:
				file = 2;
				Temp = 0;
				break;
			case 0x03:
				Lib.numoffiles++;
				file = 0;
				Time = 0;
				Temp = 0;
				UARTprintf(UART0_BASE_PTR,tosend);
				break;
			default:
				if (file == 1){
					if (Temp != '\n') UartInt = UartInt*10 + (int)(Temp-'0');
				}
				if (file == 2){
					if (Time != 0 && Time != Lib.Filenum[Lib.numoffiles].size+1){ 
					*(Lib.Filenum[Lib.numoffiles].text + Time-1) = Temp;
					}
					else if (Time == Lib.Filenum[Lib.numoffiles].size+1)
					Time++;
					Temp = 0;
				}
				break;
		}
	}
	if (start == 3){
		if (Temp != '!'){
			extractfile = (int)(Temp - '0');
			Temp = ' ';
		}
		else if (Temp == '!') {
			start = 0;
			Temp = ' ';
		}
	}*/
	if (start==2){
		if (Temp != '@'){
			UartInt=UartInt*10 + (int)(Temp - '0');
			Temp = ' ';
		}
		if (Temp == '@'){
			start=0;
			Temp = ' ';
			UartInt *= -1;
		}
	}	
	if (start == 1){
		if (Temp =='^') {
			start = 0;
			Temp = ' ';
		while (z <4){
			recived[z] = '0';
			z++;
		}
			
		}
		else {
			recived[z] = Temp;
			z++;
		}
	}
	if (Temp =='^' && start == 0) {
			start = 1;
			z=0;
	}
	
	
	if (Temp =='^' && start == 0) {
		start = 1;
	}
	if (Temp =='@' && start == 0) {
		start = 2;
		UartInt=0;
	}
	/*if (Temp == '!' && start == 0){
		start = 3;
	}*/

	}




	/*if(UART0_S1 & UART_S1_TDRE_MASK){ // TX buffer is empty and ready for sending
		Temp = *(Point+count);
		UART0_D = Temp;
		
		count++;
	}
	if (count == 6)
	{
		disable_irq(INT_UART0-16);
		count =0;
	}*/
	






void PORTD_IRQHandler(){
	if (PORTD_ISFR & 0x80){
		DTMFInput = ((GPIOC_PDIR)>>11)&0x7;
		DTMFInput += ((GPIOA_PDIR)>>14)&0x8;
		PORTD_ISFR |= 0x80;
	}
}

int Decode(char* got)
{
	int i;
	int sum = 0;
	if ((got[0]-'0')>10){
	for (i=0;i<4;i++){
		sum += got[i];
	}
	}
	else 	{
		for (i=0;i<4;i++){
			sum =sum*10 + ((int)(got[i]-'0'));
		}
	}

	recived [3] = '$';
	return sum;
}
