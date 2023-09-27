/** @file   gpio.c
 *
 *  @brief  GPIO interface implementation
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 20 September 2022
 *  @author CMU 14-642
**/

#include <gpio.h>
#include <arm.h>
#include <assert.h>

/**
 * Initialize gpio configuration.
 */
void gpio_init(uint8_t port, uint8_t pin, uint8_t mode, uint8_t otype, uint8_t pupd) {
    uint32_t gpio_base = get_gpio_base(port);
    
    REG_WRITE((uint32_t *)(gpio_base + GPIO_PIN_CNF_0_OFFSET + (pin << 2)), 
             ((otype << GPIO_PIN_CNF_DRIVE_OFFSET) 
              | (pupd << GPIO_PIN_CNF_PULL_OFFSET) 
              | mode << GPIO_PIN_CNF_DIR_OFFSET));
} 

/**
 * Set gpio output.
 */
void gpio_set(uint8_t port, uint8_t pin) {
    uint32_t gpio_base = get_gpio_base(port);
    
    REG_WRITE((uint32_t *)(gpio_base + GPIO_OUT_OFFSET), 
             0x1 << pin);
}

/**
 * Clear gpio output.
 */
void gpio_clr(uint8_t port, uint8_t pin) {
    uint32_t gpio_base = get_gpio_base(port);
    
    REG_WRITE((uint32_t *)(gpio_base + GPIO_OUTCLR_OFFSET), 
             0x1 << pin);
}

/**
 * Initialize gpio configuration.
 */
void gpio_dir_set_output(uint8_t port, uint8_t pin) {
    uint32_t gpio_base = get_gpio_base(port);
    
    REG_WRITE((uint32_t *)(gpio_base + GPIO_DIRSET_OFFSET), 
             0x1 << pin);
}

/**
 * Read all gpio pins.
 */
uint32_t gpio_read_all(uint8_t port) {
    uint32_t reg_val;
    uint32_t gpio_base = get_gpio_base(port);
    
    reg_val = REG_READ((uint32_t *)(gpio_base + GPIO_IN_OFFSET));
    return reg_val;
}

/**
 * Read specific gpio pin.
 */
uint8_t gpio_read(uint8_t port, uint8_t pin) {
    uint8_t reg_val;
    uint32_t gpio_base = get_gpio_base(port);

    reg_val = REG_SHIFT_MASK(REG_READ((uint32_t *)(gpio_base + GPIO_IN_OFFSET)), pin, 0x1);
    return reg_val;
}

/**
 * Get gpio base.
 */
uint32_t get_gpio_base(uint8_t port) {
    uint32_t reg_base;
    if (port == GPIO_PORT_0) {
            reg_base = GPIO_P0_BASE;
    } else if (port == GPIO_PORT_1) {
            reg_base = GPIO_P1_BASE;
    }       
    return reg_base;                      
}

/**
 * Test gpio function.
 */
void gpio_test(){
    uint32_t reg_val;

    // configure gpio pin15
    gpio_init(1, 15, 1, 0, 0);
    breakpoint();
    // set relevant GPIO output value to ON
    gpio_set(1, 15);
    breakpoint();
    // read gpio all
    reg_val = gpio_read_all(1);
    if (reg_val == 0x00008000) {
        breakpoint();
    }
    reg_val = gpio_read(1, 15);
    if (reg_val == 0x1) {
        breakpoint();
    }
    // should turn off the LED
    gpio_clr(1, 15);
}