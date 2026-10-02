/**
 * @file    board.h
 * @brief   Único lugar con el mapa de pines de la tarjeta. Cambiar de placa = cambiar este archivo.
 *          Placa: STM32F407VE "negra" (esquemático en docs/datasheets/stm32f407vet_schematics.pdf).
 */
#ifndef BOARD_H
#define BOARD_H

#include "drv_gpio.h"

/* ---- Reloj ---------------------------------------------------------------- */
#define BOARD_HSE_HZ        8000000U     /* cristal de 8 MHz de la placa (no se usa con HSI) */

/* ---- LEDs de usuario (D2, D3): 510 Ω a 3V3 → encienden con nivel BAJO ---------- */
#define BOARD_LED_D2        ((GPIO_Pin_t){ GPIO_PORT_A, 6U })
#define BOARD_LED_D3        ((GPIO_Pin_t){ GPIO_PORT_A, 7U })
#define BOARD_LED_ON        GPIO_LOW
#define BOARD_LED_OFF       GPIO_HIGH

/* ---- SPI hacia el ADXL345 (GY-291) --------------------------------------------
 * Se usa SPI2 y no SPI1 porque, en esta placa:
 *   - PA6/PA7 (SPI1) tienen los LEDs D2/D3 → cargarían MISO.
 *   - PB3/PB4/PB5 (SPI1 remapeado) comparten bus con la flash W25Q16 y el zócalo NRF24L01,
 *     y PB3/PB4 son pines JTAG.
 *   - PB13/PB14/PB15 (SPI2) solo se comparten con el táctil del conector TFT (sin pantalla: libres).
 * Acordado con el compañero: ver "Diagrama de conexión (PoC)" en el README. */
#define BOARD_SPI_INSTANCE  2U
#define BOARD_SPI_AF        5U           /* AF5 = SPI1/SPI2 */
#define BOARD_SPI_SCK       ((GPIO_Pin_t){ GPIO_PORT_B, 13U })   /* → GY-291 SCL */
#define BOARD_SPI_MISO      ((GPIO_Pin_t){ GPIO_PORT_B, 14U })   /* ← GY-291 SDO */
#define BOARD_SPI_MOSI      ((GPIO_Pin_t){ GPIO_PORT_B, 15U })   /* → GY-291 SDA */
#define BOARD_ADXL345_CS    ((GPIO_Pin_t){ GPIO_PORT_B, 12U })   /* → GY-291 CS (GPIO, activo en bajo) */

#endif /* BOARD_H */
