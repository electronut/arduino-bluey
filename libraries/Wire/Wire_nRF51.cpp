/*
 * TWI/I2C library for nRF5x
 * Copyright (c) 2015 Arduino LLC. All rights reserved.
 * Copyright (c) 2016 Sandeep Mistry All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef NRF51

extern "C" {
#include <string.h>
}

#include "nrf_gpio.h"

#include <Arduino.h>
#include <wiring_private.h>

#include "Wire.h"

TwoWire::TwoWire(NRF_TWI_Type * p_twi, IRQn_Type IRQn, uint8_t pinSDA, uint8_t pinSCL)
{
  this->_p_twi = p_twi;
  this->_IRQn = IRQn;
  this->_uc_pinSDA = g_ADigitalPinMap[pinSDA];
  this->_uc_pinSCL = g_ADigitalPinMap[pinSCL];
  transmissionBegun = false;
}

void TwoWire::begin(void) {
  //Master Mode
  master = true;

  nrf_gpio_cfg(_uc_pinSCL, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg(_uc_pinSDA, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);

  nrf_twi_frequency_set(_p_twi, NRF_TWI_FREQ_100K);
  nrf_twi_enable(_p_twi);
  nrf_twi_pins_set(_p_twi, _uc_pinSCL, _uc_pinSDA);

  NVIC_ClearPendingIRQ(_IRQn);
  NVIC_SetPriority(_IRQn, 2);
  NVIC_EnableIRQ(_IRQn);
}

void TwoWire::setClock(uint32_t baudrate) {
  nrf_twi_disable(_p_twi);

  nrf_twi_frequency_t frequency;

  if (baudrate <= 100000)
  {
    frequency = NRF_TWI_FREQ_100K;
  }
  else if (baudrate <= 250000)
  {
    frequency = NRF_TWI_FREQ_250K;
  }
  else
  {
    frequency = NRF_TWI_FREQ_400K;
  }

  nrf_twi_frequency_set(_p_twi, frequency);
  nrf_twi_enable(_p_twi);
}

void TwoWire::end() {
  nrf_twi_disable(_p_twi);
}

uint8_t TwoWire::requestFrom(uint8_t address, size_t quantity, bool stopBit)
{
  if(quantity == 0)
  {
    return 0;
  }

  size_t byteRead = 0;
  rxBuffer.clear();

  nrf_twi_address_set(_p_twi, address);

  nrf_twi_task_trigger(_p_twi, NRF_TWI_TASK_RESUME);
  nrf_twi_task_trigger(_p_twi, NRF_TWI_TASK_STARTRX);

  for (size_t i = 0; i < quantity; i++)
  {
    while(!nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_RXDREADY) && !nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_ERROR));

    if (nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_ERROR))
    {
      break;
    }

    nrf_twi_event_clear(_p_twi, NRF_TWI_EVENT_RXDREADY);

    rxBuffer.store_char(nrf_twi_rxd_get(_p_twi));

    nrf_twi_task_trigger(_p_twi, NRF_TWI_TASK_RESUME);
  }

  if (stopBit || nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_ERROR))
  {
    nrf_twi_task_trigger(_p_twi, NRF_TWI_TASK_STOP);
    while(!nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_STOPPED));
    nrf_twi_event_clear(_p_twi, NRF_TWI_EVENT_STOPPED);
  }
  else
  {
    nrf_twi_task_trigger(_p_twi, NRF_TWI_TASK_SUSPEND);
    while(!nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_SUSPENDED));
    nrf_twi_event_clear(_p_twi, NRF_TWI_EVENT_SUSPENDED);
  }

  if (nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_ERROR))
  {
    nrf_twi_event_clear(_p_twi, NRF_TWI_EVENT_ERROR);
  }

  return byteRead;
}

uint8_t TwoWire::requestFrom(uint8_t address, size_t quantity)
{
  return requestFrom(address, quantity, true);
}

void TwoWire::beginTransmission(uint8_t address) {
  // save address of target and clear buffer
  txAddress = address;
  txBuffer.clear();

  transmissionBegun = true;
}

// Errors:
//  0 : Success
//  1 : Data too long
//  2 : NACK on transmit of address
//  3 : NACK on transmit of data
//  4 : Other error
uint8_t TwoWire::endTransmission(bool stopBit)
{
  transmissionBegun = false ;

  // Start I2C transmission
  nrf_twi_address_set(_p_twi, txAddress);

  nrf_twi_task_trigger(_p_twi, NRF_TWI_TASK_RESUME);
  nrf_twi_task_trigger(_p_twi, NRF_TWI_TASK_STARTTX);


  while (txBuffer.available())
  {
    nrf_twi_txd_set(_p_twi, txBuffer.read_char());

    while(!nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_TXDSENT) && !nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_ERROR));

    if (nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_ERROR))
    {
      break;
    }

    nrf_twi_event_clear(_p_twi, NRF_TWI_EVENT_TXDSENT);
  }

  if (stopBit || nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_ERROR))
  {
    nrf_twi_task_trigger(_p_twi, NRF_TWI_TASK_STOP);
    while(!nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_STOPPED));
    nrf_twi_event_clear(_p_twi, NRF_TWI_EVENT_STOPPED);
  }
  else
  {
    nrf_twi_task_trigger(_p_twi, NRF_TWI_TASK_SUSPEND);
    while(!nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_SUSPENDED));
    nrf_twi_event_clear(_p_twi, NRF_TWI_EVENT_SUSPENDED);
  }

  if (nrf_twi_event_check(_p_twi, NRF_TWI_EVENT_ERROR))
  {
    nrf_twi_event_clear(_p_twi, NRF_TWI_EVENT_ERROR);

    uint32_t error = nrf_twi_errorsrc_get_and_clear(_p_twi);

    if (error == NRF_TWI_ERROR_ADDRESS_NACK)
    {
      return 2;
    }
    else if (error == NRF_TWI_ERROR_DATA_NACK)
    {
      return 3;
    }
    else
    {
      return 4;
    }
  }

  return 0;
}

uint8_t TwoWire::endTransmission()
{
  return endTransmission(true);
}

size_t TwoWire::write(uint8_t ucData)
{
  // No writing, without begun transmission or a full buffer
  if ( !transmissionBegun || txBuffer.isFull() )
  {
    return 0 ;
  }

  txBuffer.store_char( ucData ) ;

  return 1 ;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
  //Try to store all data
  for(size_t i = 0; i < quantity; ++i)
  {
    //Return the number of data stored, when the buffer is full (if write return 0)
    if(!write(data[i]))
      return i;
  }

  //All data stored
  return quantity;
}

int TwoWire::available(void)
{
  return rxBuffer.available();
}

int TwoWire::read(void)
{
  return rxBuffer.read_char();
}

int TwoWire::peek(void)
{
  return rxBuffer.peek();
}

void TwoWire::flush(void)
{
  // Do nothing, use endTransmission(..) to force
  // data transfer.
}

void TwoWire::onService(void)
{
}

TwoWire Wire(NRF_TWI0, SPI0_TWI0_IRQn, PIN_WIRE_SDA, PIN_WIRE_SCL);

extern "C"
{
  void SPI0_TWI0_IRQHandler(void)
  {
    Wire.onService();
  }
}

#endif
