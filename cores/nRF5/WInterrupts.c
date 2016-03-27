/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.

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

#include <nrf_gpiote.h>

#include "Arduino.h"
#include "wiring_private.h"

#include <string.h>

static voidFuncPtr callbacksInt[NUMBER_OF_GPIO_TE];
static int8_t channelMap[NUMBER_OF_GPIO_TE];
static int enabled = 0;

/* Configure I/O interrupt sources */
static void __initialize()
{
  memset(callbacksInt, 0, sizeof(callbacksInt));
  memset(channelMap, -1, sizeof(channelMap));

  NVIC_DisableIRQ(GPIOTE_IRQn);
  NVIC_ClearPendingIRQ(GPIOTE_IRQn);
  NVIC_SetPriority(GPIOTE_IRQn, 1);
  NVIC_EnableIRQ(GPIOTE_IRQn);
}

/*
 * \brief Specifies a named Interrupt Service Routine (ISR) to call when an interrupt occurs.
 *        Replaces any previous function that was attached to the interrupt.
 */
void attachInterrupt(uint32_t pin, voidFuncPtr callback, uint32_t mode)
{
  if (!enabled) {
    __initialize();
    enabled = 1;
  }

  nrf_gpiote_polarity_t polarity;

  switch (mode) {
    case CHANGE:
      polarity = NRF_GPIOTE_POLARITY_TOGGLE;
      break;

    case FALLING:
      polarity = NRF_GPIOTE_POLARITY_HITOLO;
      break;

    case RISING:
      polarity = NRF_GPIOTE_POLARITY_LOTOHI;
      break;

    default:
      return;
  }

  for (int ch = 0; ch < NUMBER_OF_GPIO_TE; ch++) {
    if (channelMap[ch] == -1 || (uint32_t)channelMap[ch] == pin) {
      channelMap[ch] = pin;
      callbacksInt[ch] = callback;

      nrf_gpiote_event_configure(ch, pin, polarity);
      nrf_gpiote_event_enable(ch);
      nrf_gpiote_int_enable(1 << ch);

      break;
    }
  }
}

/*
 * \brief Turns off the given interrupt.
 */
void detachInterrupt(uint32_t pin)
{
  for (int ch = 0; ch < NUMBER_OF_GPIO_TE; ch++) {
    if ((uint32_t)channelMap[ch] == pin) {
      channelMap[ch] = -1;
      callbacksInt[ch] = NULL;

      nrf_gpiote_event_disable(ch);
      nrf_gpiote_int_disable(1 << ch);

      break;
    }
  }
}

void GPIOTE_IRQHandler()
{
  nrf_gpiote_events_t event = NRF_GPIOTE_EVENTS_IN_0;

  for (int ch = 0; ch < NUMBER_OF_GPIO_TE; ch++) {
    if (nrf_gpiote_event_is_set(event) && nrf_gpiote_int_is_enabled(1 << ch)) {
      if (channelMap[ch] != -1 && callbacksInt[ch]) {
        callbacksInt[ch]();
      }

      nrf_gpiote_event_clear(event);
    }

    event = (nrf_gpiote_events_t)((uint32_t)event + 4);
  }
}
