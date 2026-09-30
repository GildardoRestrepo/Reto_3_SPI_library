/**
 * @file    ll_rcc.c
 * @brief   Bajo nivel RCC para STM32F407VE (RM0090, cap. 7 "Reset and clock control").
 */
#include "stm32f407xx.h"
#include "ll_rcc.h"

#define HSI_HZ   16000000U   /* Oscilador interno: fijo en el silicio (RM0090, 7.2.2) */

/* ------------------------------------------------------------------------- */
/* GPIO: en RCC_AHB1ENR el bit n habilita el puerto n (GPIOAEN = bit 0 ...)   */
/* ------------------------------------------------------------------------- */
void ll_rcc_gpio_clock_enable(uint8_t port)
{
    RCC->AHB1ENR |= (1UL << port);
    (void)RCC->AHB1ENR;   /* lectura dummy: garantiza que el reloj esté activo antes de usar el puerto */
}

void ll_rcc_gpio_clock_disable(uint8_t port)
{
    RCC->AHB1ENR &= ~(1UL << port);
}

/* ------------------------------------------------------------------------- */
/* SPI                                                                        */
/* ------------------------------------------------------------------------- */
void ll_rcc_spi_clock_enable(uint8_t spi)
{
    switch (spi) {
    case 1: RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; (void)RCC->APB2ENR; break;
    case 2: RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; (void)RCC->APB1ENR; break;
    case 3: RCC->APB1ENR |= RCC_APB1ENR_SPI3EN; (void)RCC->APB1ENR; break;
    default: break;
    }
}

void ll_rcc_spi_clock_disable(uint8_t spi)
{
    switch (spi) {
    case 1: RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN; break;
    case 2: RCC->APB1ENR &= ~RCC_APB1ENR_SPI2EN; break;
    case 3: RCC->APB1ENR &= ~RCC_APB1ENR_SPI3EN; break;
    default: break;
    }
}

/* ------------------------------------------------------------------------- */
/* Frecuencias                                                                */
/* ------------------------------------------------------------------------- */
uint32_t ll_rcc_get_sysclk_hz(void)
{
    uint32_t sws = (RCC->CFGR & RCC_CFGR_SWS_Msk) >> RCC_CFGR_SWS_Pos;

    if (sws == 0U) return HSI_HZ;          /* 00: HSI */
    if (sws == 1U) return BOARD_HSE_HZ;    /* 01: HSE */

    /* 10: PLL →  f = f_in / PLLM * PLLN / PLLP   (RM0090, 7.3.2 RCC_PLLCFGR) */
    uint32_t pllcfgr = RCC->PLLCFGR;
    uint32_t f_in = (pllcfgr & RCC_PLLCFGR_PLLSRC) ? BOARD_HSE_HZ : HSI_HZ;
    uint32_t pllm = (pllcfgr & RCC_PLLCFGR_PLLM_Msk) >> RCC_PLLCFGR_PLLM_Pos;
    uint32_t plln = (pllcfgr & RCC_PLLCFGR_PLLN_Msk) >> RCC_PLLCFGR_PLLN_Pos;
    uint32_t pllp = ((((pllcfgr & RCC_PLLCFGR_PLLP_Msk) >> RCC_PLLCFGR_PLLP_Pos) + 1U) * 2U);

    if (pllm == 0U) return 0U;             /* configuración inválida */
    return (uint32_t)(((uint64_t)f_in * plln) / (pllm * pllp));
}

uint32_t ll_rcc_get_hclk_hz(void)
{
    /* HPRE[3:0]: 0xxx = /1; 1000..1111 = /2, /4, /8, /16, /64, /128, /256, /512 */
    static const uint8_t shift[8] = { 1, 2, 3, 4, 6, 7, 8, 9 };
    uint32_t hpre = (RCC->CFGR & RCC_CFGR_HPRE_Msk) >> RCC_CFGR_HPRE_Pos;
    uint32_t sysclk = ll_rcc_get_sysclk_hz();

    return (hpre & 0x8U) ? (sysclk >> shift[hpre & 0x7U]) : sysclk;
}

/* PPREx[2:0]: 0xx = /1; 100 = /2, 101 = /4, 110 = /8, 111 = /16 */
static uint32_t apb_div(uint32_t hclk, uint32_t ppre)
{
    return (ppre & 0x4U) ? (hclk >> ((ppre & 0x3U) + 1U)) : hclk;
}

uint32_t ll_rcc_get_pclk1_hz(void)
{
    return apb_div(ll_rcc_get_hclk_hz(), (RCC->CFGR & RCC_CFGR_PPRE1_Msk) >> RCC_CFGR_PPRE1_Pos);
}

uint32_t ll_rcc_get_pclk2_hz(void)
{
    return apb_div(ll_rcc_get_hclk_hz(), (RCC->CFGR & RCC_CFGR_PPRE2_Msk) >> RCC_CFGR_PPRE2_Pos);
}
