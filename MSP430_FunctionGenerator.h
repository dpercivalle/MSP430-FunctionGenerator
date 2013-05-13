/**
  * CPE 329 - 07/08
  * Spring 2013
  *
  * Project 2 - Function Generator
  * Engineers:  Donny Percivalle
  *             Alex Lin
  **/

#ifndef MSP430_FUNCTIONGENERATOR_H_
#define MSP430_FUNCTIONGENERATOR_H_
//
// Include used libraries
//
#include <msp430g2553.h>
#include <stdint.h>
#include "MSP430_LCD_nibble.c"
//
// Waveform enumeration
//
#define  SQUARE   1
#define  SAWTOOTH 2
#define  SIN      0
#define SQUARE_SAW_HIGH   3000
#define SQUARE_SAW_LOW    0
//
// Loop counter variables
//
static volatile int WAVE = SQUARE;
static volatile unsigned int DAC_VALUE = SQUARE_SAW_LOW;
static volatile unsigned int WAVE_DUTY = 1, WAVE_DUTY_SET = 1, SQUARE_NESTED_DELAY = 3;
static volatile int square_count = 0, sin_count = 0;
//
// Waveform frequency/step variables
//
static volatile unsigned int TIMER_FREQ = 20000;
static int SQ_FREQ[] = {20000, 10000, 6666, 5000, 4000};
static int SIN_FREQ[] = {675};
static volatile unsigned int SAW_STEP = 10;
//
// Sin value look-up table
//
static uint16_t SIN_LUT[] =
            {1500,
            1552, 1604, 1656, 1708, 1760, 1811, 1862, 1913, 1963, 2013, 2061,
            2110, 2157, 2204, 2250, 2294, 2338, 2381, 2423, 2464, 2503, 2541,
            2579, 2614, 2649, 2682, 2713, 2743, 2772, 2799, 2824, 2848, 2870,
            2890, 2909, 2926, 2941, 2955, 2967, 2977, 2985, 2991, 2996, 2999,
            3000, 2999, 2996, 2991, 2985, 2977, 2967, 2955, 2941, 2926, 2909,
            2890, 2870, 2848, 2824, 2799, 2772, 2743, 2713, 2682, 2649, 2614,
            2579, 2541, 2503, 2464, 2423, 2381, 2338, 2294, 2250, 2204, 2157,
            2110, 2061, 2013, 1963, 1913, 1862, 1811, 1760, 1708, 1656, 1604,
            1552, 1500, 1447, 1395, 1343, 1291, 1239, 1188, 1137, 1086, 1036,
            986, 938, 889, 842, 795, 749, 705, 661, 618, 576, 535,
            496, 458, 420, 385, 350, 317, 286, 256, 227, 200, 175,
            151, 129, 109, 90, 73, 58, 44, 32, 22, 14, 8,
            3, 0, 0, 0, 3, 8, 14, 22, 32, 44, 58,
            73, 90, 109, 129, 151, 175, 200, 227, 256, 286, 317,
            350, 385, 420, 458, 496, 535, 576, 618, 661, 705, 749,
            795, 842, 889, 938, 986, 1036, 1086, 1137, 1188, 1239, 1291,
            1343, 1395, 1447, 1499};
//
// Function prototypes
//
void setDACOutput (unsigned int level);
/* END MSP430_FunctionGenerator header*/
#endif /* MSP430_FUNCTIONGENERATOR_H_ */
