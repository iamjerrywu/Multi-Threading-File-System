/** @file   arm.c
 *
 *  @brief  Assembly wrappers for arm instructions.
 *
 *  @date   Last modified 14 Oct 2022
 *
 *  @author CMU 14-642
**/

#include <arm.h>
#include <unistd.h>

/** @brief Interrupt Control and State Register */
#define ICSR ((volatile uint32_t *) 0xE000ED04)

/** @brief Configuration and Control Register */
#define CCR ((volatile uint32_t *) 0xE000ED14)

/** @brief System handler priority register 1 */
#define SHPR1 ((volatile uint32_t *) 0xE000ED18)

/** @brief System handler priority register 2 */
#define SHPR2 ((volatile uint32_t *) 0xE000ED1C)

/** @brief System handler priority register 3 */
#define SHPR3 ((volatile uint32_t *) 0xE000ED20)

/** @brief System handler control and state register */
#define SHCSR ((volatile uint32_t *) 0xE000ED24)

/** @brief I-code cache configuration register, to configure instr caching on nrf52840 */
#define ICACHECNF ((volatile uint32_t *) 0x540)

/** @brief Register to enable/disable fpu */
#define CPACR ((volatile uint32_t *) 0xE000ED88)

/** @brief FPU control data */
#define FPCCR ((volatile uint32_t *) 0xE000EF34)

/** @brief disable stack alignment and set SVC handler priority */
void init_align_prio() {
    // stack alignment
    *CCR &= ~(0x1 << 9);

    // MemoryMgmt priority ==> 1
    *SHPR1 |= 0x00000020;
    *SHPR1 &= 0xFFFFFF20;

    // SVC handler priority ==> 2
    *SHPR2 |= 0x40000000;
    *SHPR2 &= 0x40FFFFFF;

    // Systick & PendSV priority ==> 1
    *SHPR3 |= 0x20200000;
    *SHPR3 &= 0x2020FFFF;

    __asm volatile("dsb");
    __asm volatile("isb");
}

/** @brief enable instruction caching */
void enable_prefetch_i_cache(){
    // enable bit
    *ICACHECNF |= (0x1 << 0);
}

/** @brief enable the FPU */
void enable_fpu(){
    *CPACR |= (0xF << 20);
    *FPCCR |= (0x1 << 31);
    __asm volatile("dsb");
    __asm volatile("isb");
}

/** @brief pend a pendsv */
void pend_pendsv( void ){
    *ICSR |= (1 << 28);
}

/** @brief clears pendsv */
void clear_pendsv( void ){
    *ICSR |= (1 << 27);
}

/** @brief indicator for SVC active/inactive */
int get_svc_status() {
    return (*SHCSR & (1 << 7));
}

/** @brief set SVC active/inactive */
void set_svc_status(int status) {
    if(status) *SHCSR |= (1 << 7);
    else *SHCSR &= ~(1 << 7);
}

void delay(void){
  uint64_t cnt = 100000;
  
  while (cnt) cnt--;
}

void CLEAR(uint32_t ms) {
  uint64_t cnt = MILI_SECOND_PAR * ms;
  while (cnt) cnt--;
}

uint32_t arm_abs(int32_t val) {
  return (val > 0) ? val : -val;
}