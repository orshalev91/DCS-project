#include "TFC.h"
#include "adc.h"

extern double ADCMeas[3];
int circle = 0;
extern double samp[2][23];
/*	adc_init()
 * Calibrates and initializes adc to perform single conversions and generate
 * DMA requests at the end of the conversion
 * 
 * */
void adc_init(void)
{
	// Enable clocks
	SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;	// ADC0 clock
	
	// Calibrate ADC
//	adc_cal(); 

	// Configure ADC
	ADC0_CFG1 = 0; // Reset register
	ADC0_CFG1 |= (ADC_CFG1_MODE(3)  |  	// 16 bits single ended convertion  
				  ADC_CFG1_ADICLK(0)|	// Input Bus Clock (20-25 MHz out of reset (FEI mode))
				  ADC_CFG1_ADIV(1)) |   // Bus Clock
				  ADC_CFG1_ADLSMP_MASK;	// Long sample time                             Clock not divided by 2 (10-12.5 MHz)
	
	ADC0_CFG2 &= ~ADC_CFG2_ADHSC_MASK; // normal speed configuration
	ADC0_CFG2 |= ADC_CFG2_ADLSTS(0) | ADC_CFG2_MUXSEL_MASK; //24 Cycles sample time
	ADC0_CFG2 |= ADC_CFG2_ADHSC_MASK;
	
	ADC0_SC2 &= ~ADC_SC2_ADTRG_MASK ;    // DMA Enable, hardware trigger enabled
	
	ADC0_SC3 = 0; // Reset SC3
	ADC0_SC3 |= ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3); // Hardware average enable, 32 samples average 
	
	ADC0_SC1A = ADC_SC1_ADCH(31); // disable adc before start
	enable_irq(INT_ADC0-16);
	set_irq_priority(INT_ADC0-16,0);
}

//-----------------------------------------------------------------
// ADC0 - ISR = Interrupt Service Routine
//-----------------------------------------------------------------
void ADC0_IRQHandler(){
	static int ADCLoop = 0;
	static int i =0;
	if (circle == 0){
		i=0;
		switch(ADCLoop){
			case 0:  //Left IR
				ADCMeas[0]=ADC0_RA*3.3/0xFFFF; 
				ADC0_SC1A = ADC_SC1_ADCH(6) | ADC_SC1_AIEN_MASK;
				ADCLoop++;
				break;
			case 1: // Right IR
				ADCMeas[1]=ADC0_RA*3.3/0xFFFF;
				ADC0_SC1A = ADC_SC1_ADCH(3) | ADC_SC1_AIEN_MASK;
				ADCLoop++;
				break;
			case 2: // Battery
				ADCMeas[2]=ADC0_RA*3.3/0xFFFF;
				ADC0_SC1A = ADC_SC1_ADCH(7) | ADC_SC1_AIEN_MASK;
				ADCLoop=0;
				break;
		}
	}	
	if (circle == 1) {
		switch (ADCLoop){
			case 0:  //Left IR
				ADCMeas[0]=ADC0_RA*3.3/0xFFFF; 
				ADC0_SC1A = ADC_SC1_ADCH(6) | ADC_SC1_AIEN_MASK;
				ADCLoop=1;
				TPM1_C1V += 0x40;  // left servo motor
				samp[0][i]=ADCMeas[0];
				waitt(2);
				break;
			case 1: // Right IR
				ADCMeas[1]=ADC0_RA*3.3/0xFFFF;
				ADC0_SC1A = ADC_SC1_ADCH(7) | ADC_SC1_AIEN_MASK;
				ADCLoop=0;
				TPM0_C4V -= 0x40;  //right servo motor
				samp[1][i]=ADCMeas[1];
				i++;
				waitt(2);
				break;
			case 2: 
				ADCLoop = 0;
				break;
		}
	}
	if(TPM0_C4V <= 0x740) circle = 0;
	}












void InitDAC(){
//When the DAC is enabled and the buffer is not enabled, 
//the DAC module always converts the data in DAT0 to analog output voltage.
// pin PTE30 is by default (ALT0) configured as DAC0_OUT
	//VDDA reference voltage (Use this option for the best ADC operation).
	SIM_SCGC6 |= SIM_SCGC6_DAC0_MASK; //DAC0 Clock Gate Control
	DAC0_C0 |= DAC_C0_DACEN_MASK + DAC_C0_DACRFS_MASK  + DAC_C0_LPEN_MASK; //dac enable, low mode
	DAC0_C0 &= ~DAC_C0_DACTRGSEL_MASK; //HW TRIGGER
	DAC0_C1 |= DAC_C1_DMAEN_MASK; //DMA ENABLE

    /* Normal buffer mode */
   // DAC0_C1 &= ~DAC_C1_DACBFMD_MASK;///??????
}

/*unsigned short	adc_read(unsigned char ch)
 * 	Reads the specified adc channel and returns the 12 bits read value
 * 	
 * 	ch -> Number of the channel in which the reading will be performed
 * 	Returns the -> Result of the conversion performed by the adc
 * 
 * */
unsigned short adc_read(unsigned char ch) //not used?
{
	ADC0_SC1A = (ch & ADC_SC1_ADCH_MASK) | 
				(ADC0_SC1A & (ADC_SC1_AIEN_MASK | ADC_SC1_DIFF_MASK)); // Write to SC1A to start conversion
	while(ADC0_SC2 & ADC_SC2_ADACT_MASK); 	 // Conversion in progress
	while(!(ADC0_SC1A & ADC_SC1_COCO_MASK)); // Run until the conversion is complete
	return ADC0_RA;
}

