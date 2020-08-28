/***************************************************************************//**
 * @file main_series1.c
 * @brief This project demonstrates controlling servo motors using pulse width
 * modulation generated by the TIMER module. The GPIO pin specified in the
 * readme.txt is configured for output and outputs a 1kHz, 30% duty cycle
 * signal. The duty cycle can be adjusted by writing to the CCVB or changing the
 * global dutyCyclePercent variable.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable 
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/

#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "bsp.h"

// Note: change this to set the desired output frequency in Hz
#define PWM_FREQ 1000

// Note: change this to set the desired duty cycle (used to update CCVB value)
static volatile int dutyCyclePercent = 0;

// stores 1 msTicks from the SysTick timer
volatile uint32_t msTicks;

/**************************************************************************//**
 * @brief
 *    Interrupt handler for TIMER0 that changes the duty cycle
 *
 * @note
 *    This handler doesn't actually dynamically change the duty cycle. Instead,
 *    it acts as a template for doing so. Simply change the dutyCyclePercent
 *    global variable here to dynamically change the duty cycle.
 *****************************************************************************/
void TIMER0_IRQHandler(void)
{
  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(TIMER0);
  TIMER_IntClear(TIMER0, flags);

  // Update CCVB to alter duty cycle starting next period
  TIMER_CompareBufSet(TIMER0, 0, (TIMER_TopGet(TIMER0) * dutyCyclePercent) / 100);
}

/**************************************************************************//**
 * @brief GPIO initialization
 *****************************************************************************/
void initGpio(void)
{
  // Enable GPIO and clock
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Configure PC10 (Expansion Header Pin 16) as output
  GPIO_PinModeSet(gpioPortC, 10, gpioModePushPull, 0);
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
  // Use PWM mode, which sets output on overflow and clears on compare events
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModePWM;
  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Route TIMER0 CC0 to location 15 and enable CC0 route pin
  // TIM0_CC0 #15 is GPIO Pin PC10
  TIMER0->ROUTELOC0 |=  TIMER_ROUTELOC0_CC0LOC_LOC15;
  TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN;

  // Set top value to overflow at the desired PWM_FREQ frequency
  TIMER_TopSet(TIMER0, CMU_ClockFreqGet(cmuClock_TIMER0) / PWM_FREQ);

  // Set compare value for initial duty cycle
  TIMER_CompareSet(TIMER0, 0, (TIMER_TopGet(TIMER0) * dutyCyclePercent) / 100);

  // Initialize the timer
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  TIMER_Init(TIMER0, &timerInit);

  // Enable TIMER0 compare event interrupts to update the duty cycle
  TIMER_IntEnable(TIMER0, TIMER_IEN_CC0);
  NVIC_EnableIRQ(TIMER0_IRQn);
}

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       // increment counter necessary in Delay()
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}

/**************************************************************************//**
 * @brief
 *    Main function
 *****************************************************************************/
int main(void)
{
  // Chip errata
  CHIP_Init();
  
  // Init DCDC regulator with kit specific parameters
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
  EMU_DCDCInit(&dcdcInit);

  // Initializations
  initGpio();
  initTimer();

  // Setup SysTick Timer for 1 msec interrupts
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

  while (1)
  {
    for(int i = 0; i < 20; i++)
    {
    	dutyCyclePercent = i;
    	Delay(60);
    }

    for(int i = 20; i >= 0; i--)
    {
    	dutyCyclePercent = i;
    	Delay(60);
    }
  }
}

