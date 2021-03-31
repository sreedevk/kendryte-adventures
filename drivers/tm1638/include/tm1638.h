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

#pragma once
#include <stdint.h>
#include <stdbool.h>

#define TM1638_DATA_PIN   27
#define TM1638_CLOCK_PIN  28
#define TM1638_STROBE_PIN 29

#define TM1638_CLK_DELAY 1

typedef struct {
  uint8_t strobe_pio;
  uint8_t clock_pio;
  uint8_t data_pio;

  uint8_t strobe_vio;
  uint8_t clock_vio;
  uint8_t data_vio;
  uint8_t display_register[16];
} tm1638_device_t;

/* clock display map */
/* 0b10000000 -> .   */
/* 0b01000000 -> MM  */
/* 0b00100000 -> LT  */
/* 0b00010000 -> LB  */
/* 0b00001000 -> MB  */
/* 0b00000100 -> RB  */
/* 0b00000010 -> RT  */
/* 0b00000001 -> RT  */

typedef enum {
  TM1638_CHAR_A   = 0x77,
  TM1638_CHAR_B   = 0x7F,
  TM1638_CHAR_C   = 0x39,
  TM1638_CHAR_D   = 0x1F,
  TM1638_CHAR_E   = 0x79,
  TM1638_CHAR_F   = 0x71,
  TM1638_CHAR_G   = 0x3C,
  TM1638_CHAR_H   = 0x76,
  TM1638_CHAR_I   = 0x30,
  TM1638_CHAR_J   = 0xE0,
  TM1638_CHAR_K   = 0x75,
  TM1638_CHAR_L   = 0x38,
  TM1638_CHAR_M   = 0x2B,
  TM1638_CHAR_N   = 0x37,
  TM1638_CHAR_O   = 0x3F,
  TM1638_CHAR_P   = 0x73,
  TM1638_CHAR_Q   = 0x6B,
  TM1638_CHAR_R   = 0x7B,
  TM1638_CHAR_S   = 0x6D,
  TM1638_CHAR_T   = 0x31,
  TM1638_CHAR_U   = 0x3E,
  TM1638_CHAR_V   = 0x2E,
  TM1638_CHAR_W   = 0x1D,
  TM1638_CHAR_X   = 0x49,
  TM1638_CHAR_Y   = 0x6E,
  TM1638_CHAR_Z   = 0x5B,

  TM1638_CHAR_0   = 0x3F,
  TM1638_CHAR_1   = 0x30,
  TM1638_CHAR_2   = 0x5B,
  TM1638_CHAR_3   = 0x4F,
  TM1638_CHAR_4   = 0x66,
  TM1638_CHAR_5   = 0x6D,
  TM1638_CHAR_6   = 0x7C,
  TM1638_CHAR_7   = 0x70,
  TM1638_CHAR_8   = 0x7F,
  TM1638_CHAR_9   = 0x6F,
  TM1638_CHAR_DOT = 0x80
} tm1638_font_t;

typedef enum {
  TM1638_LED_1 = 0xC1,
  TM1638_LED_2 = 0xC3,
  TM1638_LED_3 = 0xC5,
  TM1638_LED_4 = 0xC7,
  TM1638_LED_5 = 0xC9,
  TM1638_LED_6 = 0xCB,
  TM1638_LED_7 = 0xCD,
  TM1638_LED_8 = 0xCF
} tm1638_led_address_t;

typedef enum {
  TM1638_TUBE_1 = 0xC0,
  TM1638_TUBE_2 = 0xC2,
  TM1638_TUBE_3 = 0xC4,
  TM1638_TUBE_4 = 0xC6,
  TM1638_TUBE_5 = 0xC8,
  TM1638_TUBE_6 = 0xCA,
  TM1638_TUBE_7 = 0xCC,
  TM1638_TUBE_8 = 0xCE
} tm1638_display_address_t;

typedef enum {
  DISPLAY_ISEQ          = 0x80,
  DATA_ISEQ             = 0x40,
  ADDRESS_ISEQ          = 0xC0,

  TURN_DISPLAY_ON       = 0x08,
  TURN_DISPLAY_OFF      = 0x00,
  ADDRESS_AUTO_INCR     = 0x40,
  SINGLE_ADDRESS_MODE   = 0x44,
  NORMAL_MODE           = 0x00,
  READ_KEYSCAN_DATA     = 0x02
} tm1638_command_t;

typedef enum {
  TM1638_OK  = 0,
  TM1638_ERR = 1
} tm1638_result_t;


tm1638_result_t tm1638_configure_device(tm1638_device_t *dev, uint8_t clk_pin, uint8_t stb_pin, uint8_t dat_pin);
tm1638_result_t tm1638_initialize(tm1638_device_t *dev, uint8_t brightness, bool display_state);
void tm1638_clock_high(tm1638_device_t *dev);
void tm1638_clock_low(tm1638_device_t *dev);
void tm1638_strobe_high(tm1638_device_t *dev);
void tm1638_strobe_low(tm1638_device_t *dev);
void tm1638_data_high(tm1638_device_t *dev);
void tm1638_data_low(tm1638_device_t *dev);
void tm1638_data_set(tm1638_device_t *dev, uint8_t bitval);
void tm1638_clk_delay_us(uint32_t delay_period);

tm1638_result_t tm1638_write_bytes(tm1638_device_t *dev, uint8_t *data, uint8_t datalen);
tm1638_result_t tm1638_read_bytes(tm1638_device_t *dev, uint8_t *data, uint8_t datalen);

/* driver functions */
tm1638_result_t tm1638_set_led(tm1638_device_t *dev, uint8_t led_addr, uint8_t led_status);
tm1638_result_t tm1638_set_digit(tm1638_device_t *dev, uint8_t digit_data, uint8_t display_addr);
tm1638_result_t tm1638_display_text(tm1638_device_t *dev, char *data, uint8_t datalen);
tm1638_result_t tm1638_scan_key_registers(tm1638_device_t *dev, uint8_t *keys_registers_buffer);
tm1638_result_t tm1638_scan_keys(tm1638_device_t *dev, uint32_t *keys);
