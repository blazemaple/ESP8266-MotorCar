/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_app_beacon_main main.c
 * @{
 * @ingroup ble_sdk_app_beacon
 * @brief Beacon Transmitter Sample Application main file.
 *
 * This file contains the source code for an Beacon transmitter sample application.
 */

#include "app_timer.h"
#include "app_uart.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_pwr_mgmt.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined(UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "dong_ble_adv.h"
#include "dong_deca_ctrl.h"
#include "dong_printer.h"

// #include "my_Deca_API.h"
// #include "my_Deca_regs.h"

#define AID 0x01

/* Frames used in the ranging process. See NOTE 3 below. */
static uint8 tx_poll_msg[] = {0x00, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
static uint8 rx_resp_msg[] = {0x01, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* Frame sequence number, incremented after each transmission. */
static uint8 frame_seq_nb = 0;

/* Buffer to store received response message.
 * Its size is adjusted to longest frame that this example code is supposed to handle. */
#define RX_BUF_LEN 20
static uint8 rx_buffer[RX_BUF_LEN];
static uint8 rx_buffer2[RX_BUF_LEN];

/* Hold copy of status register state here for reference so that it can be examined at a debug
 * breakpoint. */
static uint32 status_reg = 0;

/* Speed of light in air, in metres per second. */
#define SPEED_OF_LIGHT 299702547

/* Hold copies of computed time of flight and distance here for reference so that it can be examined
 * at a debug breakpoint. */
static double tof;
static double distance;

/* String used to display measured distance on LCD screen (16 characters maximum). */
char dist_str[16] = {0};

// basic functions for nrf
static void timers_init(void);
static void power_management_init(void);
static void idle_state_handle(void);

/****************************************************************************/
/**
 *                              UART
 *
 *******************************************************************************/
#define UART_TX_BUF_SIZE 256 /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256 /**< UART RX buffer size. */
enum uartMASK {
  uart_xfer = 0x01,
  Report_xfer = 0x10
};
static uint8_t uart_mutex = 0;
uint8_t rx_time[11] = {0};

/**@brief Function for print to uart port
 * pin number:
 *            tx -> 6
 *            rx -> 8
 */
uint32_t uart_print(uint8_t *data, uint16_t len) {
  uint32_t err_code;
  for (uint32_t i = 0; i < len; i++) {
    do {
      err_code = app_uart_put(data[i]);
      if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY)) {
        msg_print("UART failed. Error 0x%x. ", err_code);
        APP_ERROR_CHECK(err_code);
      }
    } while (err_code == NRF_ERROR_BUSY);
  }
  if (data[len - 1] == '\r') {
    while (app_uart_put('\n') == NRF_ERROR_BUSY)
      ;
  }
  return err_code;
}

void getRemainingSubstring(const char *str, int startIndex, char *result) {
  int length = 0;

  // 獲取字串長度
  while (str[startIndex + length] != '\0') {
    length++;
  }

  // 複製剩餘部分到結果陣列
  int i;
  for (i = 0; i < length; i++) {
    result[i] = str[startIndex + i];
  }

  result[length] = '\r'; // 加上結束符
}

void ATcmd_judge(uint8_t *data) {
  char substring[10];
  char buf[10];
  const char *delimiter = ",";
  if (memcmp(data, "AT+setLpwm=", 11) == 0) {
    getRemainingSubstring(data, 11, substring);
    tx_poll_msg[1] = 1; // LPWM type
    tx_poll_msg[2] = atoi(substring);
    uart_print(substring, 4);
  } else if (memcmp(data, "AT+setRpwm=", 11) == 0) {
    getRemainingSubstring(data, 11, substring);
    tx_poll_msg[1] = 2; // RPWM type
    tx_poll_msg[2] = atoi(substring);
    uart_print(substring, 4);
  } else if (memcmp(data, "AT+setLRpwm=", 12) == 0) {
    getRemainingSubstring(data, 12, substring);
    char *token = strtok(substring, delimiter);
    int strcnt = 2;
    tx_poll_msg[1] = 3; // LRPWM type
    while (token != NULL) {
      tx_poll_msg[strcnt] = atoi(token);
      strcnt++;
      sprintf(buf, "token %03d\n", atoi(token));
      uart_print(buf, 10);
      token = strtok(NULL, delimiter);
    }

  } else if (memcmp(data, "AT\n", 2) == 0) {
    uart_print("ok\n", 3);
  }
  dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);
  int ret = dwt_starttx(DWT_START_TX_IMMEDIATE);
  if (ret == DWT_SUCCESS) {
    /* Poll DW1000 until TX frame sent event set. See NOTE 6 below. */
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {
    };

    /* Clear TXFRS event. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

    uart_print("uwb send ok\n", 12);
  }
}

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t *p_event) {
  static uint8_t data_array[25];
  static uint8_t index = 0;
  uint32_t err_code;

  switch (p_event->evt_type) {
  case APP_UART_DATA_READY:
    uart_mutex |= uart_xfer;
    UNUSED_VARIABLE(app_uart_get(&data_array[index]));
    index++;

    if ((data_array[index - 1] == '\n')) //|| (data_array[index - 1] == '\r'))
    {
      msg_print("%s\n", data_array);
      //uart_print(data_array, index);
      if (memcmp(data_array, "AT", 2) == 0) {
        ATcmd_judge(data_array);
      } else if (memcmp(data_array, "ok", 2) == 0) {
        rx_resp_msg[0] = 1;
      } else {
        char substring[10];
        char buf[10];
        const char *delimiter = ",";
        rx_resp_msg[0] = 2;
        getRemainingSubstring(data_array, 0, substring);
        char *token = strtok(substring, delimiter);
        int strcnt = 1;
        while (token != NULL) {
          rx_resp_msg[strcnt] = atoi(token);
          strcnt++;
          token = strtok(NULL, delimiter);
        }
        msg_print("%d,%d\n", rx_resp_msg[1], rx_resp_msg[2]);
        dwt_writetxdata(sizeof(rx_resp_msg), rx_resp_msg, 0); /* Zero offset in TX buffer. */
        dwt_writetxfctrl(sizeof(rx_resp_msg), 0, 1);
        int ret = dwt_starttx(DWT_START_TX_IMMEDIATE);
        if (ret == DWT_SUCCESS) {
          /* Poll DW1000 until TX frame sent event set. See NOTE 6 below. */
          while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {
          };
          /* Clear TXFRS event. */
          dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
        }
      }
      memset(data_array, 0x00, index);
      index = 0;
    }
    uart_mutex &= (~uart_xfer);

    break;

  case APP_UART_COMMUNICATION_ERROR:
    // APP_ERROR_HANDLER(p_event->data.error_communication);
    break;

  case APP_UART_FIFO_ERROR:
    APP_ERROR_HANDLER(p_event->data.error_code);
    break;

  default:
    break;
  }
}

/**@snippet [Handling the data received over UART] */

/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void) {
  uint32_t err_code;
  app_uart_comm_params_t const comm_params =
  {.rx_pin_no = RX_PIN_NUMBER,
    .tx_pin_no = TX_PIN_NUMBER,
    .rts_pin_no = RTS_PIN_NUMBER,
    .cts_pin_no = CTS_PIN_NUMBER,
    .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
    .use_parity = false,
#if defined(UART_PRESENT)
    .baud_rate = NRF_UART_BAUDRATE_115200
#else
    .baud_rate = NRF_UARTE_BAUDRATE_115200
#endif
  };

  APP_UART_FIFO_INIT(&comm_params, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, uart_event_handle,
      APP_IRQ_PRIORITY_LOWEST, err_code);
  APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for application main entry.
 */
int main(void) {
  // Initialize.
  uart_init();
  log_init();
  // timers_init();
  // power_management_init();
  // leds_init();
  // ble_stack_init();
  // advertising_init();

  // Start execution.
  msg_print("\tStart execution.\n");
  // advertising_start();

  APP_ERROR_CHECK(DW1000_init());
  // NRF_LOG_FLUSH();

  // pinMode(2, 0);
  // pinMode(3, 0);
  // digitalWrite(2, 1);
  // digitalWrite(3, 1);
  char ATcmd[25];
  // Enter main loop.
  for (;;) {
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    /* Poll for reception of a frame or error/timeout. See NOTE 6 below. */
    do {
      status_reg = dwt_read32bitreg(SYS_STATUS_ID);
    } while (!(status_reg & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)));

    if (status_reg & SYS_STATUS_RXFCG) {
      /* Clear good RX frame event in the DW1000 status register. */
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);

      /* A frame has been received, read it into the local buffer. */
      uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
      if (frame_len <= RX_BUF_LEN) {
        dwt_readrxdata(rx_buffer, frame_len, 0);
        msg_print("get data\n");
      }
      msg_print("LRPM: %d\n", rx_buffer[2]);
      msg_print("RRPM: %d\n", rx_buffer[3]);
      if (rx_buffer[1] == 3) {
        sprintf(ATcmd, "AT+setLRpwm=%d,%d", rx_buffer[2], rx_buffer[3]);
        uart_print(ATcmd, strlen(ATcmd));
        msg_print("%s\n", ATcmd);
      }
    } else {
      /* Clear RX error events in the DW1000 status register. */
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);

      /* Reset RX to properly reinitialise LDE operation. */
      dwt_rxreset();
    }
  }
}
/**
 * @}
 */

/**@brief Function for initializing timers. */
static void timers_init(void) {
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void) {
  ret_code_t err_code;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void) {
  if (NRF_LOG_PROCESS() == false) {
    nrf_pwr_mgmt_run();
  }
}