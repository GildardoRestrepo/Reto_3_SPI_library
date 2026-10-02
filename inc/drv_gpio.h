/**
 * @file    drv_gpio.h
 * @brief   API de alto nivel para GPIO. No toca registros: usa ll_gpio y ll_rcc.
 */
#ifndef DRV_GPIO_H
#define DRV_GPIO_H

#include <stdint.h>

typedef enum { GPIO_PORT_A = 0, GPIO_PORT_B, GPIO_PORT_C, GPIO_PORT_D, GPIO_PORT_E } GPIO_Port_t;

typedef struct {
    GPIO_Port_t port;
    uint8_t     pin;          /* 0..15 */
} GPIO_Pin_t;

/* Los valores coinciden con los códigos de los registros (RM0090, 8.4) */
typedef enum { GPIO_MODE_INPUT = 0, GPIO_MODE_OUTPUT = 1, GPIO_MODE_AF = 2, GPIO_MODE_ANALOG = 3 } GPIO_Mode_t;
typedef enum { GPIO_OTYPE_PUSHPULL = 0, GPIO_OTYPE_OPENDRAIN = 1 } GPIO_OType_t;
typedef enum { GPIO_SPEED_LOW = 0, GPIO_SPEED_MEDIUM = 1, GPIO_SPEED_HIGH = 2, GPIO_SPEED_VERY_HIGH = 3 } GPIO_Speed_t;
typedef enum { GPIO_PULL_NONE = 0, GPIO_PULL_UP = 1, GPIO_PULL_DOWN = 2 } GPIO_Pull_t;
typedef enum { GPIO_LOW = 0, GPIO_HIGH = 1 } GPIO_Level_t;
typedef enum { GPIO_OK = 0, GPIO_ERR_PARAM } GPIO_Status_t;

typedef struct {
    GPIO_Mode_t  mode;
    GPIO_OType_t otype;
    GPIO_Speed_t speed;
    GPIO_Pull_t  pull;
    uint8_t      af;          /* 0..15, solo se usa si mode = GPIO_MODE_AF               */
    GPIO_Level_t init_level;  /* solo si mode = GPIO_MODE_OUTPUT: nivel antes de activar  */
} GPIO_Config_t;

GPIO_Status_t GPIO_Init(GPIO_Pin_t pin, const GPIO_Config_t *cfg);
void          GPIO_Write(GPIO_Pin_t pin, GPIO_Level_t level);
void          GPIO_Toggle(GPIO_Pin_t pin);
GPIO_Level_t  GPIO_Read(GPIO_Pin_t pin);

#endif /* DRV_GPIO_H */
