/*
 * Clocks.h
 *
 *  Created on: Jun 8, 2016
 *      Author: os
 */

#include <stdint.h>

#ifndef CLOCKS_H_
#define CLOCKS_H_

void ClockSetup();
void InitTPM();
void InitPIT();
void waitt (int i);
void delay (int i);

#endif /* CLOCKS_H_ */
