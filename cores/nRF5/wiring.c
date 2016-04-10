/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
  Copyright (c) 2016 Sandeep Mistry All right reserved.

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

#include "nrf_clock.h"
#include "nrf_rtc.h"

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

void init( void )
{
  NVIC_SetPriority(RTC1_IRQn, 15);
  NVIC_ClearPendingIRQ(RTC1_IRQn);
  NVIC_EnableIRQ(RTC1_IRQn);

  nrf_clock_lf_src_set(CLOCK_LFCLKSRC_SRC_RC);
  nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);

  nrf_rtc_prescaler_set(NRF_RTC1, 0);
  nrf_rtc_event_enable(NRF_RTC1, NRF_RTC_EVENT_OVERFLOW);
  nrf_rtc_int_enable(NRF_RTC1, NRF_RTC_EVENT_OVERFLOW);
  nrf_rtc_task_trigger(NRF_RTC1, NRF_RTC_TASK_START);
}

#ifdef __cplusplus
}
#endif