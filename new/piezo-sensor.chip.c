#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  pin_t    sig;
  uint32_t intensity_attr;
} chip_state_t;

void chip_timer_callback(void *user_data) {
  chip_state_t *chip = (chip_state_t *)user_data;

  uint32_t intensity = attr_read(chip->intensity_attr);
  

  float voltage = (intensity / 100.0f) * 3.3f;
  

  pin_dac_write(chip->sig, voltage);
}

void chip_init(void) {
  chip_state_t *chip = (chip_state_t *)malloc(sizeof(chip_state_t));
  
  chip->sig            = pin_init("SIG", ANALOG);      
  chip->intensity_attr = attr_init("intensity", 0);   


  pin_dac_write(chip->sig, 0.0f);

  timer_config_t t_cfg = {
    .callback  = chip_timer_callback,
    .user_data = chip
  };
  timer_t t = timer_init(&t_cfg);
  timer_start(t, 50000, true);  
}
