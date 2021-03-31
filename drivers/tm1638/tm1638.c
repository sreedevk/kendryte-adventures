/*  TM1638 Driver for Kendryte Standalone SDK 
 *
 * Copyright (c) 2021 Sreedev kodichath (MIT License)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 */

#include <gpio.h>
#include <fpioa.h>
#include <gpiohs.h>
#include <tm1638.h>
#include <string.h>
#include <syslog.h>
#include <stdint.h>

tm1638_result_t tm1638_configure_device(tm1638_device_t *dev, uint8_t clk_pin, uint8_t stb_pin, uint8_t dat_pin) {
  /* set pins on device handler */
  dev->clock_pio                 = clk_pin;
  dev->strobe_pio                = stb_pin;
  dev->data_pio                  = dat_pin;

  memset(dev->display_register, 0, 16);

  /* initialize_pin_functions */
  fpioa_set_function(dev->clock_pio, FUNC_GPIO1);
  fpioa_set_function(dev->strobe_pio, FUNC_GPIO2);
  fpioa_set_function(dev->data_pio, FUNC_GPIO3);

  dev->clock_vio  = 1;
  dev->strobe_vio = 2;
  dev->data_vio   = 3;

  /* set gpio directions */
  gpio_init();
  gpio_set_drive_mode(dev->clock_vio, GPIO_DM_OUTPUT);
  gpio_set_drive_mode(dev->strobe_vio, GPIO_DM_OUTPUT);
  gpio_set_drive_mode(dev->data_vio, GPIO_DM_OUTPUT);

  tm1638_clock_high(dev);
  tm1638_strobe_high(dev);
  tm1638_data_high(dev);

  return TM1638_OK;
}

/* IO FUNCTIONS */
void tm1638_clock_high(tm1638_device_t *dev) { gpio_set_pin(dev->clock_vio, GPIO_PV_HIGH);  }
void tm1638_clock_low(tm1638_device_t *dev)  { gpio_set_pin(dev->clock_vio, GPIO_PV_LOW);   }
void tm1638_strobe_high(tm1638_device_t *dev){ gpio_set_pin(dev->strobe_vio, GPIO_PV_HIGH); }
void tm1638_strobe_low(tm1638_device_t *dev) { gpio_set_pin(dev->strobe_vio, GPIO_PV_LOW);  }
void tm1638_data_high(tm1638_device_t *dev)  { gpio_set_pin(dev->data_vio, GPIO_PV_HIGH);   }
void tm1638_data_low(tm1638_device_t *dev)   { gpio_set_pin(dev->data_vio, GPIO_PV_LOW);    }

void tm1638_data_set(tm1638_device_t *dev, uint8_t bitval) {
  bitval ? tm1638_data_high(dev) : tm1638_data_low(dev);
}

void tm1638_clk_delay_us(uint32_t delay_period) {
  for(uint32_t delay_ticker = 0; delay_ticker < (1000 * delay_period); delay_ticker++) {
    delay_ticker = delay_ticker;
  }
}

tm1638_result_t tm1638_write_bytes(tm1638_device_t *dev, uint8_t *data, uint8_t datalen) {
  gpio_set_drive_mode(dev->data_vio, GPIO_DM_OUTPUT);
  uint8_t byte_c, bit_c, byte_buffer;
  for(byte_c = 0; byte_c < datalen; byte_c++) {
    byte_buffer = data[byte_c];
    for(bit_c=0; bit_c < 8; bit_c++) {
      tm1638_clock_low(dev);
      tm1638_clk_delay_us(TM1638_CLK_DELAY);
      tm1638_data_set(dev, (byte_buffer & 0x01));
      tm1638_clock_high(dev);
      tm1638_clk_delay_us(TM1638_CLK_DELAY);
      byte_buffer >>= 1;
    }
  }

  return TM1638_OK;
}

tm1638_result_t tm1638_read_bytes(tm1638_device_t *dev, uint8_t *data_buffer, uint8_t datalen) {
  uint8_t byte_c, bit_c;
  uint8_t byte_buffer = 0;
  gpio_set_drive_mode(dev->data_vio, GPIO_DM_INPUT);
  tm1638_clk_delay_us(5);

  for(byte_c = 0; byte_c < datalen; byte_c++) {
    for(bit_c = 0; bit_c < 8; bit_c++) {
      tm1638_clock_low(dev);        
      tm1638_clk_delay_us(TM1638_CLK_DELAY);
      tm1638_clock_high(dev);        
      byte_buffer |= (gpio_get_pin(dev->data_vio) << bit_c);
      tm1638_clk_delay_us(TM1638_CLK_DELAY);
    }
    data_buffer[byte_c] = byte_buffer;
  }

  return TM1638_OK;
}

/* DRIVER FUNCTIONS */

tm1638_result_t tm1638_initialize(tm1638_device_t *dev, uint8_t brightness, bool display_state) {
  /* STARTUP DEVICE */
  uint8_t payload = DISPLAY_ISEQ;
  payload        |= brightness & 0x07;
  payload        |= (display_state) ? TURN_DISPLAY_ON : TURN_DISPLAY_OFF;

  tm1638_strobe_low(dev);
  tm1638_write_bytes(dev, &payload, 1);
  tm1638_strobe_high(dev);

  /* RESET DISPLAYS & LEDS */
  payload = DATA_ISEQ;
  tm1638_strobe_low(dev);
  tm1638_write_bytes(dev, &payload, 1);
  tm1638_strobe_high(dev);

  payload = ADDRESS_ISEQ;
  tm1638_strobe_low(dev);
  tm1638_write_bytes(dev, &payload, 1);
  for(uint8_t current_addr=0; current_addr < 16; current_addr++) { 
    payload = 0x00;
    tm1638_write_bytes(dev, &payload, 1);
  }
  tm1638_strobe_high(dev);

  /* set single address mode */
  payload = SINGLE_ADDRESS_MODE;
  tm1638_strobe_low(dev);
  tm1638_write_bytes(dev, &payload, 1);
  tm1638_strobe_high(dev);

  return TM1638_OK;
}

tm1638_result_t tm1638_clear_display(tm1638_device_t *dev) {
  /* RESET DISPLAYS & LEDS */
  uint8_t payload = DISPLAY_ISEQ;
  payload = DATA_ISEQ;
  tm1638_strobe_low(dev);
  tm1638_write_bytes(dev, &payload, 1);
  tm1638_strobe_high(dev);

  payload = ADDRESS_ISEQ;
  tm1638_strobe_low(dev);
  tm1638_write_bytes(dev, &payload, 1);
  for(uint8_t current_addr=0; current_addr < 16; current_addr++) { 
    payload = 0x00;
    tm1638_write_bytes(dev, &payload, 1);
  }
  tm1638_strobe_high(dev);
  return TM1638_OK;
}


tm1638_result_t tm1638_set_led(tm1638_device_t *dev, uint8_t led_addr, uint8_t led_status)  {
  tm1638_strobe_low(dev);
  tm1638_write_bytes(dev, &led_addr, 1);
  tm1638_write_bytes(dev, &led_status, 1);
  tm1638_strobe_high(dev);
  return TM1638_OK;
}

tm1638_result_t tm1638_set_digit(tm1638_device_t *dev, uint8_t digit_data, uint8_t display_addr) {
  tm1638_strobe_low(dev);
  tm1638_write_bytes(dev, &display_addr, 1);
  tm1638_write_bytes(dev, &digit_data, 1);
  tm1638_strobe_high(dev);
  return TM1638_OK;
}

tm1638_result_t tm1638_display_text(tm1638_device_t *dev, char *data, uint8_t datalen) {
  tm1638_clear_display(dev);
  uint8_t current_addr = TM1638_TUBE_1;
  if(datalen > 8) return TM1638_ERR;

  uint8_t next_char = 0;
  for(uint8_t data_ptr=0; data_ptr < datalen; data_ptr++) {
    switch(data[data_ptr]) {
      case 'A':
      case 'a':
        next_char = TM1638_CHAR_A;
        break;
      case 'B':
      case 'b':
        next_char = TM1638_CHAR_B;
        break;
      case 'C':
      case 'c':
        next_char = TM1638_CHAR_C;
        break;
      case 'D':
      case 'd':
        next_char = TM1638_CHAR_D;
        break;
      case 'E':
      case 'e':
        next_char = TM1638_CHAR_E;
        break;
      case 'F':
      case 'f':
        next_char = TM1638_CHAR_F;
        break;
      case 'G':
      case 'g':
        next_char = TM1638_CHAR_G;
        break;
      case 'H':
      case 'h':
        next_char = TM1638_CHAR_H;
        break;
      case 'I':
      case 'i':
        next_char = TM1638_CHAR_I;
        break;
      case 'J':
      case 'j':
        next_char = TM1638_CHAR_J;
        break;
      case 'K':
      case 'k':
        next_char = TM1638_CHAR_K;
        break;
      case 'L':
      case 'l':
        next_char = TM1638_CHAR_L;
        break;
      case 'M':
      case 'm':
        next_char = TM1638_CHAR_M;
        break;
      case 'N':
      case 'n':
        next_char = TM1638_CHAR_N;
        break;
      case 'O':
      case 'o':
        next_char = TM1638_CHAR_O;
        break;
      case 'P':
      case 'p':
        next_char = TM1638_CHAR_P;
        break;
      case 'Q':
      case 'q':
        next_char = TM1638_CHAR_Q;
        break;
      case 'R':
      case 'r':
        next_char = TM1638_CHAR_R;
        break;
      case 'S':
      case 's':
        next_char = TM1638_CHAR_S;
        break;
      case 'T':
      case 't':
        next_char = TM1638_CHAR_T;
        break;
      case 'U':
      case 'u':
        next_char = TM1638_CHAR_U;
        break;
      case 'V':
      case 'v':
        next_char = TM1638_CHAR_V;
        break;
      case 'W':
      case 'w':
        next_char = TM1638_CHAR_W;
        break;
      case 'X':
      case 'x':
        next_char = TM1638_CHAR_X;
        break;
      case 'Y':
      case 'y':
        next_char = TM1638_CHAR_Y;
        break;
      case 'Z':
      case 'z':
        next_char = TM1638_CHAR_Z;
        break;
      case '1':
        next_char = TM1638_CHAR_1;
        break;
      case '2':
        next_char = TM1638_CHAR_2;
        break;
      case '3':
        next_char = TM1638_CHAR_3;
        break;
      case '4':
        next_char = TM1638_CHAR_4;
        break;
      case '5':
        next_char = TM1638_CHAR_5;
        break;
      case '6':
        next_char = TM1638_CHAR_6;
        break;
      case '7':
        next_char = TM1638_CHAR_7;
        break;
      case '8':
        next_char = TM1638_CHAR_8;
        break;
      case '9':
        next_char = TM1638_CHAR_9;
        break;
      case '0':
        next_char = TM1638_CHAR_0;
        break;
      default:
        break;
    }
    
    tm1638_set_digit(dev, next_char, current_addr);
    current_addr += 2;
  }
  return TM1638_OK;
}

tm1638_result_t tm1638_scan_key_registers(tm1638_device_t *dev, uint8_t *keys_registers_buffer) {
  uint8_t payload = DATA_ISEQ | READ_KEYSCAN_DATA;
  tm1638_strobe_low(dev);
  tm1638_write_bytes(dev, &payload, 1);
  tm1638_read_bytes(dev, keys_registers_buffer, 4);
  tm1638_strobe_high(dev); 
  return TM1638_OK;
}

tm1638_result_t tm1638_scan_keys(tm1638_device_t *dev, uint32_t *keys) {
  uint8_t  key_registers[4];
  uint8_t  key_addr     = 0x01;
  uint32_t keys_buffer  = 0;
  tm1638_scan_key_registers(dev, key_registers);
  for(uint8_t key_reg_c = 0; key_reg_c < 3; key_reg_c++) {
    for(int reg_byte_c = 3; reg_byte_c >= 0; reg_byte_c--) {
      keys_buffer <<= 1;
      if(key_registers[reg_byte_c] & (key_addr << 4)) keys_buffer |= 1;

      keys_buffer <<= 1;
      if(key_registers[reg_byte_c] & key_addr) keys_buffer |= 1;
    }

    key_addr <<= 1;
  }
  *keys = keys_buffer;
  return TM1638_OK;
}
