/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
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
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_gpio.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "boards.h"

#define SPI_INSTANCE        0 /**< SPI instance index. */
#define SPI_SS_PIN_INVERTED 28
#define SPI_SCK_PIN 29
#define SPI_MISO_PIN 31
#define SPI_MOSI_PIN 30

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

static uint8_t       m_tx_buf[2]={0x80,0x04};    /**< TX buffer. */
static uint8_t       m_rx_buf[3]={0x00,0x00,0x00};    /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf);   /**< Transfer length. */

void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context);
void init_spi(void);

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
    nrf_gpio_pin_clear(SPI_SS_PIN_INVERTED);
    printf("Transfer completed : %d\n", (int)(m_tx_buf[0]),(int)(m_tx_buf[1]));
    if((m_tx_buf[0] == 0x80))
    {
        m_tx_buf[0]=0x00;
        m_tx_buf[1]=0x00;
    }
    if((m_tx_buf[0] == 0x00) && (m_rx_buf[1]==0x04))
    {
        printf("Temperature sensor on !");
        m_tx_buf[0]=0x02;
    }
    if ((m_rx_buf[0] != 0) || (m_rx_buf[1] != 0))
    {
        if(m_tx_buf[0]=0x02)
        {
          printf("Temp: %d,%d\n", (int)(m_rx_buf[1]), (int)(m_rx_buf[2]));
        }
        else
        {
                printf("Received: %d,%d,%d\n", (int)(m_rx_buf[0]), (int)(m_rx_buf[1]), (int)(m_rx_buf[2]));
        }
    }
}
void init_spi(void)
{
    /*init MISO MOSI SCK*/
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    spi_config.frequency = NRF_DRV_SPI_FREQ_125K;
    spi_config.mode = NRF_DRV_SPI_MODE_1;

    ret_code_t err_code = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    /* Configure board. */
    //nrf_gpio_cfg_output(13);
    nrf_gpio_cfg_output(SPI_SS_PIN_INVERTED);
    printf("BLINKAGE DE LED WOW\n");
    init_spi();
    printf("CONFIG SPI.");
    /* Toggle LEDs. */
    while (true)
    {
        m_rx_buf[0] = 0;
        m_rx_buf[1] = 0;
        m_rx_buf[2] = 0;
        spi_xfer_done = false;

        nrf_gpio_pin_set(SPI_SS_PIN_INVERTED);
        ret_code_t err_code = nrf_drv_spi_transfer(&spi, m_tx_buf, 2, m_rx_buf, 3);
        APP_ERROR_CHECK(err_code);
        
        while (!spi_xfer_done)
        {
            __WFE();
        }
        nrf_delay_ms(10);
    }
}

/**
 *@}
 **/
