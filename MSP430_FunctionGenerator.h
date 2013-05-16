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
#include <intrinsics.h>
//
// Waveform enumeration
//
#define SQUARE            1
#define SIN               2
#define SAWTOOTH          3
#define EXP_PULSE         4
#define SQUARE_SAW_HIGH   3000
#define SQUARE_SAW_LOW    0
#define SIN_FREQ          890
#define SAW_FREQ          510
//
// Loop counter variables
//
static volatile unsigned int WAVE = SQUARE;
static volatile unsigned int DAC_VALUE = SQUARE_SAW_LOW;
static volatile unsigned int WAVE_DUTY_HIGH = 20000, WAVE_DUTY_LOW = 20000;
static volatile int square_count = 0, sin_count = 0, duty = 50, freq = 100;
static volatile int exp_up = 0, exp_down = 1, exp_hold = 0;
static volatile int exp_hold_time = 0, exp_toggle = 0, exp_index = 0;
//
// Waveform frequency/step variables
//
static volatile unsigned int TIMER_FREQ = 10000;
static int SQ_FREQ[] = {16000, 8000, 5400, 4000, 3200};
static volatile unsigned int SAW_STEP = 10, SIN_STEP = 1;
//
// Sin value look-up table, 180 values
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
// Exponential pulse LUT, 55 values
//
#pragma location = 0x1800
static uint16_t EXP_LUT[]  = {999,
            1009, 1019, 1029, 1039, 1050, 1060, 1071, 1082, 1093, 1104, 1115,
            1126, 1137, 1149, 1160, 1172, 1184, 1196, 1208, 1220, 1232, 1245,
            1257, 1270, 1283, 1295, 1308, 1322, 1335, 1348, 1362, 1376, 1389,
            1403, 1418, 1432, 1446, 1461, 1475, 1490, 1505, 1520, 1536, 1551,
            1567, 1583, 1598, 1615, 1631, 1647, 1664, 1681, 1697, 1715, 1732,
            1749, 1767, 1785, 1802, 1821, 1839, 1857, 1876, 1895, 1914, 1933,
            1953, 1972, 1992, 2012, 2032, 2053, 2074, 2094, 2116, 2137, 2158,
            2180, 2202, 2224, 2246, 2269, 2292, 2315, 2338, 2362, 2385, 2409,
            2434, 2458, 2483, 2508, 2533, 2558, 2584, 2610, 2636, 2663, 2690,
            2717, 2744, 2772, 2800, 2828, 2856, 2885, 2914, 2943};
//
// Function prototypes
//
void setDACOutput (unsigned int level);
/* END MSP430_FunctionGenerator header*/
#endif /* MSP430_FUNCTIONGENERATOR_H_ */
