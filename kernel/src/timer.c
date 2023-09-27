/** @file   timer.c
 *
 *  @brief  timer implementations for systick timer
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 25 October 2022
 *  @author CMU 14-642
**/

#include <timer.h>
#include <unistd.h>
#include <arm.h>
#include <printk.h>
#include <stdbool.h>
#include <unistd.h>
static volatile bool wait_flag = true;

void systick_start(int frequency) {
    (void)frequency;
    REG_WRITE((uint32_t *)(SYST_CSR), 0);
    REG_WRITE((uint32_t *)(SYST_CSR), 7);
    REG_WRITE((uint32_t *)(SYST_RVR), 64000000);
    REG_WRITE((uint32_t *)(SYST_CSR), 0);
    return;
}

void systick_stop() {
    REG_WRITE((uint32_t *)(TIMER_0 + TASKS_STOP_OFFSET), 1);
    REG_WRITE((uint32_t *)(TIMER_0 + INTENCLR_OFFSET), (1 << 16));
}

/**
 * Use system clock to delay.
 */
void systick_delay(uint32_t ms) {
    uint32_t ticks = 0;
    REG_WRITE((uint32_t *)(SYST_CSR), 0);
    REG_WRITE((uint32_t *)(SYST_CSR), 5);
    REG_WRITE((uint32_t *)(SYST_RVR), 64000);
    while (ticks < ms) {
        while (!(REG_READ((uint32_t *)(SYST_CSR))&(0x1 << 16))) {
            ticks++;
        }
        CLEAR_CSR(ms);
    }
}

/**
 * Start adc timer.
 */
void adc_timer_start(int freq) {
    (void) freq;
    REG_WRITE((uint32_t *)(TIMER_0 + TASKS_CLEAR_OFFSET), 1);
    REG_WRITE((uint32_t *)(TIMER_0 + PRESCALER_OFFSET), 8);
    REG_WRITE((uint32_t *)(TIMER_0 + CC_0_OFFSET), 1);
    REG_WRITE((uint32_t *)(TIMER_0 + INTENSET_OFFSET), (1 << 16));
    REG_WRITE((uint32_t *)(TIMER_0 + TASKS_START_OFFSET), 1);

    REG_WRITE((uint32_t *)NVIC_ISER0, REG_READ((uint32_t *)NVIC_ISER0 | 0x100));
    return;
}

/**
 * Polling adc interrupt status.
 */
void poll_adc_interrupt_status(void) {
    while ((REG_READ((uint32_t *)(TIMER_0 + EVENT_COMPARE)) == 0));

}

/**
 * Clear adc interrupt flag.
 */
void adc_interrupt_clear_flag(void) {
    REG_WRITE((uint32_t *)(TIMER_0 + EVENT_COMPARE), 0);
}

/**
 * Enable adc interrupt.
 */
void enable_adc_interrupt(void) {
    REG_WRITE((uint32_t *)(TIMER_0 + INTENSET_OFFSET), (1 << 16));
    REG_WRITE((uint32_t *)(TIMER_0 + TASKS_START_OFFSET), 1);
}

/**
 * Clear csr.
 */
void CLEAR_CSR(uint32_t ms) {
    REG_WRITE((uint32_t *)(SYST_CSR), 0);
    CLEAR(ms);
}

/**
 * System tick interrupt handler.
 */
// void SysTick_Handler(void)  {                              
//     wait_flag = false;
//     printk("Hello World\n");
// }
