#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
  pin_t    sig;
  uint32_t intensity_attr;
  uint32_t pwm_value; // 0 to 255
  bool     pin_state;
} chip_state_t;

static chip_state_t *chip;

// Simulates an analog voltage using PWM (Pulse Width Modulation)
// This fires every 10 microseconds
void chip_timer_callback(void *user_data) {
  static uint32_t counter = 0;
  
  // Every 1 millisecond, read the slider and update the target PWM value
  if (counter % 100 == 0) {
    uint32_t inten = attr_read(chip->intensity_attr); // 0 to 100
    // Map 0-100% to a PWM value of 0-255 (Max ADC reading)
    chip->pwm_value = (inten * 255) / 100;
  }

  // Generate the PWM signal (0-255 scale)
  uint32_t cycle_pos = counter % 255;
  bool new_state = (cycle_pos < chip->pwm_value);
  
  // Only write to the pin if the state changed (to save simulator CPU)
  if (new_state != chip->pin_state) {
    pin_write(chip->sig, new_state ? HIGH : LOW);
    chip->pin_state = new_state;
  }
  
  counter++;
}

void chip_init(void) {
  chip = (chip_state_t *)malloc(sizeof(chip_state_t));
  
  // IMPORTANT: Changed to OUTPUT instead of ANALOG
  chip->sig            = pin_init("SIG", OUTPUT);      
  chip->intensity_attr = attr_init("intensity", 0);   
  chip->pwm_value      = 0;
  chip->pin_state      = false;

  pin_write(chip->sig, LOW);

  // Fast timer for PWM generation (10us = 100kHz base frequency)
  timer_config_t t_cfg = {
    .callback  = chip_timer_callback,
    .user_data = NULL
  };
  timer_t t = timer_init(&t_cfg);
  timer_start(t, 10, true);
}
