#include "TFC.h"
#include "mcg.h"




// set I/O for switches and LEDs
void InitGPIO()
{
	//enable Clocks to all ports - page 206, enable clock to Ports
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;

	//GPIO Configuration - LEDs - Output
	PORTD_PCR1 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;  //Blue
	GPIOD_PDDR |= BLUE_LED_LOC; //Setup as output pin	
	PORTB_PCR18 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //Red  
	PORTB_PCR19 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //Green
	GPIOB_PDDR |= RED_LED_LOC + GREEN_LED_LOC; //Setup as output pins
	RGB_LED_OFF;
	//GPIO Configuration - Pushbutton - Input
	// RF configurations

}


//-----------------------------------------------------------------
// Servo Motors Config
//-----------------------------------------------------------------
void ServosConfig (){
	//Servo Motor 1
	PORTE_PCR21 = PORT_PCR_MUX(3); //assign PTE21 as TPM1
	PORTE_PCR29 = PORT_PCR_MUX(0); //assign PTE29 as ADC0
	
	// Servo motor 2
	PORTE_PCR31 = PORT_PCR_MUX(3); //assign PTE31 as TPM0
	PORTE_PCR30 = PORT_PCR_MUX(0); //assign PTE30 as ADC0


}

//-----------------------------------------------------------------
// Distance Measuring Sensors
//-----------------------------------------------------------------
void DistSensConfig (){
	// Distance samp 1
	PORTD_PCR6 = PORT_PCR_MUX(0); //assign PTD6 as ADC0

	// Distance samp 2
	PORTD_PCR5 = PORT_PCR_MUX(0); //assign PTD5 as ADC0
	
}

//-----------------------------------------------------------------
// Reflectance Sensor config
//-----------------------------------------------------------------
void ColorSensConfig (){
	PORTE_PCR2 = PORT_PCR_MUX(1); // assign PTE2 as GPIO
	GPIOE_PDDR &= ~PORT_LOC(2);  // PTE(2) is Input
	//5PORTE_PCR2 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK | PORT_PCR_IRQC(0x0a);
	PORTE_PCR3 = PORT_PCR_MUX(1); // assign PTE3 as GPIO
	GPIOE_PDDR &= ~PORT_LOC(3);  // PTE(3) is Input
	//PORTE_PCR3 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK | PORT_PCR_IRQC(0x0a);
	PORTE_PCR4 = PORT_PCR_MUX(1); // assign PTE4 as GPIO
	GPIOE_PDDR &= ~PORT_LOC(4);  // PTE(4) is Input
	//PORTE_PCR4 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK | PORT_PCR_IRQC(0x0a);
	PORTE_PCR5 = PORT_PCR_MUX(1); // assign PTE5 as GPIO
	GPIOE_PDDR &= ~PORT_LOC(5);  // PTE(5) is Input
//	PORTE_PCR5 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK | PORT_PCR_IRQC(0x0a);
	PORTA_PCR16 = PORT_PCR_MUX(1); // assign PTA16 as GPIO
	GPIOA_PDDR &= ~PORT_LOC(16);  // PTA(16) is Input
	//PORTA_PCR16 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK | PORT_PCR_IRQC(0x0a);
	
}

//-----------------------------------------------------------------
// Battery config
//-----------------------------------------------------------------
void BattLedConfig (){ 
	PORTE_PCR22 = PORT_PCR_MUX(0); // assign PTE22 as ADC0
	PORTB_PCR8 = PORT_PCR_MUX(1);  // assign PTE5 as GPIO
	GPIOE_PDDR |= PORT_LOC(8);  // PTB(8) is Output
	PORTB_PCR9 = PORT_PCR_MUX(1); // assign PTE5 as GPIO
	GPIOE_PDDR |= PORT_LOC(9);  // PTB(9) is Output
	PORTB_PCR10 = PORT_PCR_MUX(1); // assign PTE5 as GPIO
	GPIOE_PDDR |= PORT_LOC(10);  // PTB(10) is Output

}

//-----------------------------------------------------------------
// DTMF config
//-----------------------------------------------------------------

void DTMFConfig(){
	PORTC_PCR11 = PORT_PCR_MUX(1); // assign PTC11 as GPIO
	GPIOC_PDDR &= ~PORT_LOC(11);  // PTC(11) is Input
	PORTC_PCR12 = PORT_PCR_MUX(1); // assign PTC12 as GPIO
	GPIOC_PDDR &= ~PORT_LOC(12);  // PTC(12) is Input
	PORTC_PCR13 = PORT_PCR_MUX(1); // assign PTC13 as GPIO
	GPIOC_PDDR &= ~PORT_LOC(13);  // PTC(13) is Input
	PORTA_PCR17 = PORT_PCR_MUX(1); // assign PTC17 as GPIO
	GPIOA_PDDR &= ~PORT_LOC(17);  // PTC(17) is Input
	PORTD_PCR7 = PORT_PCR_MUX(1); // assign PTD7 as GPIO
	GPIOD_PDDR &= ~PORT_LOC(7);  // PTD(7) is Input
	PORTD_PCR7 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK; 
	enable_irq(INT_PORTD-16); // Enable Interrupts 
	set_irq_priority (INT_PORTD-16,0);  // Interrupt priority = 0 = max	
	PORTD_PCR7 |= PORT_PCR_IRQC(0x0a);
}
//-----------------------------------------------------------------
// DipSwitch data reading
//-----------------------------------------------------------------
uint8_t TFC_GetDIP_Switch()
{
	uint8_t DIP_Val=0;
	
	DIP_Val = (GPIOC_PDIR>>4) & 0xF;

	return DIP_Val;
}


void PrintDouble (double x){
	char arr1[16];
	char arr2[16];
	int V1 = (int) x;
	int V2 = (int)((x-(int)(x))*100);
	sprintf(arr1, "%d", V1);
	sprintf(arr2, "%d", V2);
	UARTprintf(UART0_BASE_PTR, arr1);
	UARTprintf(UART0_BASE_PTR, ".");
	if(V2<10) UARTprintf(UART0_BASE_PTR, "0");
	UARTprintf(UART0_BASE_PTR, arr2);
	UARTprintf(UART0_BASE_PTR, "\r\n");
}

void MapUpdate (int x, int y, char Char){
	char XC = (char)x+' ';
	char YC = (char)y+' ';
	char eof = 0x1A;
	char tosend [4] = {Char,XC,YC,eof};
	UARTprintf(UART0_BASE_PTR, tosend);
}
