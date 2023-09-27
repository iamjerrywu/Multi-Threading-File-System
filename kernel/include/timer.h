/** @file   timer.h
 *
 *  @brief  function prototypes for systick timer
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 14 October 2022
 *  @author CMU 14-642
**/

#ifndef _TIMER_H_
#define _TIMER_H_

#include <unistd.h>

#define SYST_CSR   0xE000E010
#define SYST_RVR   0xE000E014
#define SYST_CVR   0xE000E018
#define SYST_CALIB 0xE000E01C

#define TIMER_0            0x40008000
#define TASKS_START_OFFSET 0x000
#define TASKS_STOP_OFFSET  0x004
#define MODE_OFFSET        0x504
#define PRESCALER_OFFSET   0x510
#define INTENSET_OFFSET    0x304
#define INTENCLR_OFFSET    0x308
#define BITMODE_OFFSET     0x508
#define TASKS_CLEAR_OFFSET 0x00C
#define CC_0_OFFSET        0x540
#define EVENT_COMPARE      0x140

#define NVIC_ISER0        0xE000E100

void systick_start(int frequency);

void systick_stop();

void systick_delay(uint32_t ms);

void adc_timer_start(int freq);

void poll_adc_interrupt_status(void);

void CLEAR_CSR(uint32_t ms);

void adc_interrupt_clear_flag(void);

void enable_adc_interrupt(void);

#endif /* _TIMER_H_ */
