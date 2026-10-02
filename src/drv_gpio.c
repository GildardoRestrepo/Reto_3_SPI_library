/**
 * @file    drv_gpio.c
 * @brief   API GPIO: valida parámetros y ordena la configuración para evitar glitches.
 */
#include <stddef.h>
#include "drv_gpio.h"
#include "ll_gpio.h"
#include "ll_rcc.h"

static int pin_valido(GPIO_Pin_t pin)
{
    return (pin.port < LL_GPIO_PORT_COUNT) && (pin.pin < 16U);
}

GPIO_Status_t GPIO_Init(GPIO_Pin_t pin, const GPIO_Config_t *cfg)
{
    if (cfg == NULL || !pin_valido(pin) || cfg->mode > GPIO_MODE_ANALOG ||
        cfg->pull > GPIO_PULL_DOWN || cfg->af > 15U) {
        return GPIO_ERR_PARAM;
    }

    ll_rcc_gpio_clock_enable(pin.port);

    /* 1. Si es salida, fijar el nivel ANTES de activar el modo: el pin nunca muestra un valor falso
     *    (clave para el CS del ADXL345, que debe arrancar en alto). */
    if (cfg->mode == GPIO_MODE_OUTPUT) {
        GPIO_Write(pin, cfg->init_level);
    }

    /* 2. Características eléctricas y función alterna */
    ll_gpio_set_otype(pin.port, pin.pin, cfg->otype);
    ll_gpio_set_speed(pin.port, pin.pin, cfg->speed);
    ll_gpio_set_pull (pin.port, pin.pin, cfg->pull);
    if (cfg->mode == GPIO_MODE_AF) {
        ll_gpio_set_af(pin.port, pin.pin, cfg->af);   /* AF antes que MODER: el periférico toma
                                                         el pin ya con la función correcta */
    }

    /* 3. El modo al final */
    ll_gpio_set_mode(pin.port, pin.pin, cfg->mode);
    return GPIO_OK;
}

void GPIO_Write(GPIO_Pin_t pin, GPIO_Level_t level)
{
    if (level == GPIO_HIGH) ll_gpio_set_pin(pin.port, pin.pin);
    else                    ll_gpio_reset_pin(pin.port, pin.pin);
}

void GPIO_Toggle(GPIO_Pin_t pin)
{
    GPIO_Write(pin, ll_gpio_read_output(pin.port, pin.pin) ? GPIO_LOW : GPIO_HIGH);
}

GPIO_Level_t GPIO_Read(GPIO_Pin_t pin)
{
    return ll_gpio_read_input(pin.port, pin.pin) ? GPIO_HIGH : GPIO_LOW;
}
