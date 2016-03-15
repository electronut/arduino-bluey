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

#include "Uart.h"
#include "Arduino.h"
#include "wiring_private.h"

Uart::Uart(NRF_UART_Type *_nrfUart, IRQn_Type _IRQn, uint8_t _pinRX, uint8_t _pinTX)
{
  nrfUart = _nrfUart;
  IRQn = _IRQn;
  uc_pinRX = _pinRX;
  uc_pinTX = _pinTX;
}

void Uart::begin(unsigned long baudrate)
{
  begin(baudrate, (uint8_t)SERIAL_8N1);
}

void Uart::begin(unsigned long baudrate, uint16_t /*config*/)
{
  pinMode(uc_pinTX, OUTPUT);
  pinMode(uc_pinRX, INPUT);

  nrf_uart_txrx_pins_set(nrfUart, uc_pinTX, uc_pinRX);
  nrf_uart_configure(nrfUart, NRF_UART_PARITY_EXCLUDED, NRF_UART_HWFC_DISABLED);

  nrf_uart_baudrate_t nrfBaudRate;

  if (baudrate <= 1200) {
    nrfBaudRate = NRF_UART_BAUDRATE_1200;
  } else if (baudrate <= 2400) {
    nrfBaudRate = NRF_UART_BAUDRATE_2400;
  } else if (baudrate <= 4800) {
    nrfBaudRate = NRF_UART_BAUDRATE_4800;
  } else if (baudrate <= 9600) {
    nrfBaudRate = NRF_UART_BAUDRATE_9600;
  } else if (baudrate <= 14400) {
    nrfBaudRate = NRF_UART_BAUDRATE_14400;
  } else if (baudrate <= 19200) {
    nrfBaudRate = NRF_UART_BAUDRATE_19200;
  } else if (baudrate <= 28800) {
    nrfBaudRate = NRF_UART_BAUDRATE_28800;
  } else if (baudrate <= 38400) {
    nrfBaudRate = NRF_UART_BAUDRATE_38400;
  } else if (baudrate <= 57600) {
    nrfBaudRate = NRF_UART_BAUDRATE_57600;
  } else if (baudrate <= 76800) {
    nrfBaudRate = NRF_UART_BAUDRATE_76800;
  } else if (baudrate <= 115200) {
    nrfBaudRate = NRF_UART_BAUDRATE_115200;
  } else if (baudrate <= 230400) {
    nrfBaudRate = NRF_UART_BAUDRATE_230400;
  } else if (baudrate <= 250000) {
    nrfBaudRate = NRF_UART_BAUDRATE_250000;
  } else if (baudrate <= 460800) {
    nrfBaudRate = NRF_UART_BAUDRATE_460800;
  } else if (baudrate <= 921600) {
    nrfBaudRate = NRF_UART_BAUDRATE_921600;
  } else {
    nrfBaudRate = NRF_UART_BAUDRATE_1000000;
  }

  nrf_uart_baudrate_set(nrfUart, nrfBaudRate);

  nrf_uart_enable(nrfUart);

  nrf_uart_event_clear(nrfUart, NRF_UART_EVENT_RXDRDY);
  nrf_uart_event_clear(nrfUart, NRF_UART_EVENT_TXDRDY);

  nrf_uart_task_trigger(nrfUart, NRF_UART_TASK_STARTRX);
  nrf_uart_task_trigger(nrfUart, NRF_UART_TASK_STARTTX);

  nrf_uart_int_enable(nrfUart, NRF_UART_INT_MASK_RXDRDY);

  NVIC_ClearPendingIRQ(IRQn);
  NVIC_SetPriority(IRQn, 3);
  NVIC_EnableIRQ(IRQn);
}

void Uart::end()
{
  NVIC_DisableIRQ(IRQn);

  nrf_uart_int_disable(nrfUart, NRF_UART_INT_MASK_RXDRDY);

  nrf_uart_task_trigger(nrfUart, NRF_UART_TASK_STOPRX);
  nrf_uart_task_trigger(nrfUart, NRF_UART_TASK_STOPTX);

  nrf_uart_disable(nrfUart);

  nrf_uart_txrx_pins_disconnect(nrfUart);

  rxBuffer.clear();
}

void Uart::flush()
{
}

void Uart::IrqHandler()
{
  if (nrf_uart_event_check(nrfUart, NRF_UART_EVENT_RXDRDY))
  {
    rxBuffer.store_char(nrf_uart_rxd_get(nrfUart));

    nrf_uart_event_clear(nrfUart, NRF_UART_EVENT_RXDRDY);
  }
}

int Uart::available()
{
  return rxBuffer.available();
}

int Uart::peek()
{
  return rxBuffer.peek();
}

int Uart::read()
{
  return rxBuffer.read_char();
}

size_t Uart::write(const uint8_t data)
{
  nrf_uart_txd_set(nrfUart, data);

  while(!nrf_uart_event_check(nrfUart, NRF_UART_EVENT_TXDRDY));

  nrf_uart_event_clear(nrfUart, NRF_UART_EVENT_TXDRDY);

  return 1;
}
