/*
 * SPI Master library for nRF5x.
 * Copyright (c) 2015 Arduino LLC
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

#include "SPI.h"
#include <Arduino.h>
#include <wiring_private.h>
#include <assert.h>

#define SPI_IMODE_NONE   0
#define SPI_IMODE_EXTINT 1
#define SPI_IMODE_GLOBAL 2

const SPISettings DEFAULT_SPI_SETTINGS = SPISettings();

SPIClass::SPIClass(NRF_SPI_Type *p_spi, uint8_t uc_pinMISO, uint8_t uc_pinSCK, uint8_t uc_pinMOSI)
{
  initialized = false;
  assert(p_spi != NULL);
  _p_spi = p_spi;

  // pins
  _uc_pinMiso = g_ADigitalPinMap[uc_pinMISO];
  _uc_pinSCK = g_ADigitalPinMap[uc_pinSCK];
  _uc_pinMosi = g_ADigitalPinMap[uc_pinMOSI];

  _dataMode = NRF_SPI_MODE_0;
  _bitOrder = NRF_SPI_BIT_ORDER_MSB_FIRST;
}

void SPIClass::begin()
{
  init();

  nrf_spi_pins_set(_p_spi, _uc_pinSCK, _uc_pinMosi, _uc_pinMiso);

  config(DEFAULT_SPI_SETTINGS);
}

void SPIClass::init()
{
  if (initialized)
    return;
  interruptMode = SPI_IMODE_NONE;
  interruptSave = 0;
  interruptMask = 0;
  initialized = true;
}

void SPIClass::config(SPISettings settings)
{
  nrf_spi_disable(_p_spi);

  nrf_spi_configure(_p_spi, settings.dataMode, settings.bitOrder);
  nrf_spi_frequency_set(_p_spi, settings.clockFreq);

  nrf_spi_enable(_p_spi);
}

void SPIClass::end()
{
  nrf_spi_disable(_p_spi);
  initialized = false;
}

void SPIClass::usingInterrupt(int /*interruptNumber*/)
{
}

void SPIClass::beginTransaction(SPISettings settings)
{
  config(settings);
}

void SPIClass::endTransaction(void)
{
}

void SPIClass::setBitOrder(BitOrder order)
{
  if (order == LSBFIRST) {
    nrf_spi_configure(_p_spi, _dataMode, NRF_SPI_BIT_ORDER_LSB_FIRST);
  } else {
    nrf_spi_configure(_p_spi, _dataMode, NRF_SPI_BIT_ORDER_MSB_FIRST);
  }
}

void SPIClass::setDataMode(uint8_t mode)
{
  switch (mode)
  {
    case SPI_MODE0:
      nrf_spi_configure(_p_spi, NRF_SPI_MODE_0, _bitOrder);
      break;

    case SPI_MODE1:
      nrf_spi_configure(_p_spi, NRF_SPI_MODE_1, _bitOrder);
      break;

    case SPI_MODE2:
      nrf_spi_configure(_p_spi, NRF_SPI_MODE_2, _bitOrder);
      break;

    case SPI_MODE3:
      nrf_spi_configure(_p_spi, NRF_SPI_MODE_3, _bitOrder);
      break;

    default:
      break;
  }
}

void SPIClass::setClockDivider(uint8_t div)
{
  nrf_spi_frequency_t clockFreq;

  if (div >= SPI_CLOCK_DIV128) {
    clockFreq = NRF_SPI_FREQ_125K;
  } else if (div >= SPI_CLOCK_DIV64) {
    clockFreq = NRF_SPI_FREQ_250K;
  } else if (div >= SPI_CLOCK_DIV32) {
    clockFreq = NRF_SPI_FREQ_500K;
  } else if (div >= SPI_CLOCK_DIV16) {
    clockFreq = (nrf_spi_frequency_t)SPI_FREQUENCY_FREQUENCY_M1;
  } else if (div >= SPI_CLOCK_DIV8) {
    clockFreq = (nrf_spi_frequency_t)SPI_FREQUENCY_FREQUENCY_M2;
  } else if (div >= SPI_CLOCK_DIV4) {
    clockFreq = (nrf_spi_frequency_t)SPI_FREQUENCY_FREQUENCY_M4;
  } else {
    clockFreq = (nrf_spi_frequency_t)SPI_FREQUENCY_FREQUENCY_M8;
  }

  nrf_spi_frequency_set(_p_spi, clockFreq);
}

byte SPIClass::transfer(uint8_t data)
{
  nrf_spi_txd_set(_p_spi, data);

  while(!nrf_spi_event_check(_p_spi, NRF_SPI_EVENT_READY));

  data = nrf_spi_rxd_get(_p_spi);

  nrf_spi_event_clear(_p_spi, NRF_SPI_EVENT_READY);

  return data;
}

uint16_t SPIClass::transfer16(uint16_t data) {
  union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } t;

  t.val = data;

  if (_bitOrder == NRF_SPI_BIT_ORDER_LSB_FIRST) {
    t.lsb = transfer(t.lsb);
    t.msb = transfer(t.msb);
  } else {
    t.msb = transfer(t.msb);
    t.lsb = transfer(t.lsb);
  }

  return t.val;
}

void SPIClass::attachInterrupt() {
  // Should be enableInterrupt()
}

void SPIClass::detachInterrupt() {
  // Should be disableInterrupt()
}

SPIClass SPI (NRF_SPI0,  PIN_SPI_MISO,  PIN_SPI_SCK,  PIN_SPI_MOSI);
