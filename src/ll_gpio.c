/**
 * @file    ll_gpio.c
 * @brief   Bajo nivel GPIO para STM32F407VE (RM0090, cap. 8 "General-purpose I/Os").
 */
#include "stm32f407xx.h"
#include "ll_gpio.h"

/* Índice de puerto → registros del puerto */
static GPIO_TypeDef * const gpio_port[LL_GPIO_PORT_COUNT] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE };

/* Máscaras: campos de 2 bits (MODER, OSPEEDR, PUPDR) y de 4 bits (AFR) */
#define MASK2(pin)   (0x3UL << ((pin) * 2U))
#define MASK4(pin)   (0xFUL << (((pin) % 8U) * 4U))

void ll_gpio_set_mode(uint8_t port, uint8_t pin, uint32_t mode)
{
    GPIO_TypeDef *p = gpio_port[port];
    p->MODER = (p->MODER & ~MASK2(pin)) | ((mode & 0x3UL) << (pin * 2U));
}

void ll_gpio_set_otype(uint8_t port, uint8_t pin, uint32_t otype)
{
    GPIO_TypeDef *p = gpio_port[port];
    p->OTYPER = (p->OTYPER & ~(1UL << pin)) | ((otype & 0x1UL) << pin);
}

void ll_gpio_set_speed(uint8_t port, uint8_t pin, uint32_t speed)
{
    GPIO_TypeDef *p = gpio_port[port];
    p->OSPEEDR = (p->OSPEEDR & ~MASK2(pin)) | ((speed & 0x3UL) << (pin * 2U));
}

void ll_gpio_set_pull(uint8_t port, uint8_t pin, uint32_t pull)
{
    GPIO_TypeDef *p = gpio_port[port];
    p->PUPDR = (p->PUPDR & ~MASK2(pin)) | ((pull & 0x3UL) << (pin * 2U));
}

void ll_gpio_set_af(uint8_t port, uint8_t pin, uint32_t af)
{
    GPIO_TypeDef *p = gpio_port[port];
    uint32_t idx = pin / 8U;                       /* AFR[0]: pines 0-7, AFR[1]: pines 8-15 */
    p->AFR[idx] = (p->AFR[idx] & ~MASK4(pin)) | ((af & 0xFUL) << ((pin % 8U) * 4U));
}

void ll_gpio_set_pin(uint8_t port, uint8_t pin)
{
    gpio_port[port]->BSRR = (1UL << pin);          /* escritura atómica, no hace read-modify-write */
}

void ll_gpio_reset_pin(uint8_t port, uint8_t pin)
{
    gpio_port[port]->BSRR = (1UL << (pin + 16U));
}

uint32_t ll_gpio_read_input(uint8_t port, uint8_t pin)
{
    return (gpio_port[port]->IDR >> pin) & 0x1UL;
}

uint32_t ll_gpio_read_output(uint8_t port, uint8_t pin)
{
    return (gpio_port[port]->ODR >> pin) & 0x1UL;
}
