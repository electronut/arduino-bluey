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

#include "nrf_gpio.h"

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

void pinMode( uint32_t ulPin, uint32_t ulMode )
{
  // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
  switch ( ulMode )
  {
    case INPUT:
      // Set pin to input mode
      nrf_gpio_cfg_input(ulPin, NRF_GPIO_PIN_NOPULL);
    break ;

    case INPUT_PULLUP:
      // Set pin to input mode with pull-up resistor enabled
      nrf_gpio_cfg_input(ulPin, NRF_GPIO_PIN_PULLUP);
    break ;

    case INPUT_PULLDOWN:
      // Set pin to input mode with pull-down resistor enabled
      nrf_gpio_cfg_input(ulPin, NRF_GPIO_PIN_PULLDOWN);
    break ;

    case OUTPUT:
      // Set pin to output mode
      nrf_gpio_cfg_output(ulPin);
    break ;

    default:
      // do nothing
    break ;
  }
}

void digitalWrite( uint32_t ulPin, uint32_t ulVal )
{
  switch ( ulVal )
  {
    case LOW:
      nrf_gpio_pin_write(ulPin, 0);
    break ;

    default:
      nrf_gpio_pin_write(ulPin, 1);
    break ;
  }

  return ;
}

int digitalRead( uint32_t ulPin )
{
  return nrf_gpio_pin_read(ulPin) ? HIGH : LOW ;
}

#ifdef __cplusplus
}
#endif
