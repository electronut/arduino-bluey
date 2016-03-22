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

#include "nrf_saadc.h"
#include "nrf_pwm.h"

#include "Arduino.h"
#include "wiring_private.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_COUNT 3

static NRF_PWM_Type* pwms[PWM_COUNT] = {
  NRF_PWM0,
  NRF_PWM1,
  NRF_PWM2
};

static uint32_t pwmChannelPins[PWM_COUNT] = {
  NRF_PWM_PIN_NOT_CONNECTED,
  NRF_PWM_PIN_NOT_CONNECTED,
  NRF_PWM_PIN_NOT_CONNECTED
};

static uint16_t pwmChannelSequence[PWM_COUNT];

static int writeResolution = 8;

void analogReadResolution( int res )
{
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

uint32_t analogRead( uint32_t ulPin )
{
  return 0;
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite( uint32_t ulPin, uint32_t ulValue )
{
  for (int i = 0; i < PWM_COUNT; i++) {
    if (pwmChannelPins[i] == NRF_PWM_PIN_NOT_CONNECTED || pwmChannelPins[i] == ulPin) {
      pwmChannelPins[i] = ulPin;
      pwmChannelSequence[i] = ulValue;

      NRF_PWM_Type* pwm = pwms[i];

      nrf_pwm_pins_set(pwm, pwmChannelPins);
      nrf_pwm_enable(pwm);
      nrf_pwm_configure(pwm, NRF_PWM_CLK_16MHz, NRF_PWM_MODE_UP, (1 << writeResolution) - 1);
      nrf_pwm_loop_set(pwm, 0);
      nrf_pwm_decoder_set(pwm, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
      nrf_pwm_seq_ptr_set(pwm, 0, &pwmChannelSequence[i]);
      nrf_pwm_seq_cnt_set(pwm, 0, 1);
      nrf_pwm_seq_refresh_set(pwm, 0, 1);
      nrf_pwm_seq_end_delay_set(pwm, 0, 0);
      nrf_pwm_task_trigger(pwm, NRF_PWM_TASK_SEQSTART0);

      break;
    }
  }
}

#ifdef __cplusplus
}
#endif
