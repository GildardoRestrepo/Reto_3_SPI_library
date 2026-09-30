# Reto 3: Protocolos de Comunicación Serial

---
Autores: Marco Aurelio Guardia Medrano & Gildardo Estevan Restrepo Duque
Curso: Microcontroladores.
Semestre-curso: 2026-02

---
## Descripción

Driver **Bare-Metal** del periférico **SPI** de un microcontrolador STM32, escrito desde el nivel de registros sin usar librerías del fabricante (STM32Cube HAL/LL prohibidas). Sobre el driver se construye una **Capa de Abstracción de Hardware (HAL) propia**, dividida en:
- **Bajo nivel (capa física):** manipulación directa de registros (`RCC`, `GPIO`, `SPI`): habilitar relojes en APB/AHB, configurar pines en función alterna, modo de reloj (CPOL/CPHA), prescaler del baud rate, gestión de banderas `TXE`, `RXNE`, `BSY`, `OVR`, `MODF`.
- **Alto nivel (API):** funciones públicas con firmas claras (`SPI_Init`, `SPI_TransmitReceive`, `SPI_Write`, `SPI_Read`...) que aíslan a la aplicación de los registros.
**Prueba de concepto (PoC):** lectura continua de aceleración en 3 ejes desde un módulo **GY-291 (ADXL345)** por SPI2 (modo 3, 4 hilos) en una placa **STM32F407VET6**. Como no se usa USART, los datos se visualizan de dos formas:
- **LEDs de la placa** como indicador de inclinación (eje X / eje Y superan un umbral) y de estado del sensor.
- **Live Expressions** del depurador de STM32CubeIDE, observando en tiempo real la estructura global con X, Y, Z en mg.

### Protocolo SPI (resumen)

| Característica | Valor en este proyecto |
| -------------- | ---------------------- |
| Topología | Maestro (STM32) – esclavo (ADXL345) |
| Líneas | `SCK`, `MOSI`, `MISO`, `CS` (NSS por software) |
| Modo | Modo 3 → `CPOL = 1`, `CPHA = 1` (requerido por el ADXL345) |
| Formato | 8 bits, MSB primero, full-duplex |
| Periférico | `SPI2` (bus APB1) |
| Velocidad | ≤ 5 MHz (límite del ADXL345); con HSI 16 MHz → `f_PCLK1 / 4` = 4 MHz |
| Trama ADXL345 | Bit 7 = R/W (1 = lectura), bit 6 = MB (multibyte), bits 5..0 = dirección |

---

## Estado de proyecto
>  En proceso

---

## Entornos
- STM32Cube IDE 2.2.0 (proyecto *Empty*, sin HAL/LL; solo arranque y cabeceras CMSIS de registros).
- Tarjeta objetivo: **STM32F407VET6** (Cortex-M4, 168 MHz máx., 512 KB Flash). Manual de referencia: RM0090.
- Visualización: **Live Expressions** / *Expressions* del depurador de STM32CubeIDE (sin USART).

## Herramientas
- Módulo acelerómetro de 3 ejes `GY-291` (ADXL345), comunicación SPI/I2C — aquí se usa SPI.
- Programador/depurador **ST-LINK V2** por SWD (`SWDIO` = PA13, `SWCLK` = PA14).

---

## Arquitectura del proyecto

El código se organiza en capas. Cada capa solo conoce a la inmediatamente inferior; `main.c` nunca toca registros.

```mermaid
flowchart TD
    APP["Aplicación<br/>main.c / app.c<br/>(lógica de la PoC)"]
    DEV["Driver de dispositivo<br/>dev_adxl345<br/>(registros del sensor)"]
    API["API de alto nivel<br/>drv_spi · drv_gpio · drv_delay"]
    LL["Bajo nivel (registros)<br/>ll_spi · ll_gpio · ll_rcc · ll_systick"]
    HW["Hardware STM32F407VET6<br/>CMSIS: stm32f407xx.h"]

    APP --> DEV
    APP --> API
    DEV --> API
    API --> LL
    LL --> HW
```

| Capa | Prefijo | Qué hace | Qué **no** hace |
| ---- | ------- | -------- | --------------- |
| Aplicación | `app_`, `main` | Orquesta la PoC: inicializa, lee el sensor, actualiza LEDs y variables de depuración | Acceder a registros |
| Dispositivo | `dev_` | Conoce el mapa de registros del ADXL345 y lo expone como funciones (`ADXL345_ReadXYZ`) | Saber qué SPI o pines se usan |
| API (HAL propia) | `drv_` | Interfaz pública y portable del periférico; valida parámetros, maneja *timeouts* y errores | Escribir bits de registros directamente |
| Bajo nivel | `ll_` | Lectura/escritura de bits en registros, banderas de estado | Lógica de negocio |

### Flujo de una lectura del sensor

```mermaid
sequenceDiagram
    participant A as app
    participant D as dev_adxl345
    participant S as drv_spi
    participant L as ll_spi / ll_gpio
    A->>D: ADXL345_ReadXYZ(&acc)
    D->>S: SPI_Read(cs, 0x32 | 0xC0, buf, 6)
    S->>L: CS bajo
    loop 7 bytes (dirección + 6 datos)
        S->>L: esperar TXE, escribir DR
        S->>L: esperar RXNE, leer DR
    end
    S->>L: esperar BSY = 0, CS alto
    D-->>A: x, y, z en mg
```

---

## Etapas de desarrollo

Cada etapa se trabaja en una rama `feature/*` y se integra a `develop` por Pull Request revisado por el compañero. Se busca que ambos integrantes toquen tanto el bajo nivel como la API (se evalúa el dominio de los dos).

| # | Etapa | Entregable verificable | Responsable |
| - | ----- | ---------------------- | ----------- |
| 0 | Estructura del repo y proyecto CubeIDE vacío (STM32F407VETx) | Compila, se programa con ST-LINK y hace parpadear un LED solo con registros | Ambos |
| 1 | `ll_rcc` + `ll_gpio` + `drv_gpio` | Pines en modo alterno AF5 para SPI2, salida para CS y LEDs | Marco |
| 2 | `ll_systick` + `drv_delay` | `Delay_ms()` y base de tiempo para *timeouts* | Gildardo |
| 3 | Visualización sin USART | Variables globales `volatile` visibles en Live Expressions; patrones de LED para estado/error | Marco |
| 4 | `ll_spi` (registros de SPI2) | Configuración de `CR1`/`CR2`, lectura de `SR`, escritura de `DR` | Gildardo |
| 5 | `drv_spi` (API) | `SPI_TransmitReceive`, `SPI_Write`, `SPI_Read` con *timeout* y códigos de error | Ambos |
| 6 | `dev_adxl345` | Lectura de `DEVID = 0xE5`, configuración de rango y lectura XYZ | Marco |
| 7 | Aplicación PoC | Lectura continua; LEDs indican inclinación en X/Y; valores en mg en Live Expressions | Gildardo |
| 8 | Robustez | Manejo de `OVR`/`MODF`, reintentos, sensor desconectado sin bloqueo | Ambos |
| 9 | Documentación | Esquemático, guía de la API, capturas del analizador lógico | Ambos |

---

## Estructura del repositorio

```
09_challenge_3_library_spi_microcontrollers/
├── README.md
├── LICENSE
├── .gitignore
├── .gitattributes
├── .project / .cproject        # Proyecto de STM32CubeIDE (sí se versiona)
├── STM32F407VETX_FLASH.ld      # Linker script
├── startup/
│   └── startup_stm32f407vetx.s
├── drivers/
│   └── CMSIS/                  # Solo cabeceras de registros (core_cm4.h, stm32f407xx.h)
├── inc/
│   ├── ll_rcc.h
│   ├── ll_gpio.h
│   ├── ll_systick.h
│   ├── ll_spi.h
│   ├── drv_gpio.h
│   ├── drv_delay.h
│   ├── drv_spi.h
│   ├── dev_adxl345.h
│   ├── app.h
│   └── board.h                 # Mapa de pines y constantes de la tarjeta
├── src/
│   ├── ll_rcc.c
│   ├── ll_gpio.c
│   ├── ll_systick.c
│   ├── ll_spi.c
│   ├── drv_gpio.c
│   ├── drv_delay.c
│   ├── drv_spi.c
│   ├── dev_adxl345.c
│   ├── app.c
│   ├── main.c
│   ├── syscalls.c / sysmem.c   # Generados por CubeIDE
└── docs/
    ├── esquematico/            # Diagrama de conexión de la PoC
    ├── registros/              # Notas de registros SPI (CR1, CR2, SR, DR) para la sustentación
    ├── capturas/               # Analizador lógico / Live Expressions
    └── datasheets/             # Datasheet STM32F407VE, esquemático y features de la placa, RM0090, ADXL345
```

Módulos previstos en `src/` e `inc/`:

| Módulo         | Responsabilidad                                                                                                                                                                       |
| -------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `board.h`      | Único lugar con el mapa de pines (SPI2, CS, LEDs) y la frecuencia de reloj; cambiar de tarjeta = cambiar este archivo                                                                  |
| `ll_rcc`       | Habilitar/deshabilitar relojes de periféricos en `AHB1ENR`, `APB1ENR`, `APB2ENR`; consultar frecuencia de `PCLK`                                                                      |
| `ll_gpio`      | Escritura de `MODER`, `OTYPER`, `OSPEEDR`, `PUPDR`, `AFR[0/1]`, `BSRR`, `IDR`                                                                                                         |
| `ll_systick`   | Configuración de `SysTick` (`LOAD`, `VAL`, `CTRL`) y contador de milisegundos                                                                                                         |
| `ll_spi`       | Registros de SPI2: `CR1` (`MSTR`, `BR`, `CPOL`, `CPHA`, `SSM`, `SSI`, `SPE`, `DFF`), `CR2`, banderas `TXE`, `RXNE`, `BSY`, `OVR`, `MODF` en `SR`, `DR`                                |
| `drv_gpio`     | API: `GPIO_Init(pin, config)`, `GPIO_Write`, `GPIO_Toggle`, `GPIO_Read`                                                                                                               |
| `drv_delay`    | API: `Delay_Init`, `Delay_ms`, `GetTick` (base de los *timeouts*)                                                                                                                     |
| `drv_spi`      | **API principal del reto:** `SPI_Init(&config)`, `SPI_TransmitReceive(byte)`, `SPI_Write(cs, buf, len)`, `SPI_Read(cs, reg, buf, len)`, `SPI_Deinit`, códigos de error `SPI_Status_t` |
| `dev_adxl345`  | Driver del sensor sobre `drv_spi`: `ADXL345_Init`, `ADXL345_ReadID`, `ADXL345_SetRange`, `ADXL345_ReadXYZ`, conversión a g                                                            |
| `app` / `main` | Lógica de la PoC: inicialización del sistema, lazo de lectura periódica, LEDs de inclinación/error y variables globales para Live Expressions                                         |

---

## Diagrama de conexión (PoC)

Conexión GY-291 ↔ STM32F407VET6 usando **SPI2** (AF5) en modo 4 hilos:

| GY-291 | Función SPI | STM32F407VET6 |
| ------ | ----------- | ------------- |
| `VCC` | Alimentación | `3V3` |
| `GND` | Tierra | `GND` |
| `CS` | Chip Select (GPIO, NSS por software) | `PB12` |
| `SCL` | SCK | `PB13` (AF5) |
| `SDO` | MISO | `PB14` (AF5) |
| `SDA` | MOSI (SDI) | `PB15` (AF5) |
| `INT1`/`INT2` | Interrupciones (opcional) | sin conectar |

**¿Por qué SPI2 y no SPI1?** Según el esquemático de la placa ([`docs/datasheets/stm32f407vet_schematics.pdf`](docs/datasheets/stm32f407vet_schematics.pdf)):
- `PA6`/`PA7` (MISO/MOSI de SPI1) están cableados a los LEDs D2/D3.
- `PB3`/`PB4`/`PB5` (SPI1 remapeado) ya son el bus de la Flash W25Q16 (CS = `PB0`) y del zócalo NRF24L01 (CE = `PB6`, CS = `PB7`).
- `PB12..PB15` (SPI2) solo van al conector de la pantalla TFT (`T_CS`, `T_SCK`, `T_MISO`, `T_MOSI` del táctil) y al header J2 → libres mientras **no haya una TFT conectada**.

Indicadores (sin USART):

| Elemento | Pin | Uso en la PoC |
| -------- | --- | ------------- |
| LED D2 | `PA6` (ánodo a 3V3 → activo en bajo) | Inclinación en X supera ±0,5 g |
| LED D3 | `PA7` (ánodo a 3V3 → activo en bajo) | Inclinación en Y supera ±0,5 g |
| Ambos LEDs parpadeando | — | Error: `DEVID ≠ 0xE5` o *timeout* SPI (sensor desconectado) |
| Botón K0 / K1 | `PE4` / `PE3` (a GND → usar pull-up) | Opcional: cambiar rango (±2 g / ±4 g) o umbral en vivo |

Programación y depuración con ST-LINK V2 por el conector SWD (`3V3`, `GND`, `PA13` SWDIO, `PA14` SWCLK).

> Pendiente: agregar el esquemático en `docs/esquematico/`.

---

## Guía de uso de la API
```c
#include "drv_spi.h"
#include "dev_adxl345.h"

/* Visible en Live Expressions de STM32CubeIDE */
volatile ADXL345_Data_t g_acc;
volatile SPI_Status_t   g_spi_status;

SPI_Config_t spi_cfg = {
    .instance  = SPI_2,
    .mode      = SPI_MODE_3,       /* CPOL = 1, CPHA = 1 */
    .prescaler = SPI_BAUD_DIV_4,   /* PCLK1 16 MHz / 4 = 4 MHz */
    .bit_order = SPI_MSB_FIRST,
};

SPI_Init(&spi_cfg);
ADXL345_Init(ADXL345_RANGE_2G);

g_spi_status = ADXL345_ReadXYZ(&g_acc);
if (g_spi_status == SPI_OK) {
    GPIO_Write(LED_X, (g_acc.x_mg > 500 || g_acc.x_mg < -500));
    GPIO_Write(LED_Y, (g_acc.y_mg > 500 || g_acc.y_mg < -500));
}
```

| Función | Descripción | Retorno |
| ------- | ----------- | ------- |
| `SPI_Init(const SPI_Config_t *cfg)` | Habilita reloj, configura pines y registros, activa el periférico | `SPI_Status_t` |
| `SPI_TransmitReceive(uint8_t tx, uint8_t *rx)` | Envía un byte y recibe el byte simultáneo (full-duplex) | `SPI_OK`, `SPI_TIMEOUT`, `SPI_ERR_OVR` |
| `SPI_Write(cs, const uint8_t *buf, uint16_t len)` | Baja CS, envía `len` bytes, espera `BSY = 0`, sube CS | `SPI_Status_t` |
| `SPI_Read(cs, uint8_t reg, uint8_t *buf, uint16_t len)` | Envía la dirección y lee `len` bytes | `SPI_Status_t` |
| `SPI_Deinit()` | Deshabilita el periférico y su reloj | `void` |

---
## Cómo reproducir e implementar

1. Clonar el repositorio:
   ```bash
   git clone https://github.com/<usuario>/<repo>.git
   ```
2. En STM32CubeIDE: `File → Import → General → Existing Projects into Workspace` y seleccionar la carpeta clonada.
3. Verificar que `inc/` esté en *Include paths* y `src/` en *Source Location* (`Project → Properties → C/C++ General → Paths and Symbols`).
4. Compilar con `Project → Build Project` (configuración `Debug`).
5. Conectar el GY-291 según la tabla de conexión y el ST-LINK V2 al conector SWD de la placa.
6. `Run → Debug As → STM32 C/C++ Application` y luego *Resume* (F8).
7. En la vista **Live Expressions** (`Window → Show View → Live Expressions`) agregar `g_acc` y `g_spi_status` para ver X, Y, Z en mg en tiempo real. Inclinar la placa del sensor y verificar que los LEDs D2/D3 respondan.

---
## Flujo de trabajo en Git
- `main`: solo versiones estables y demostrables (cada merge = algo que funciona en hardware).
- `develop`: integración del trabajo en curso.
- `feature/<modulo>` (p. ej. `feature/ll-spi`, `feature/dev-adxl345`): una rama por módulo o etapa.
- Todo cambio entra a `develop` por **Pull Request** revisado por el otro integrante.
- Mensajes de commit en español, en imperativo y con prefijo: `feat:`, `fix:`, `docs:`, `refactor:`, `test:`, `chore:`.
  Ejemplo: `feat(drv_spi): agregar SPI_TransmitReceive con timeout`.

---
## Contacto
```
marco.guardia@upb.edu.co
gildardo.restrepo@upb.edu.co
```

---

## Enlaces
[[proyectos_personal|Proyectos Electrónica]]
