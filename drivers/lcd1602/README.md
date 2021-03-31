## LCD1602 K210 Driver (Kendryte SDK)

LCD1602 Driver for MAiX Bit / MAiXduino / MAiX Dock (Kendryte SDK). This implementation uses an SN74HC164N PDIP-14 Counter Shift Register IC (Checkout the connection schematic in the README)

### Usage

Connect your parallel output pins Qₐ throught Qₕ SN74HC164N to the datapins of your LCD1602. D₀ through D₇.
Add this repo as a submodule in your kendryte-standalone-sdk project & make sure to include the files in your CMakeLists.txt

NOTE: You can also use my custom kendryte-standalone-sdk template that has drivers LCD1602 & a bunch of other useful libraries I developed.
https://github.com/sreedevk/kendryte-sdk-dev

```c
#include <lcd1602.h>
#include <stdlib.h>

void main() {
  lcd1602_display_t *lcd_display = (lcd1602_display_t *) malloc(sizeof(lcd1602_display_t));

  initialize_lcd_hal(
    lcd_display,
    29, /* shift register clock */
    28, /* shift register clear */
    35, /* shift register data (AND gated input) (tie both inputs of the shift register to this io) */
    10, /* LCD1602 EN pin */
    33, /* LCD1602 RS (Register Select) Pin */
    34, /* LCD1602 RW (Read Write) Pin */
  );

  lcd1602_initialize_display(lcd_display);
  lcd1602_clear_display(lcd_display);
  lcd1602_goto(lcd_display, 0 /*column*/, 0 /*row*/);
  lcd1602_draw_str(lcd_display, "Hello");
}
```

The above program should start up your LCD & Display "Hello" on the first line like shown in the image below.

![IMG_20210327_211810__01](https://user-images.githubusercontent.com/36154121/112726559-21a5e380-8f44-11eb-8aea-05dd94725b63.jpg)



The `lcd1602_display_t` is an HAL for the LCD1602 module. You can modify some settings including the FPIOA assigned multiplexed IOs for the SN74HC164N shift register & the LCD1602. This is the definition of the `lcd1602_display_t` struct.


```c
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
```

1. The instruction bytes in the instruction set on the datasheet for LCD1602A is reversed. The bool member `input_mirrored` of the struct will make sure that the instructions bytes sent to the LCD1602 are mirrored. (you can set this to false, if you connect the shift register outputs in reverse to the data inputs of the lcd).
2. the uint8_t member `buffer` is used internally by the driver & should not be modified.
3. All the uint8_t members starting with the letter `p (for physical)` are the physical pin definitions. 
4. All the uint8_t members starting with the letter `v (for virtual)`  are the fpioa multiplexed GPIOHS pin numbers.

you can modify the HAL struct to suit your requirements in case any of the virtual multiplexed pins are used by other modules in your project.

#### Public Functions of the driver

HAL & Display Control Functions.

1. initialize_lcd_hal         - Initializes the LCD1602 HAL with the configuration provided to it through arguments.
2. lcd1602_initialize_display - Initializes the Display by executing the startup sequence. (Check Datasheet for LCD1602).
3. lcd1602_write_command      - Outputs a command to the shift register, enables CMD Write Pins on the LCD1602 & pulses the EN pin.
4. lcd1602_write_data         - Writes data to the DDRAM / CGRAM of the LCD1602 module.

GFX Functions.
1. lcd1602_clear_display      - Clears the Display.
2. lcd1602_goto               - moves the cursor to the specifed column / row. (can also be used to modify the address counter).
3. lcd1602_draw_str           - Writes a string onto the display where the cursor is at.

