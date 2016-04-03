/*
  Copyright (c) 2014 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef NRF51

#include "nrf_timer.h"
#include "nrf_gpio.h"

#include "Arduino.h"
#include "wiring_private.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_COUNT 3
#define PIN_FREE 0xffffffff

struct PWMContext {
  uint32_t pin;
  uint32_t value;
  nrf_timer_cc_channel_t channel;
  nrf_timer_int_mask_t mask;
  nrf_timer_event_t event;
};

static struct PWMContext pwmContext[PWM_COUNT] = {
  { PIN_FREE, 0, NRF_TIMER_CC_CHANNEL1, NRF_TIMER_INT_COMPARE1_MASK, NRF_TIMER_EVENT_COMPARE1 },
  { PIN_FREE, 0, NRF_TIMER_CC_CHANNEL2, NRF_TIMER_INT_COMPARE2_MASK, NRF_TIMER_EVENT_COMPARE2 },
  { PIN_FREE, 0, NRF_TIMER_CC_CHANNEL3, NRF_TIMER_INT_COMPARE3_MASK, NRF_TIMER_EVENT_COMPARE3 }
};

static int timerEnabled = 0;

static uint32_t readResolution = 10;
static uint32_t writeResolution = 8;

void analogReadResolution( int res )
{
  readResolution = res;
}

void analogWriteResolution( int res )
{
  writeResolution = res;
}

static inline uint32_t mapResolution( uint32_t value, uint32_t from, uint32_t to )
{
  if ( from == to )
  {
    return value ;
  }

  if ( from > to )
  {
    return value >> (from-to) ;
  }
  else
  {
    return value << (to-from) ;
  }
}

/*
 * Internal Reference is at 1.0v
 * External Reference should be between 1v and VDDANA-0.6v=2.7v
 *
 * Warning : On Arduino Zero board the input/output voltage for SAMD21G18 is 3.3 volts maximum
 */
void analogReference( eAnalogReference ulMode )
{
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite( uint32_t ulPin, uint32_t ulValue )
{
  if (ulPin >= PINS_COUNT) {
    return;
  }

  ulPin = g_ADigitalPinMap[ulPin];

  if (!timerEnabled) {
    NVIC_SetPriority(TIMER1_IRQn, 3);
    NVIC_ClearPendingIRQ(TIMER1_IRQn);
    NVIC_EnableIRQ(TIMER1_IRQn);

    nrf_timer_mode_set(NRF_TIMER1, NRF_TIMER_MODE_TIMER);
    nrf_timer_bit_width_set(NRF_TIMER1, NRF_TIMER_BIT_WIDTH_8);

    nrf_timer_frequency_set(NRF_TIMER1, NRF_TIMER_FREQ_125kHz);
    nrf_timer_cc_write(NRF_TIMER1, NRF_TIMER_CC_CHANNEL0, 0);
    nrf_timer_int_enable(NRF_TIMER1, NRF_TIMER_INT_COMPARE0_MASK);

    nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_START);

    timerEnabled = true;
  }

  for (int i = 0; i < PWM_COUNT; i++) {
    if (pwmContext[i].pin == PIN_FREE || pwmContext[i].pin == ulPin) {
      pwmContext[i].pin = ulPin;

      nrf_gpio_cfg_output(ulPin);

      ulValue = mapResolution(ulValue, writeResolution, 8);

      pwmContext[i].value = ulValue;

      nrf_timer_cc_write(NRF_TIMER1, pwmContext[i].channel, ulValue);
      nrf_timer_int_enable(NRF_TIMER1, pwmContext[i].mask);
      break;
    }
  }
}

void TIMER1_IRQHandler(void)
{
  if (nrf_timer_event_check(NRF_TIMER1, NRF_TIMER_EVENT_COMPARE0)) {
    for (int i = 0; i < PWM_COUNT; i++) {
      if (pwmContext[i].pin != PIN_FREE && pwmContext[i].value != 0) {
        nrf_gpio_pin_write(pwmContext[i].pin, 1);
      }
    }

    nrf_timer_event_clear(NRF_TIMER1, NRF_TIMER_EVENT_COMPARE0);
  }

  for (int i = 0; i < PWM_COUNT; i++) {
    if (nrf_timer_event_check(NRF_TIMER1, pwmContext[i].event)) {
      if (pwmContext[i].pin != PIN_FREE && pwmContext[i].value != 255) {
        nrf_gpio_pin_write(pwmContext[i].pin, 0);
      }

      nrf_timer_event_clear(NRF_TIMER1, pwmContext[i].event);
    }
  }
}

#ifdef __cplusplus
}
#endif

#endif
