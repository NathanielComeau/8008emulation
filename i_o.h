#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#ifndef __i_o_h__
#define __i_o_h__

uint8_t i_oGetInput(uint8_t fromDeviceNum);
uint8_t i_oGiveOutput(uint8_t toDeviceNum);

#endif