/*
 * @file: rtt.c -- real-time transfer for debugging
 *
 * @date: last updated 23 February 2021
 * 
 * @desc: this is the RTT client to run on the "device" side to interact with "host"
 *        -- RTT operates over a debug channel using a "control block" data structure
 *        -- multiple channels of input and output can be supported (up to 16) for multiplexing
 *        -- control block contains:
 *             ---- an array of "up" buffers for writing data to the host (buffer 0 ==> "terminal")
 *             ---- an array of "down" buffers for reading input from the host (buffer 0 ==> "terminal")
 *             ---- size of each up and down buffer array
 *             ---- an id that allows the RTT tool on "host" to identify the control block
 *        -- each up/down buffer is a ring buffer or circular array, meaning reads and writes wrap back to index 0
 *        -- each ring buffer maintains two indices for last written and last read entry
**/

#include <rtt.h>
#include <printk.h>
#include <arm.h>

extern rtt_control_t __rtt_start;

static char up_buffer[BUFFER_SIZE_UP];
static char down_buffer[BUFFER_SIZE_DOWN];

/*
 * @brief: initialize control blocks, must be called before any other rtt ops
 * 
 * @note: this is provided for you and should not be changed in any way
 */
void rtt_init() {
  rtt_control_t* p;
  p = &__rtt_start;
  p->num_up_buffers = RTT_MAX_UP_BUFFERS;
  p->num_down_buffers = RTT_MAX_DOWN_BUFFERS;

  p->up_buffers[0].name = "Terminal";
  p->up_buffers[0].p_buffer = up_buffer;
  p->up_buffers[0].buffer_size = BUFFER_SIZE_UP;
  p->up_buffers[0].pos_rd = 0;
  p->up_buffers[0].pos_wr = 0;
  p->up_buffers[0].flags = 2;

  p->down_buffers[0].name = "Terminal";
  p->down_buffers[0].p_buffer = down_buffer;
  p->down_buffers[0].buffer_size = BUFFER_SIZE_DOWN;
  p->down_buffers[0].pos_rd = 0;
  p->down_buffers[0].pos_wr = 0;
  p->down_buffers[0].flags = 2;

  p->id[7] = 'R'; p->id[8] = 'T'; p->id[9] = 'T';
  RTT__DMB();
  p->id[0] = 'S'; p->id[1] = 'E'; p->id[2] = 'G'; p->id[3] = 'G'; p->id[4] = 'E'; p->id[5] = 'R';
  RTT__DMB();
  p->id[6] = ' ';
  RTT__DMB();
  p->id[10] = '\0'; p->id[11] = '\0'; p->id[12] = '\0'; p->id[13] = '\0'; p->id[14] = '\0'; p->id[15] = '\0';
  RTT__DMB();
}

/**
 * Write data by rtt.
 */
uint32_t rtt_write(uint32_t buffer_index, const void* p_buffer, uint32_t num_bytes) {
  volatile rtt_buffer_up_t * buffer = get_rtt_buffer_up(buffer_index);
  char* start = (char*)p_buffer;
  char* nxt_wr = (char*) buffer->p_buffer + buffer->pos_wr;

  for (uint32_t i = 0; i < num_bytes; i++){
    //  implemented blocking mechanism
    while ((nxt_wr + 1 == buffer->pos_rd + buffer->p_buffer) | (nxt_wr + 1 == (buffer->p_buffer + buffer->buffer_size) && buffer->pos_rd == 0 ));
    *nxt_wr = start[i];
    nxt_wr++;
    if (nxt_wr >= (buffer->p_buffer + buffer->buffer_size)) {
      nxt_wr = buffer->p_buffer;
    }
  }
  buffer->pos_wr = (uint32_t)(nxt_wr - buffer->p_buffer);
  return num_bytes;
}


/**
 * Read data from rtt.
 */
uint32_t rtt_read(uint32_t buffer_index, void* p_buffer, uint32_t buffer_size) {
  volatile rtt_buffer_down_t *buffer = get_rtt_buffer_down(buffer_index); 
  char* end_of_buffer = buffer->p_buffer + buffer->buffer_size;
  char *read_idx = (char*)p_buffer;
  char* nxt_rd = (char*)buffer->p_buffer + buffer->pos_rd;

  for (int i = 0; i < (int)buffer_size; i++)
  {
    while (nxt_rd == buffer->p_buffer + buffer->pos_wr);
    *read_idx = *nxt_rd;
    read_idx++;
    nxt_rd++;
    if (nxt_rd >= end_of_buffer) {
      nxt_rd = buffer->p_buffer;
    }
    // nxt_rd = nxt_rd < end_of_buffer ? nxt_rd : buffer->p_buffer;
  }

  buffer->pos_rd = nxt_rd - buffer->p_buffer;
  return buffer_size;
}


/**
 * Check if there is any data in dowm buffer.
 */
uint32_t rtt_has_data(uint32_t buffer_index) {
  volatile rtt_buffer_down_t *buffer = get_rtt_buffer_down(buffer_index); 
  uint32_t write_pos= buffer->pos_wr >= buffer->pos_rd ? buffer->pos_wr : buffer->pos_wr + buffer->buffer_size;
  return write_pos - buffer->pos_rd;
}



/**
 * Get down buffer.
 */
rtt_buffer_down_t* get_rtt_buffer_down(uint32_t buffer_index) {
  rtt_control_t* ptr = &__rtt_start;
  return ptr->down_buffers + buffer_index;
}

/**
 * Get up buffer.
 */
rtt_buffer_up_t* get_rtt_buffer_up(uint32_t buffer_index) {
  rtt_control_t* ptr = &__rtt_start;
  return ptr->up_buffers + buffer_index;
}

/**
 * Write data to buffer.
 */
void buffer_write(uint32_t bytes, volatile char* dst, volatile char* src) {
  while (bytes) {
    *dst = *src;
    dst++;
    src++;
    bytes--;
  }
}

/**
 * Clear buffer.
 */
void clear_buffer() {
  for (int i = 0; i < BUFFER_SIZE_UP; i++) {
    up_buffer[i] = 0;
  }
  for (int i = 0; i < BUFFER_SIZE_DOWN; i++) {
    down_buffer[i] = 0;
  }
}

/**
 * Clear read buffer.
 */
void clear_read_buffer() {
  for (int i = 0; i < BUFFER_SIZE_DOWN; i++) {
    down_buffer[i] = 0;
  }
}

/**
 * Clear write buffer.
 */
void clear_write_buffer() {
  for (int i = 0; i < BUFFER_SIZE_UP; i++) {
    up_buffer[i] = 0;
  }
}

/**
 * Test rtt functions.
 */
void rtt_write_test() {
  rtt_init();
  printk("Let's start counting from 1 to 10\n");
  for (int i = 1; i <= 10; i++) {
    printk_int(i);
  }

  printk("Let's start counting from -1 to -10\n");
  for (int i = -1; i >= -10; i--) {
    printk_int(i);
  }
}