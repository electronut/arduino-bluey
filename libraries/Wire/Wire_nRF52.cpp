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

#ifdef NRF52

extern "C" {
#include <string.h>
}

#include "nrf_gpio.h"

#include <Arduino.h>
#include <wiring_private.h>

#include "Wire.h"

TwoWire::TwoWire(NRF_TWIM_Type * p_twim, NRF_TWIS_Type * p_twis, IRQn_Type IRQn, uint8_t pinSDA, uint8_t pinSCL)
{
  this->_p_twim = p_twim;
  this->_p_twis = p_twis;
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

  nrf_twim_frequency_set(_p_twim, NRF_TWIM_FREQ_100K);
  nrf_twim_enable(_p_twim);
  nrf_twim_pins_set(_p_twim, _uc_pinSCL, _uc_pinSDA);

  NVIC_ClearPendingIRQ(_IRQn);
  NVIC_SetPriority(_IRQn, 2);
  NVIC_EnableIRQ(_IRQn);
}

void TwoWire::begin(uint8_t address) {
  //Slave mode
  master = false;

  nrf_gpio_cfg(_uc_pinSCL, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg(_uc_pinSDA, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);

  nrf_twis_address_set(_p_twis, 0, address);
  nrf_twis_config_address_set(_p_twis, NRF_TWIS_CONFIG_ADDRESS0_MASK);
  nrf_twis_pins_set(_p_twis, _uc_pinSCL, _uc_pinSDA);

  nrf_twis_orc_set(_p_twis, 0xff);

  nrf_twis_int_enable(_p_twis, NRF_TWIS_INT_STOPPED_MASK | NRF_TWIS_INT_ERROR_MASK | NRF_TWIS_INT_WRITE_MASK | NRF_TWIS_INT_READ_MASK);

  NVIC_ClearPendingIRQ(_IRQn);
  NVIC_SetPriority(_IRQn, 2);
  NVIC_EnableIRQ(_IRQn);

  nrf_twis_enable(_p_twis);
}

void TwoWire::setClock(uint32_t baudrate) {
  if (master) {
    nrf_twim_disable(_p_twim);

    nrf_twim_frequency_t frequency;

    if (baudrate <= 100000)
    {
      frequency = NRF_TWIM_FREQ_100K;
    }
    else if (baudrate <= 250000)
    {
      frequency = NRF_TWIM_FREQ_250K;
    }
    else
    {
      frequency = NRF_TWIM_FREQ_400K;
    }

    nrf_twim_frequency_set(_p_twim, frequency);
    nrf_twim_enable(_p_twim);
  }
}

void TwoWire::end() {
  if (master)
  {
    nrf_twim_disable(_p_twim);
  }
  else
  {
    nrf_twis_disable(_p_twis);
  }
}

uint8_t TwoWire::requestFrom(uint8_t address, size_t quantity, bool stopBit)
{
  if(quantity == 0)
  {
    return 0;
  }

  size_t byteRead = 0;
  rxBuffer.clear();

  nrf_twim_address_set(_p_twim, address);

  nrf_twim_task_trigger(_p_twim, NRF_TWIM_TASK_RESUME);
  nrf_twim_rx_buffer_set(_p_twim, rxBuffer._aucBuffer, quantity);
  nrf_twim_task_trigger(_p_twim, NRF_TWIM_TASK_STARTRX);

  while(!nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_RXSTARTED) && !nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_ERROR));
  nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_RXSTARTED);

  while(!nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_LASTRX) && !nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_ERROR));
  nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_LASTRX);

  if (stopBit || nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_ERROR))
  {
    nrf_twim_task_trigger(_p_twim, NRF_TWIM_TASK_STOP);
    while(!nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_STOPPED));
    nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_STOPPED);
  }
  else
  {
    nrf_twim_task_trigger(_p_twim, NRF_TWIM_TASK_SUSPEND);
    while(!nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_SUSPENDED));
    nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_SUSPENDED);
  }

  if (nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_ERROR))
  {
    nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_ERROR);
  }

  byteRead = rxBuffer._iHead = nrf_twim_rxd_amount_get(_p_twim);

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
  nrf_twim_address_set(_p_twim, txAddress);

  nrf_twim_task_trigger(_p_twim, NRF_TWIM_TASK_RESUME);
  nrf_twim_task_trigger(_p_twim, NRF_TWIM_TASK_STARTTX);

  nrf_twim_tx_buffer_set(_p_twim, txBuffer._aucBuffer, txBuffer.available());

  nrf_twim_task_trigger(_p_twim, NRF_TWIM_TASK_STARTTX);

  while(!nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_TXSTARTED) && !nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_ERROR));
  nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_TXSTARTED);

  while(!nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_LASTTX) && !nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_ERROR));
  nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_LASTTX);

  if (stopBit || nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_ERROR))
  {
    nrf_twim_task_trigger(_p_twim, NRF_TWIM_TASK_STOP);
    while(!nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_STOPPED));
    nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_STOPPED);
  }
  else
  {
    nrf_twim_task_trigger(_p_twim, NRF_TWIM_TASK_SUSPEND);
    while(!nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_SUSPENDED));
    nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_SUSPENDED);
  }

  if (nrf_twim_event_check(_p_twim, NRF_TWIM_EVENT_ERROR))
  {
    nrf_twim_event_clear(_p_twim, NRF_TWIM_EVENT_ERROR);

    uint32_t error = nrf_twim_errorsrc_get_and_clear(_p_twim);

    if (error == NRF_TWIM_ERROR_ADDRESS_NACK)
    {
      return 2;
    }
    else if (error == NRF_TWIM_ERROR_DATA_NACK)
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

void TwoWire::onReceive(void(*function)(int))
{
  onReceiveCallback = function;
}

void TwoWire::onRequest(void(*function)(void))
{
  onRequestCallback = function;
}

void TwoWire::onService(void)
{
  if (nrf_twis_event_get_and_clear(_p_twis, NRF_TWIS_EVENT_WRITE))
  {
    receiving = true;

    nrf_twis_rx_prepare(_p_twis, rxBuffer._aucBuffer, sizeof(rxBuffer._aucBuffer));
  }

  if (nrf_twis_event_get_and_clear(_p_twis, NRF_TWIS_EVENT_READ))
  {
    receiving = false;
    transmissionBegun = true;

    txBuffer.clear();

    if (onRequestCallback)
    {
      onRequestCallback();
    }

    transmissionBegun = false;

    nrf_twis_tx_prepare(_p_twis, txBuffer._aucBuffer, txBuffer.available());
  }

  if (nrf_twis_event_get_and_clear(_p_twis, NRF_TWIS_EVENT_STOPPED))
  {
    if (receiving)
    {
      int rxAmount = nrf_twis_rx_amount_get(_p_twis);

      rxBuffer._iHead = rxAmount;

      if (onReceiveCallback)
      {
        onReceiveCallback(rxAmount);
      }
    }
  }

  if (nrf_twis_event_get_and_clear(_p_twis, NRF_TWIS_EVENT_ERROR))
  {
    nrf_twis_error_source_get_and_clear(_p_twis);
    nrf_twis_task_trigger(_p_twis, NRF_TWIS_TASK_STOP);
  }
}

TwoWire Wire(NRF_TWIM0, NRF_TWIS0, SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn, PIN_WIRE_SDA, PIN_WIRE_SCL);

extern "C"
{
  void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(void)
  {
    Wire.onService();
  }
}

#endif
