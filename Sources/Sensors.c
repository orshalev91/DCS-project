/*
 * Sensors.c
 *
 *  Created on: Jun 9, 2016
 *      Author: omerfeld
 */

#include "derivative.h" 				// Include peripheral declarations 
# include "TFC.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>

#define PI 3.14159265
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
extern int ArenaX;
extern int ArenaY;
extern double ADCMeas[3];
extern double IRDist[14];
int SetRotate =0;
double OpenR[2] = {0,0};; // 0 = dist, 1= angle
double OpenL[2] = {0,0}; // 0 = dist, 1= angle
int i=0;
int placeL =-1;
int placeR =-1;
int OpeningL;
int OpeningR;
int OpenEndL=0;
int OpenEndR=0;
double samp[2][23]; //samp[0][] left ,samp[1][] right
double Sin;
double Cos;
int XDef;
int YDef;
int mapValL [2][23];
int mapValR [2][23];
extern int circle;
extern char Map [42][58];
extern int status;
extern int xmap;
extern int ymap;

/******************************************************************************
 * Functions
 ******************************************************************************/
void ScanForOpen()
{
	OpeningR=0;
	circle = 1;
	while(SetRotate==0 && i<23){
		if(TPM0_C4V <= 0x740){
			SetRotate=1;
			circle = 0;
		}
			
	}
	TPM1_C1V = 0x450;
	TPM0_C4V = 0xBD0;
	SetRotate =0;
	for (i=0;i<23;i++){
		Sin = sin(PI/2-i*PI/46);
		Cos = cos(PI/2-i*PI/46);
		
		int k;
		for (k=0; k<14; k++ ){ // convert the values of the seonsors to distance
			if(samp[0][i] <= IRDist[k] && samp[0][i] > IRDist[k+1]){
				placeL=k;
			}
			if(samp[1][i] <= IRDist[k] && samp[1][i] > IRDist[k+1]){
				placeR=k;
			}
		}	
			samp[0][i] = 20+placeL*10 + (1-((samp[0][i]-IRDist[placeL+1])/(IRDist[placeL]-IRDist[placeL+1])))*10;   //Left Opening
			if ((samp[0][i] > (OpenL[0]+60)) && i!=0){  // Find the begining of the exit
				OpenEndL = 1;
				if(OpeningL ==0)	OpeningL = i;
			}
			else if(OpenEndL==1){   // Find the end of the exit
				OpeningL+=i-2;
				OpeningL = OpeningL/2;
				OpenL[0] = samp[0][OpeningL];
				OpenL[1] = OpeningL*4.1;
				OpenEndL=0;
			}
			else OpenL[0]=samp[0][i];
			
			if (OpenEndL==1 && i==22){  // in case we are looking at 90 degrees
				OpeningL+=i-2;
				OpeningL = OpeningL/2;
				if(OpeningL<2) OpeningL=0;
				OpenL[0] = samp[1][OpeningL];
				OpenL[1] = OpeningL*4.1;
				OpenEndR=0;
			}
			samp[1][i] = 20+placeR*10 + (1-((samp[1][i]-IRDist[placeR+1])/(IRDist[placeR]-IRDist[placeR+1])))*10;  //Right Opening
			if ((samp[1][i] > 120) && i!=0){  
				OpenEndR = 1;     //Start of the opening
				if(OpeningR ==0){
					OpeningR = i;  //degree of the start of the opening
				}
			}
			else if(OpenEndR==1){
				OpeningR+=i-2;
				OpeningR = OpeningR/2;
				if(OpeningR<2) OpeningR=0;
				OpenR[0] = samp[1][OpeningR];
				OpenR[1] = OpeningR*4.1;
				OpenEndR=0;
			}
			else OpenR[0]=samp[1][i];
			
			if (OpenEndR==1 && i==22){  // in case we are looking at 90 degrees
				OpeningR+=i-2;
				OpeningR = OpeningR/2;
				if(OpeningR<2) OpeningR=0;
				OpenR[0] = samp[1][OpeningR];
				OpenR[1] = OpeningR*4.1;
				OpenEndR=0;
			}
			
			placeL=-1;
			placeR=-1;
			if (samp[0][i]<=100){  //Left
				XDef = (int)(samp[0][i]*sin(2*PI*i*3.9/360)/10);
				YDef = (int)(samp[0][i]*cos(2*PI*i*3.9/360)/10); 
				mapValL[0][i]= ArenaY+YDef;
				mapValL[1][i]= ArenaX-XDef;
			}
			if (samp[1][i]<=100){ // right
				XDef = (int)((samp[1][i]*sin(2*PI*i*3.9/360))/10);
				YDef = (int)((samp[1][i]*cos(2*PI*i*3.9/360))/10);
				mapValR[0][i]= ArenaY+YDef;
				mapValR[1][i]= ArenaX+XDef;
			}

	}
	int j;
	int qa;
	for (qa=0; qa<=57; qa++){
		for (j=0; j<=41;j++){
			Map[j][qa]='_';
		}
	}	
	Map[22][1] = 'V';
	for (i = 0; i< 23; i++){
		if (samp[0][i]<=100) Map[mapValL[1][i]][mapValL[0][i]] = '#';
		if (samp[1][i]<=100) Map[mapValR[1][i]][mapValR[0][i]] = '#';
	}

}
	
