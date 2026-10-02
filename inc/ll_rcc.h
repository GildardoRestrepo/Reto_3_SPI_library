/**
 * @file    ll_rcc.h
 * @brief   Bajo nivel: habilitación de relojes de periféricos y consulta de frecuencias.
 *          STM32F407VE — acceso directo a registros RCC (RM0090, cap. 7).
 *
 * Capa: ll_  →  solo manipula registros. No valida parámetros (eso lo hace la capa drv_).
 */
#ifndef LL_RCC_H
#define LL_RCC_H

#include <stdint.h>

/* Cristal externo de la placa (STM32F407VE negra: 8 MHz). Solo se usa si SYSCLK = HSE o PLL←HSE. */
#ifndef BOARD_HSE_HZ
#define BOARD_HSE_HZ   8000000U
#endif

/* ---- Relojes de puertos GPIO (bus AHB1) --------------------------------- */
/* port: 0 = GPIOA, 1 = GPIOB, 2 = GPIOC, 3 = GPIOD, 4 = GPIOE                */
void     ll_rcc_gpio_clock_enable(uint8_t port);
void     ll_rcc_gpio_clock_disable(uint8_t port);

/* ---- Relojes de SPI ------------------------------------------------------ */
/* spi: 1 = SPI1 (APB2), 2 = SPI2 (APB1), 3 = SPI3 (APB1)                     */
void     ll_rcc_spi_clock_enable(uint8_t spi);
void     ll_rcc_spi_clock_disable(uint8_t spi);

/* ---- Frecuencias actuales (leídas de RCC_CFGR / RCC_PLLCFGR) ------------- */
uint32_t ll_rcc_get_sysclk_hz(void);
uint32_t ll_rcc_get_hclk_hz(void);    /* AHB                         */
uint32_t ll_rcc_get_pclk1_hz(void);   /* APB1: SPI2, SPI3            */
uint32_t ll_rcc_get_pclk2_hz(void);   /* APB2: SPI1                  */

#endif /* LL_RCC_H */
