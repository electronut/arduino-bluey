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

#include "nrf_rtc.h"

#include "delay.h"
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

static volatile uint32_t overflows = 0;

uint32_t millis( void )
{
  uint64_t ticks = (uint64_t)((uint64_t)overflows << (uint64_t)32) | (uint64_t)nrf_rtc_counter_get(NRF_RTC1);

  return (ticks * 1000) / RTC_INPUT_FREQ;
}

uint32_t micros( void )
{
  uint64_t ticks = (uint64_t)((uint64_t)overflows << (uint64_t)32) | (uint64_t)nrf_rtc_counter_get(NRF_RTC1);

  return (ticks * 1000000) / RTC_INPUT_FREQ;
}

void delay( uint32_t ms )
{
  if ( ms == 0 )
  {
    return ;
  }

  uint32_t start = millis() ;

  do
  {
    yield() ;
  } while ( millis() - start < ms ) ;
}

void RTC1_IRQHandler(void)
{
  nrf_rtc_event_clear(NRF_RTC1, NRF_RTC_EVENT_OVERFLOW);

  overflows = (overflows + 1) & 0xff;
}

#ifdef __cplusplus
}
#endif
