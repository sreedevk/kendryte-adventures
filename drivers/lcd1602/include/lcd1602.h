#pragma once
#include <fpioa.h>
#include <gpiohs.h>
#include <stdint.h>
#include <gpio.h>
#include <stdbool.h>

/* SHIFT REGISTER DEFAULTS */
#define DEFAULT_SREG_CLK_FUNC FUNC_GPIOHS10
#define DEFAULT_SREG_CLR_FUNC FUNC_GPIOHS11
#define DEFAULT_SREG_DAT_FUNC FUNC_GPIOHS12

#define DEFAULT_SREG_CLK_VIO  10
#define DEFAULT_SREG_CLR_VIO  11
#define DEFAULT_SREG_DAT_VIO  12

/* LCD1602 DEFAULTS */
#define DEFAULT_LCD_ENABLE_FUNC FUNC_GPIOHS13
#define DEFAULT_LCD_RS_FUNC     FUNC_GPIOHS14
#define DEFAULT_LCD_RW_FUNC     FUNC_GPIOHS15

#define DEFAULT_LCD_EN_VIO  13
#define DEFAULT_LCD_RS_VIO  14
#define DEFAULT_LCD_RW_VIO  15

#define DEFAULT_BITS_FLIPPED true

/* ADDRESS MASKS */
#define DDRAM_ADDR_MASK               0x7F
#define CGRAM_ADDR_MASK               0x3F
#define CGRAM_CUSTOM_CHAR_ADDR_MASK   0x07

typedef enum {
  LCD1602_DISPLAY_WRITE = GPIO_PV_LOW,
  LCD1602_DISPLAY_READ  = GPIO_PV_HIGH
} lcd1602_rw_t;

typedef enum {
  LCD1602_DISPLAY_COMMAND = GPIO_PV_LOW,
  LCD1602_DISPLAY_DATA    = GPIO_PV_HIGH
} lcd1602_rs_t;

typedef enum {
  LCD1602_DISPLAY_4BIT_MODE = 0,
  LCD1602_DISPLAY_8BIT_MODE = 1
} lcd1602_dataline_t;

typedef enum {
  LCD1602_DISPLAY_4LINE_MODE = 0,
  LCD1602_DISPLAY_2LINE_MODE = 1
} lcd1602_display_line_count_t;

typedef enum {
  LCD1602_5x8_FONT = 0,
  LCD1602_5xA_FONT = 1
} lcd1602_cfont_t;

typedef struct {
  uint8_t buffer; 
  /* shift register clock */
  uint8_t p_sreg_clk;
  uint8_t v_sreg_clk;
  /* shift register clear */
  uint8_t p_sreg_clr;
  uint8_t v_sreg_clr;
  /* shift register data */
  uint8_t p_sreg_dat;
  uint8_t v_sreg_dat;
  
  /* lcd enable */
  uint8_t p_lcd_en;
  uint8_t v_lcd_en;
  /* lcd register select */
  uint8_t p_lcd_rs;
  uint8_t v_lcd_rs;
  /* lcd read write */
  uint8_t p_lcd_rw;
  uint8_t v_lcd_rw;

  bool input_mirrored;
} lcd1602_display_t;

typedef enum {
  LCD_CLR_DISPLAY     = 0x01,
  LCD_RET_HOME        = 0x02,
  LCD_ENTRY_MODESET   = 0x04,
  LCD_DISP_CTRL       = 0x08,
  LCD_CURSOR_SHIFT    = 0x10,
  LCD_FUNC_SET        = 0x20,
  LCD_SET_CGRAM_ADDR  = 0x40,
  LCD_SET_DDRAM_ADDR  = 0x80
} lcd1602_cmd_t;

typedef enum {
  LCD_DEMF_ADDR_DECR    = 0x00,
  LCD_DEMF_ADDR_INCR    = 0x02,
  LCD_DEMF_SHIFT_EN     = 0x01,
  LCD_DEMF_SHIFT_DIS    = 0x00
} lcd1602_display_entry_mode_flag_t;

typedef enum {
  LCD_DCF_DISPLAY_ON  = 0x04,
  LCD_DCF_DISPLAY_OFF = 0x00,
  LCD_DCF_CURSOR_ON   = 0x02,
  LCD_DCF_CURSOR_OFF  = 0x00,
  LCD_DCF_BLINK_ON    = 0x01,
  LCD_DCF_BLINK_OFF   = 0x00
} lcd1602_display_ctrl_flag_t;

typedef enum {
  LCD_DCSF_DISPLAY_MOVE = 0x08,
  LCD_DCSF_CURSOR_MOVE  = 0x00,
  LCD_DCSF_MOVE_RIGHT   = 0x04,
  LCD_DCSF_MOVE_LEFT    = 0x00
} lcd1602_display_cursor_shift_flag_t;

typedef enum {
  LCD_DFSF_8BIT_MODE    = 0x10,
  LCD_DFSF_4BIT_MODE    = 0x00,
  LCD_DFSF_2LINE        = 0x08,
  LCD_DFSF_1LINE        = 0x00,
  LCD_DFSF_5x10_FONT    = 0x04,
  LCD_DFSF_5x8_FONT     = 0x00,
} lcd1602_display_function_set_flag_t;

typedef enum {
  LCD1602_ROW0_DDRAM_ADDR = 0x80,
  LCD1602_ROW1_DDRAM_ADDR = 0xC0
} lcd1602_ddram_row_addr_t;

typedef struct {
  uint8_t cgram_addr; 
  uint8_t char_map[8];
} lcd1602_5x8_custom_char_t;

void lcd1602_initialize_hal(
  lcd1602_display_t *display,
  uint8_t sr_clk_pin,
  uint8_t sr_clr_pin,
  uint8_t sr_dat_pin,
  uint8_t lcd_en_pin,
  uint8_t lcd_rs_pin,
  uint8_t lcd_rw_pin
);
void lcd1602_initialize_display(lcd1602_display_t *display);
void lcd1602_write_byte_to_register(lcd1602_display_t *display, uint8_t payload);
void lcd1602_write_command(lcd1602_display_t *display, uint8_t cmd);
void lcd1602_write_data(lcd1602_display_t *display, uint8_t data);


/* gfx */
void lcd1602_clear_display(lcd1602_display_t *display);
void lcd1602_goto(lcd1602_display_t *display, uint8_t column, uint8_t row);
void lcd1602_draw_str(lcd1602_display_t *display, const char * text);
void lcd1602_create_custom_char(lcd1602_display_t *display, lcd1602_5x8_custom_char_t *custom_char);
