/*
 * @file: printk.c -- string printing capability that wraps rtt.c
 *
 * @date: last updated 23 February 2021
**/

#include <rtt.h>
#include <printk.h>
#include <arm.h>

static char chr[RTT_PRINTK_BUFFER_SIZE];

/*
 * @function: store_char -- pushes a char into the rtt write buffer
 */
static void store_char(rtt_printk_desc_t* p, char c) {
  uint32_t count;

  count = p->count;
  if((count + 1) <= p->buffer_size) {
    *(p->p_buffer + count) = c;
    p->count = count + 1;
    p->return_value++;
  }
  if(p->count == p->buffer_size) {
    if(rtt_write(p->buffer_index, p->p_buffer, p->count) != p->count) {
      p->return_value = -1;
    } else {
      p->count = 0;
    }
  }
}

/*
 * @function: print_unsigned -- writes a formatted unsigned value to the rtt write buffer
 */
static void print_unsigned(rtt_printk_desc_t* p_buffer_desc, uint32_t num, uint32_t base, uint32_t num_digits, uint32_t field_width, uint32_t format_flags) {
  static const char chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
  uint32_t number, div;
  uint32_t digit, width;
  char c;

  number = num;
  digit = 1;
  width = 1;
  while(number >= base) {
    number = (number / base);
    width++;
  }
  if(num_digits > width) {
    width = num_digits;
  }
  if((format_flags & FORMAT_LEFT_JUSTIFY) == 0) {
    if(field_width != 0) {
      if(((format_flags & FORMAT_PAD_ZERO) == FORMAT_PAD_ZERO) && (num_digits == 0)) {
        c = '0';
      } else {
        c = ' ';
      }
      while((field_width != 0) && (width < field_width)) {
        field_width--;
        store_char(p_buffer_desc, c);
        if(p_buffer_desc->return_value < 0) {
          break;
        }
      }
    }
  }

  if(p_buffer_desc->return_value >= 0) {
    while(1) {
      if(num_digits > 1) {
        num_digits--;
      } else {
        div = num / digit;
        if (div < base) {
          break;
        }
      }
      digit *= base;
    }
    do {
      div = num / digit;
      num -= div * digit;
      store_char(p_buffer_desc, chars[div]);
      if (p_buffer_desc->return_value < 0) {
        break;
      }
      digit /= base;
    } while(digit);

    if((format_flags & FORMAT_LEFT_JUSTIFY) == FORMAT_LEFT_JUSTIFY) {
      if(field_width != 0) {
        while((field_width != 0) && (width < field_width)) {
          field_width--;
          store_char(p_buffer_desc, ' ');
          if(p_buffer_desc->return_value < 0) {
            break;
          }
        }
      }
    }
  }
}

/*
 * @function: print_int -- writes a formatted integer value to the rtt write buffer
 */
static void print_int(rtt_printk_desc_t* p_buffer_desc, int num, uint32_t base, uint32_t num_digits, uint32_t field_width, uint32_t format_flags) {
  uint32_t width;
  int number;

  number = (num < 0) ? -num : num;

  width = 1;
  while(number >= (int)base) {
    number = (number / (int)base);
    width++;
  }
  if(num_digits > width) {
    width = num_digits;
  }
  if((field_width > 0) && ((num < 0) || ((format_flags & FORMAT_PRINT_SIGN) == FORMAT_PRINT_SIGN))) {
    field_width--;
  }

  if((((format_flags & FORMAT_PAD_ZERO) == 0) || (num_digits != 0)) && ((format_flags & FORMAT_LEFT_JUSTIFY) == 0)) {
    if(field_width != 0) {
      while((field_width != 0) && (width < field_width)) {
        field_width--;
        store_char(p_buffer_desc, ' ');
        if(p_buffer_desc->return_value < 0) {
          break;
        }
      }
    }
  }

  if(p_buffer_desc->return_value >= 0) {
    if(num < 0) {
      num = -num;
      store_char(p_buffer_desc, '-');
    } else if ((format_flags & FORMAT_PRINT_SIGN) == FORMAT_PRINT_SIGN) {
      store_char(p_buffer_desc, '+');
    } else {

    }
    if(p_buffer_desc->return_value >= 0) {
      if(((format_flags & FORMAT_PAD_ZERO) == FORMAT_PAD_ZERO) && ((format_flags & FORMAT_LEFT_JUSTIFY) == 0) && (num_digits == 0)) {
        if(field_width != 0) {
          while((field_width != 0) && (width < field_width)) {
            field_width--;
            store_char(p_buffer_desc, '0');
            if(p_buffer_desc->return_value < 0) {
              break;
            }
          }
        }
      }
      if (p_buffer_desc->return_value >= 0) {
        print_unsigned(p_buffer_desc, (uint32_t)num, base, num_digits, field_width, format_flags);
      }
    }
  }
}

/*
 * @function: rtt_vprintk -- writes formatted string to rtt up buffer for host
 *
 * returns number of bytes stored or -1 on error
 */
int rtt_vprintk(uint32_t buffer_index, const char* s_fmt, va_list * p_params) {
  char c;
  rtt_printk_desc_t buffer_desc;
  int v;
  uint32_t num_digits, field_width;
  uint32_t format_flags;
  char c_buffer[RTT_PRINTK_BUFFER_SIZE];

  buffer_desc.p_buffer      = c_buffer;
  buffer_desc.buffer_size   = RTT_PRINTK_BUFFER_SIZE;
  buffer_desc.count         = 0;
  buffer_desc.buffer_index  = buffer_index;
  buffer_desc.return_value  = 0;

  do {
    c = *s_fmt;
    s_fmt++;
    if(c == 0) {
      break;
    }
    if(c == '%') {
      format_flags = 0;
      v = 1;
      do {
        c = *s_fmt;
        switch(c) {
          case '-': format_flags |= FORMAT_LEFT_JUSTIFY; s_fmt++; break;
          case '0': format_flags |= FORMAT_PAD_ZERO;     s_fmt++; break;
          case '+': format_flags |= FORMAT_PRINT_SIGN;   s_fmt++; break;
          case '#': format_flags |= FORMAT_ALTERNATE;    s_fmt++; break;
          default: v = 0; break;
        }
      } while(v);
      field_width = 0;
      do {
        c = *s_fmt;
        if((c < '0') || (c > '9')) {
          break;
        }
        s_fmt++;
        field_width = field_width * 10 + ((uint32_t)c - '0');
      } while(1);

      num_digits = 0;
      c = *s_fmt;
      if(c == '.') {
        s_fmt++;
        do {
          c = *s_fmt;
          if((c < '0') || (c > '9')) {
            break;
          }
          s_fmt++;
          num_digits = num_digits * 10 + ((uint32_t)c - '0');
        } while(1);
      }
      c = *s_fmt;
      do {
        if((c == 'l') || (c == 'h')) {
          s_fmt++;
          c = *s_fmt;
        } else {
          break;
        }
      } while(1);

      switch(c) {
        case 'c': {
          char c0;
          v = va_arg(*p_params, int);
          c0 = (char)v;
          store_char(&buffer_desc, c0);
          break;
        }
        case 'd':
          v = va_arg(*p_params, int);
          print_int(&buffer_desc, v, 10, num_digits, field_width, format_flags);
          break;
        case 'u':
          v = va_arg(*p_params, int);
          print_unsigned(&buffer_desc, (uint32_t)v, 10, num_digits, field_width, format_flags);
          break;
        case 'x':
        case 'X':
          v = va_arg(*p_params, int);
          print_unsigned(&buffer_desc, (uint32_t)v, 16, num_digits, field_width, format_flags);
          break;
        case 's': {
          const char * s = va_arg(*p_params, const char *);
          do {
            c = *s;
            s++;
            if (c == '\0') {
              break;
            }
            store_char(&buffer_desc, c);
          } while(buffer_desc.return_value >= 0);
        }
        break;
      case 'p':
        v = va_arg(*p_params, int);
        print_unsigned(&buffer_desc, (uint32_t)v, 16, 8, 8, 0);
        break;
      case '%':
        store_char(&buffer_desc, '%');
        break;
      default:
        break;
      }
      s_fmt++;
    } else {
      store_char(&buffer_desc, c);
    }
  } while(buffer_desc.return_value >= 0);

  if(buffer_desc.return_value > 0) {
    if(buffer_desc.count != 0) {
      rtt_write(buffer_index, c_buffer, buffer_desc.count);
    }
    buffer_desc.return_value += (int)buffer_desc.count;
  }
  return buffer_desc.return_value;
}

/*
 * @function: rtt_printk -- writes formatted string to rtt up buffer for host
 *
 * returns number of bytes stored or -1 on error
 *
 * supported string format: %[flags][FieldWidth][.Precision]ConversionSpecifier, where:
 * ---- flags include: - for left justify, + for sign extension, 0 for zero-padding
 * ---- conversion specifiers include: c for char, d for signed int, u for unsigned int, x for hex, s for string, p for 8-char hex address
 */
int rtt_printk(uint32_t buffer_index, const char* s_fmt, ...) {
  int r;
  va_list param_list;

  va_start(param_list, s_fmt);
  r = rtt_vprintk(buffer_index, s_fmt, &param_list);
  va_end(param_list);
  return r;
}

/*
 * @function: printk -- simple wrapper for rtt_vprintk that is bound to terminal 0
 *
 */
int printk(const char * s_fmt, ...) {
  int r;
  va_list param_list;

  va_start(param_list, s_fmt);
  r = rtt_vprintk(0, s_fmt, &param_list);
  va_end(param_list);
  return r;
}  

/*
 * @function: printk_no_delay -- simple wrapper for rtt_vprintk that is bound to terminal 0
 *
 */
int printk_no_delay(const char * s_fmt, ...) {
  int r;
  va_list param_list;

  va_start(param_list, s_fmt);
  r = rtt_vprintk(0, s_fmt, &param_list);
  va_end(param_list);
  return r;
}

/*
 * @function: printk integer -- print integer value to terminal 0
 *
 */
void printk_int(int32_t val) {
  uint8_t digit, idx = 0;
  uint8_t start = 0, end;
  
  // neg number
  if (val < 0) {
    chr[0] = '-';
    idx = 1;
    start = 1;
  }
  val = arm_abs(val);

  // get digits
  while (val) {
    digit = val%10;
    chr[idx] = digit + '0';
    idx+=1;
    val/=10;
  }
  end = idx - 1;
  char tmp;
  
  REVERSEARR(chr, start, end, tmp);
  chr[idx++] = '\n';
  chr[idx] = '\0';

  printk((const char *)chr);
}  

