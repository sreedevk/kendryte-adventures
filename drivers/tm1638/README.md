## TM1638 DRIVER (Kendryte SDK)

![IMG_20210303_221434__01](https://user-images.githubusercontent.com/36154121/109840878-871cf200-7c6e-11eb-9654-43ac66d2a9b6.jpg)
TM1638 Display + LED + Button Driver for Kendryte Standalone SDK

## INITIALIZATION

### 1. add this repo as a submodule in the `lib` directory of kendryte standalone SDK
```bash
git submodule add git@github.com:sreedevk/tm1638-driver-kendryte-sdk.git /path/to/kendryte_sdk/lib/tm1638/
```
### 2. Modify ${SDK_ROOT}/lib/CMakeLists.txt

add the following Glob matchers to the `LIB_SRC` GLOB_RECURSE in `${SDK_ROOT}/lib/CMakeLists.txt`:

```cmake
"${CMAKE_CURRENT_LIST_DIR}/tm1638/*.h"
"${CMAKE_CURRENT_LIST_DIR}/tm1638/*.c"
```
### 3. Include  in your project files to use the TM1638 driver.

```c
#include <tm1638.h>
```

## DOCS


## Examples

```c
#include <bsp.h>
#include <sysctl.h>
#include <stdint.h>
#include <syslog.h>
#include <stdbool.h>
#include <fpioa.h>
#include <gpio.h>
#include <tm1638.h>
#include <sleep.h>

#define TM1638_DATA_PIN   27
#define TM1638_CLOCK_PIN  28
#define TM1638_STROBE_PIN 29

tm1638_device_t tubedisp;

int draw_sinewave() {
  uint8_t wave_elements[] = { 0b00000001, 0b01000000, 0b00001000, 0b01000000 };
  uint8_t wave_ptr        = 0;
  uint8_t wave_ptr_offset = 0;

  while(true) {
    wave_ptr = 0; wave_ptr_offset += 1;
    for(uint8_t tube_addr = TM1638_TUBE_1; tube_addr <= TM1638_TUBE_8; tube_addr += 2) {
      tm1638_set_digit(&tubedisp, wave_elements[((wave_ptr + wave_ptr_offset) % 4)], tube_addr);
      wave_ptr++;
    }
    msleep(100);
  }
}

void blink_led() {
  while(true) {
    tm1638_set_led(&tubedisp, TM1638_LED_1, 1);
    sleep(1);
    tm1638_set_led(&tubedisp, TM1638_LED_1, 0);
    sleep(1);
  }
}

int main(void) {
  /* K210 Init */
  sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);

  /* TM1638 INIT */
  tm1638_configure_device(&tubedisp, TM1638_CLOCK_PIN, TM1638_STROBE_PIN, TM1638_DATA_PIN);
  tm1638_initialize(&tubedisp, 7, true);

  /* 7 SEG DISPLAY TEXT */
  char *text = "hello";
  tm1638_display_text(&tubedisp, text, (uint8_t) strlen(text));

  sleep(2);

  /* 7 SEG DISPLAY SINE WAVE */
  register_core1(draw_sinewave, NULL);

  /* LED CONTROL */
  blink_led();

  return 0;
}
```
