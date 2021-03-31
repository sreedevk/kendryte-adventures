#include <lcd1602.h>
#include <stdlib.h>
#include <stdint.h>
#include <fpioa.h>
#include <gpiohs.h>
#include <gpio.h>
#include <sleep.h>
#include <syslog.h>
#include <string.h>

#define MIRROR_BYTE(x) (x<<1&0x10)|(x>>1&0x08)|(x<<3&0x20)|(x>>3&0x04)|(x<<5&0x40)|(x>>5&0x02)|(x<<7&0x80)|(x>>7&0x01)

/* shift register control */
static void clear_register(lcd1602_display_t *display) {
  gpiohs_set_pin(display->v_sreg_clr, GPIO_PV_LOW);
  usleep(10);
  gpiohs_set_pin(display->v_sreg_clr, GPIO_PV_HIGH);
}

void lcd1602_write_byte_to_register(lcd1602_display_t *display, uint8_t payload) {
  clear_register(display);
  for(uint16_t bit_mask = 0x01; bit_mask <= 0x80; bit_mask <<= 1) {
    gpiohs_set_pin(display->v_sreg_clk, GPIO_PV_LOW);
    gpiohs_set_pin(display->v_sreg_dat, (payload & bit_mask) ? GPIO_PV_HIGH : GPIO_PV_LOW);
    gpiohs_set_pin(display->v_sreg_clk, GPIO_PV_HIGH);
  }
}

/* LCD WRITE CMD/DATA */
static void lcd1602_pulse_e(lcd1602_display_t *display) {
  gpiohs_set_pin(display->v_lcd_en, GPIO_PV_LOW);
  usleep(1);
  gpiohs_set_pin(display->v_lcd_en, GPIO_PV_HIGH);
  usleep(80);
  gpiohs_set_pin(display->v_lcd_en, GPIO_PV_LOW);
  usleep(100);
}

void lcd1602_write_command(lcd1602_display_t *display, uint8_t cmd) {
  gpiohs_set_pin(display->v_lcd_rs, LCD1602_DISPLAY_COMMAND);
  gpiohs_set_pin(display->v_lcd_rw, LCD1602_DISPLAY_WRITE);
  lcd1602_write_byte_to_register(display, (display->input_mirrored ? MIRROR_BYTE(cmd) : cmd));
  lcd1602_pulse_e(display);
}

void lcd1602_write_data(lcd1602_display_t *display, uint8_t data) {
  gpiohs_set_pin(display->v_lcd_rs, LCD1602_DISPLAY_DATA);
  gpiohs_set_pin(display->v_lcd_rw, LCD1602_DISPLAY_WRITE);
  lcd1602_write_byte_to_register(display, (display->input_mirrored ? MIRROR_BYTE(data) : data));
  lcd1602_pulse_e(display);
}

/* LCD INIT/CONTROL FUNCTIONS */
void lcd1602_initialize_hal(
  lcd1602_display_t *display, uint8_t sr_clk_pin, uint8_t sr_clr_pin, uint8_t sr_dat_pin,
  uint8_t lcd_en_pin, uint8_t lcd_rs_pin, uint8_t lcd_rw_pin) {

  display->p_sreg_clk = sr_clk_pin;
  display->p_sreg_clr = sr_clr_pin;
  display->p_sreg_dat = sr_dat_pin;
  display->p_lcd_en   = lcd_en_pin;
  display->p_lcd_rs   = lcd_rs_pin;
  display->p_lcd_rw   = lcd_rw_pin;

  display->v_sreg_clk = DEFAULT_SREG_CLK_VIO;
  display->v_sreg_clr = DEFAULT_SREG_CLR_VIO;
  display->v_sreg_dat = DEFAULT_SREG_DAT_VIO;

  display->v_lcd_en   = DEFAULT_LCD_EN_VIO;
  display->v_lcd_rs   = DEFAULT_LCD_RS_VIO;
  display->v_lcd_rw   = DEFAULT_LCD_RW_VIO;

  display->buffer         = 0;
  display->input_mirrored = DEFAULT_BITS_FLIPPED;

  fpioa_set_function(display->p_sreg_clk, DEFAULT_SREG_CLK_FUNC);
  fpioa_set_function(display->p_sreg_clr, DEFAULT_SREG_CLR_FUNC);
  fpioa_set_function(display->p_sreg_dat, DEFAULT_SREG_DAT_FUNC);
  fpioa_set_function(display->p_lcd_en, DEFAULT_LCD_ENABLE_FUNC);
  fpioa_set_function(display->p_lcd_rs, DEFAULT_LCD_RS_FUNC);
  fpioa_set_function(display->p_lcd_rw, DEFAULT_LCD_RW_FUNC);

  gpiohs_set_drive_mode(display->v_sreg_clk, GPIO_DM_OUTPUT);
  gpiohs_set_drive_mode(display->v_sreg_clr, GPIO_DM_OUTPUT);
  gpiohs_set_drive_mode(display->v_sreg_dat, GPIO_DM_OUTPUT);
  gpiohs_set_drive_mode(display->v_lcd_en, GPIO_DM_OUTPUT);
  gpiohs_set_drive_mode(display->v_lcd_rs, GPIO_DM_OUTPUT);
  gpiohs_set_drive_mode(display->v_lcd_rw, GPIO_DM_OUTPUT);


  gpiohs_set_pin(display->v_sreg_clr, GPIO_PV_HIGH);
  gpiohs_set_pin(display->v_sreg_clk, GPIO_PV_HIGH);
  gpiohs_set_pin(display->v_sreg_dat, GPIO_PV_LOW);
  gpiohs_set_pin(display->v_lcd_en, GPIO_PV_LOW);
  gpiohs_set_pin(display->v_lcd_rs, GPIO_PV_LOW);
  gpiohs_set_pin(display->v_lcd_rw, GPIO_PV_LOW);
}

void lcd1602_set_ddram_addr(lcd1602_display_t *display, uint8_t addr) {
  lcd1602_write_command(display, LCD_SET_DDRAM_ADDR | (addr & DDRAM_ADDR_MASK));
}

void lcd1602_set_cgram_addr(lcd1602_display_t *display, uint8_t addr) {
  lcd1602_write_command(display, LCD_SET_CGRAM_ADDR | (addr & CGRAM_ADDR_MASK));
}

/* GFX Functions */

void lcd1602_clear_display(lcd1602_display_t *display) {
  msleep(2);
  lcd1602_write_command(display, LCD_CLR_DISPLAY);
  lcd1602_write_command(display, LCD_RET_HOME);
}

void lcd1602_create_custom_char(lcd1602_display_t *display, lcd1602_5x8_custom_char_t *custom_char) {
  /* we can store upto 8 custom characters on CGRAM addr 0x00 .. 0x07 */
  lcd1602_set_cgram_addr(display, (custom_char->cgram_addr & CGRAM_CUSTOM_CHAR_ADDR_MASK));
  for(int chr_byte=0; chr_byte<8; chr_byte++) {
    lcd1602_write_data(display, custom_char->char_map[chr_byte]);
  }
}

void lcd1602_goto(lcd1602_display_t *display, uint8_t column, uint8_t row) {
  msleep(2);
  lcd1602_set_ddram_addr(display, (row * 64) + column);
}

void lcd1602_initialize_display(lcd1602_display_t *display) {
  msleep(2);
  lcd1602_write_command(display, (LCD_ENTRY_MODESET | LCD_DEMF_ADDR_INCR | LCD_DEMF_SHIFT_DIS ));
  lcd1602_write_command(display, (LCD_DISP_CTRL     | LCD_DCF_DISPLAY_ON | LCD_DCF_CURSOR_OFF | LCD_DCF_BLINK_OFF));
  lcd1602_write_command(display, (LCD_FUNC_SET      | LCD_DFSF_8BIT_MODE | LCD_DFSF_2LINE     | LCD_DFSF_5x8_FONT));
}

void lcd1602_draw_str(lcd1602_display_t *display, const char * text) {
  size_t text_size = strlen(text);
  for(int char_index = 0; char_index < text_size; char_index++) {
    lcd1602_write_data(display, text[char_index]); /* write data to DDRAM */
  }
}
