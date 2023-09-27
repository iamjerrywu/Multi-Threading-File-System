/** @file   mpu.h
 *
 *  @brief  prototypes for memory protection
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 25 October 2022
 *  @author CMU 14-642
**/

#ifndef _MPU_H_
#define _MPU_H_

/** @enum   mpu_mode
 *  @brief  memory protection mode is PER_THREAD or KERNEL_ONLY
 */
typedef enum { PER_THREAD = 1, KERNEL_ONLY = 0 } mpu_mode;

uint32_t mm_log2ceil_size(uint32_t n);
void mm_region_disable(uint32_t region_number);
int mm_region_enable(uint32_t region_number, void *base_address, uint8_t size_log2, int execute, int user_write_access);
void enable_mpu(void);
void initialize_mpu();
void disable_mpu();
void enable_memory_fault(void);

#endif /* _MPU_H_ */
