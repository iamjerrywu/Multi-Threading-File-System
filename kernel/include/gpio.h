/** @file   gpio.h
 *
 *  @brief  Prototypes for GPIO configuration, set, clear, and read
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 05 September 2022
 *  @author CMU 14-642
**/
#ifndef _GPIO_H_
#define _GPIO_H_
#include <unistd.h>
#include <assert.h>


/* GPIO Port Mode */
#define MODE_INPUT      (0)
#define MODE_OUTPUT     (1)

/* GPIO Output Types -- S "standard", H "high", D "drain" */
#define OUTPUT_S0S1     (0)
#define OUTPUT_H0S1     (1)
#define OUTPUT_S0H1     (2)
#define OUTPUT_H0H1     (3)
#define OUTPUT_D0S1     (4)
#define OUTPUT_D0H1     (5)
#define OUTPUT_S0D1     (6)
#define OUTPUT_H0D1     (7)

/* GPIO Pull-Up or Pull-Down */
#define PUPD_NONE       (0)
#define PUPD_PULL_DOWN  (1)
#define PUPD_PULL_UP    (3)

/* GPIO Port Num */
#define GPIO_PORT_0     (0)
#define GPIO_PORT_1     (1)

/* GPIO BIT SHIFT */
#define GPIO_PIN_CNF_DIR_OFFSET    0x0
#define GPIO_PIN_CNF_INPUT_OFFSET  0x1
#define GPIO_PIN_CNF_PULL_OFFSET   0x2
#define GPIO_PIN_CNF_DRIVE_OFFSET  0x8

/* GPIO REG RELEATED */
#define GPIO_P0_BASE               0x50000000
#define GPIO_P1_BASE               0x50000300
#define GPIO_OUT_OFFSET            0x504
#define GPIO_OUTCLR_OFFSET         0x50C
#define GPIO_IN_OFFSET             0x510
#define GPIO_PIN_CNF_0_OFFSET      0x700
#define GPIO_DIRSET_OFFSET      0x518


/*
 * GPIO initialization function
 *
 * @param port   - 0 or 1
 * @param pin    - 0 to 31 for port 0, 0 to 15 for port 1
 * @param mode   - GPIO port mode (input or output)
 * @param otype  - GPIO output types (standard, high, open-drain)
 * @param pupd   - GPIO pull up, down, or neither
 */
void gpio_init(uint8_t port, uint8_t pin, uint8_t mode, uint8_t otype, uint8_t pupd);

/*
 * gpio_set: Set specified GPIO port/pin to high
 */
void gpio_set(uint8_t port, uint8_t pin);

/*
 * gpio_clr: Clear specified GPIO port/pin to low
 */
void gpio_clr(uint8_t port, uint8_t pin);


void gpio_dir_set_output(uint8_t port, uint8_t pin);
/*
 * gpio_read_all: Read all the gpio IN pins from selected port.
 * Each bit corresponds to its GPIO pin.
 */
uint32_t gpio_read_all(uint8_t port);

/*
 * gpio_read: return only a single logic value for the given port/pin
 */
uint8_t gpio_read(uint8_t port, uint8_t pin);

/*
 * get_gpio_base: return gpio base depends on port
 */
uint32_t get_gpio_base(uint8_t port);

void gpio_test();

#endif /* _GPIO_H_ */
