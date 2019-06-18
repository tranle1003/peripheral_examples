/**************************************************************************//**
 * @main_g.c
 * @brief This project demonstrates frequency generation using the
 * TIMER module. The pin specified in readme.txt is configured for output
 * compare and toggles on each overflow event at a set frequency.
 * @version 0.0.1
 ******************************************************************************
 * @section License
 * <b>Copyright 2018 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_timer.h"

// Desired frequency in Hz
// Min: 107 Hz, Max: 7.5 MHz with default settings
#define OUT_FREQ 1000

// Default prescale value
#define TIMER0_PRESCALE timerPrescale1

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void initGpio(void)
{
  // Enable GPIO clock
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Configure PD1 as output
  GPIO_PinModeSet(gpioPortD, 1, gpioModePushPull, 0);
}

/**************************************************************************//**
 * @brief
 *    TIMER initialization
 *****************************************************************************/
void initTimer(void)
{
  // Enable clock for TIMER0 module
  CMU_ClockEnable(cmuClock_TIMER0, true);

  // Configure TIMER0 Compare/Capture for output compare
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModeCompare;
  timerCCInit.cmoa = timerOutputActionToggle;
  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Route TIMER0 CC0 to location 3 and enable CC0 route pin
  // TIM0_CC0 #3 is GPIO Pin PD1
  TIMER0->ROUTE |= (TIMER_ROUTE_CC0PEN | TIMER_ROUTE_LOCATION_LOC3);

  // Set Top value
  // Note each overflow event constitutes 1/2 the signal period
  uint32_t topValue = CMU_ClockFreqGet(cmuClock_HFPER) / (2*OUT_FREQ * (1 << TIMER0_PRESCALE))-1;
  TIMER_TopSet(TIMER0, topValue);

  // Initialize and start timer with defined prescale
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  timerInit.prescale = TIMER0_PRESCALE;
  TIMER_Init(TIMER0, &timerInit);
}

/**************************************************************************//**
 * @brief
 *    Main function
 *****************************************************************************/
int main(void)
{
  // Chip errata
  CHIP_Init();

  // Initialization
  initGpio();
  initTimer();

  while (1) {
    EMU_EnterEM1(); // Enter EM1 (won't exit)
  }
}
