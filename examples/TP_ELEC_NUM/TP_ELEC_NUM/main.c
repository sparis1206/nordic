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

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "app_error.h"
#include "app_util_platform.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_twi.h"

/* PIN Definition */
#define TWI_SCL_PIN 26
#define TWI_SDA_PIN 27
/* SLAVE Adress*/
#define MCP_ADDR 0x18
/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* Indicates if operation on TWI has ended. */
static volatile bool m_xfer_done = false;
/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
/* Buffer for samples read from temperature sensor. */
static uint8_t m_sample[2];

/**
 * @brief Function for setting active mode on MMA7660 accelerometer.
 */
void MCP_set_mode(void)
{
    ret_code_t err_code;

    /* Desactive shutdown */
    uint8_t config[3] = {0x01, 0x00, 0x00};
    err_code = nrf_drv_twi_tx(&m_twi, MCP_ADDR, config, 3, false);
    APP_ERROR_CHECK(err_code);
    while (m_xfer_done == false);
}



/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            m_xfer_done = true;
            break;
        default:
            break;
    }
}

/**
 * @brief UART initialization.
 */
void twi_init (void)
{
    nrf_gpio_cfg_input(TWI_SCL_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(TWI_SDA_PIN, NRF_GPIO_PIN_PULLUP);
    const nrf_drv_twi_config_t twi_config = {
       .scl                = TWI_SCL_PIN,
       .sda                = TWI_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    ret_code_t err_code = nrf_drv_twi_init(&m_twi, &twi_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

/**
 * @brief Function for reading data from temperature sensor.
 */
static void read_sensor_data()
{
    m_xfer_done = false;
    uint8_t data_tx[1] = {0x05};
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, MCP_ADDR, data_tx, sizeof(data_tx), true);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;
    /* Read 1 byte from the specified address - skip 3 bits dedicated for fractional part of temperature. */
    err_code = nrf_drv_twi_rx(&m_twi, MCP_ADDR, m_sample, sizeof(m_sample));
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    printf("Temperature : %d-%d\n", m_sample[0], m_sample[1]);
}

int main(void)
{
    printf("\nTWI sensor example started.");
    /* Configure board. */
    twi_init();
    MCP_set_mode();
    printf("Config MCP completed.");
    while (true)
    {
        do
        {
            __WFE();
        }while (m_xfer_done == false);

        read_sensor_data();
        nrf_delay_ms(200);
    }
}

/**
 *@}
 **/
