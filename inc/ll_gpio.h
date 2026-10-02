/**
 * @file    ll_gpio.h
 * @brief   Bajo nivel: escritura/lectura de registros GPIO (RM0090, cap. 8).
 *
 * Capa: ll_  →  solo manipula registros. No valida parámetros (eso lo hace drv_gpio).
 * port: 0 = GPIOA ... 4 = GPIOE   |   pin: 0..15
 */
#ifndef LL_GPIO_H
#define LL_GPIO_H

#include <stdint.h>

#define LL_GPIO_PORT_COUNT   5U   /* STM32F407VE (100 pines): GPIOA..GPIOE */

void     ll_gpio_set_mode(uint8_t port, uint8_t pin, uint32_t mode);     /* MODER:   2 bits/pin */
void     ll_gpio_set_otype(uint8_t port, uint8_t pin, uint32_t otype);   /* OTYPER:  1 bit/pin  */
void     ll_gpio_set_speed(uint8_t port, uint8_t pin, uint32_t speed);   /* OSPEEDR: 2 bits/pin */
void     ll_gpio_set_pull(uint8_t port, uint8_t pin, uint32_t pull);     /* PUPDR:   2 bits/pin */
void     ll_gpio_set_af(uint8_t port, uint8_t pin, uint32_t af);         /* AFR[0/1]: 4 bits/pin */

void     ll_gpio_set_pin(uint8_t port, uint8_t pin);     /* BSRR bits 0..15  → salida = 1 */
void     ll_gpio_reset_pin(uint8_t port, uint8_t pin);   /* BSRR bits 16..31 → salida = 0 */
uint32_t ll_gpio_read_input(uint8_t port, uint8_t pin);  /* IDR: nivel real del pin      */
uint32_t ll_gpio_read_output(uint8_t port, uint8_t pin); /* ODR: nivel que se ordenó     */

#endif /* LL_GPIO_H */
