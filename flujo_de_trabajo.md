---
title: Flujo de trabajo en Git por equipos
created: 2026-09-30
creator: Gilbert
last update: 2026-09-30
update by: Gilbert
type: guia
status: activo
area: gestion general de proyectos
tags:
  - tipo/guia
---

# Flujo de trabajo en Git por equipos

> [!success] Resumen
> Cada equipo trabaja en ramas cortas por módulo. Todo llega a `main` por **Pull Request revisado por el otro equipo**.

---

## Reglas básicas

1. **Nadie hace commit ni push directo a `main`.** `main` siempre compila y funciona en la placa.
2. **Una rama por módulo o etapa**, de vida corta: se crea, se hace el PR, se hace el merge y se borra.
3. **Todo PR lo revisa y aprueba el otro equipo** antes del merge.
4. **Antes de abrir un PR, se trae lo último de `main`** a la rama y se resuelven los conflictos en la rama, no en `main`.
5. **Commits pequeños y frecuentes**, cada uno con un solo propósito. La rúbrica evalúa un historial equilibrado entre ambos.
6. **Archivos compartidos con dueño único:** `board.h` y `main.c` los modifica un solo equipo; el otro pide cambios en el PR.

---

## Ramas y reparto

| Rama | Contenido | Equipo |
| ---- | --------- | ------ |
| `main` | Versión estable y demostrable | — (solo por PR) |
| `feature/interfaces` | Los `.h` acordados: `board.h`, `drv_gpio.h`, `drv_delay.h`, `drv_spi.h`, `dev_adxl345.h` | Ambos (primero que todo) |
| `feature/proyecto-base` | Proyecto CubeIDE vacío, arranque, CMSIS, LED parpadeando | Gilbert |
| `feature/gpio` | `ll_rcc`, `ll_gpio`, `drv_gpio` | Marco |
| `feature/delay` | `ll_systick`, `drv_delay` | Gilbert |
| `feature/visualizacion` | LEDs de estado y variables para Live Expressions | Marco |
| `feature/ll-spi` | `ll_spi` (registros de SPI2) | Gilbert |
| `feature/drv-spi` | `drv_spi` (API) | Ambos (uno la escribe y el otro la revisa) |
| `feature/dev-adxl345` | `dev_adxl345` | Marco |
| `feature/app-poc` | `app.c`, `main.c` | Gilbert |
| `docs/<tema>` | Documentación, esquemáticos, capturas | Cualquiera |
| `fix/<problema>` | Corrección de un error ya integrado en `main` | Quien lo encuentre |

> Primero se hace el merge de `feature/interfaces`. Con las firmas fijas, cada equipo puede programar contra los `.h` aunque el otro no haya terminado su implementación.

---

## 1. Configuración inicial (una sola vez por computador)

Cada uno con **su** nombre y el correo de **su** cuenta de GitHub, para que los commits cuenten a su nombre:

```bash
git config --global user.name "Nombre Apellido"
```

```bash
git config --global user.email "correo-de-github@ejemplo.com"
```

```bash
git config --global pull.rebase false
```

```bash
git config --global init.defaultBranch main
```

El dueño del repo, dentro de la carpeta del proyecto, crea el repositorio y lo sube:

```bash
git init
```

```bash
git add .
```

```bash
git commit -m "chore: estructura inicial del repositorio"
```

```bash
gh repo create reto3-spi-baremetal --public --source=. --remote=origin --push
```

> Sin GitHub CLI: crear el repo **vacío** en github.com y luego ejecutar `git remote add origin https://github.com/<usuario>/reto3-spi-baremetal.git` y `git push -u origin main`.

Luego, en GitHub:
- **Settings → Collaborators → Add people**: invitar a Marco.
- **Settings → Branches → Add branch ruleset** (o *Add rule*) sobre `main`: activar *Require a pull request before merging* con 1 aprobación, y bloquear el *force push*.

**Marco** acepta la invitación (llega al correo) y clona el repo:
```bash
git clone https://github.com/<usuario-gilbert>/reto3-spi-baremetal.git
```

---

## 2. Ciclo de trabajo de cada equipo

### 2.1 Empezar una tarea
Siempre desde un `main` actualizado:

```bash
git switch main
```

```bash
git pull
```

```bash
git switch -c feature/ll-spi
```

### 2.2 Trabajar y hacer commits
Mientras Claude y el integrante desarrollan, ver qué cambió:

```bash
git status
```

```bash
git diff
```

Agregar **solo** los archivos de este cambio (evitar `git add .` si hay archivos de otra tarea):

```bash
git add src/ll_spi.c inc/ll_spi.h
```

```bash
git commit -m "feat(ll_spi): configurar CR1 en modo maestro, CPOL=1 y CPHA=1"
```

Subir la rama (la primera vez con `-u`, después basta `git push`):

```bash
git push -u origin feature/ll-spi
```

> Hagan push al menos al final de cada sesión, aunque la tarea no esté terminada: así el trabajo queda respaldado y el otro equipo puede verlo.

### 2.3 Antes de abrir el Pull Request: traer lo último de `main`

```bash
git fetch origin
```

```bash
git merge origin/main
```

Si hay conflictos, ver la sección 4. Luego **compilar en STM32CubeIDE y probar en la placa**, y subir:

```bash
git push
```

### 2.4 Abrir el Pull Request
En GitHub: **Compare & pull request**, con base `main`. O desde la terminal:

```bash
gh pr create --base main --title "feat(ll_spi): registros de SPI2" --body "Qué hace, cómo se probó y qué falta"
```

En la descripción del PR incluyan:
- **Qué** se implementó (módulos y funciones).
- **Cómo se probó** (en la placa, con Live Expressions o con el analizador lógico).
- **Pendientes** o dudas para el revisor.

Avisen al otro equipo por el chat del grupo con el enlace al PR.

### 2.5 Revisar el PR del otro equipo
Traer la rama del otro equipo para compilarla y probarla localmente:

```bash
git fetch origin
```

```bash
git switch feature/dev-adxl345
```

Después de probarla, en GitHub, pestaña **Files changed → Review changes**, elegir *Approve* o *Request changes*, con comentarios en líneas concretas. Si se piden cambios, el autor hace nuevos commits en la misma rama y hace push; el PR se actualiza solo.

### 2.6 Merge y cierre
Con el PR aprobado, **el autor** hace clic en **Merge pull request** (*Create a merge commit*) y luego en **Delete branch**. Después, localmente:

```bash
git switch main
```

```bash
git pull
```

```bash
git branch -d feature/ll-spi
```

Y avisa al otro equipo: **"merge de `feature/ll-spi` en `main`"**.

---

## 3. Cuando el otro equipo avisa de un merge

Si **no** tienen trabajo en curso, basta con actualizar `main`:

```bash
git switch main
```

```bash
git pull
```

Si **sí** están trabajando en una rama, integren los cambios nuevos a su rama en ese momento, no al final:

```bash
git fetch origin
```

```bash
git merge origin/main
```

---

## 4. Resolver conflictos

Si `git merge` informa conflictos:

1. Ver qué archivos tienen conflicto:
   ```bash
   git status
   ```
2. Abrir cada archivo y buscar los marcadores `<<<<<<<`, `=======` y `>>>>>>>`. Dejar el código correcto y borrar los marcadores. Claude puede ayudar a resolverlos, pero **si el conflicto está en código del otro equipo, consúltenlo con ellos**.
3. Marcar los archivos como resueltos y cerrar el merge:
   ```bash
   git add inc/drv_spi.h
   ```
   ```bash
   git commit
   ```
4. Compilar y probar antes del push.

Para abortar el merge y volver al estado anterior:

```bash
git merge --abort
```

---

## 5. Convención de commits

Formato: `tipo(módulo): descripción en imperativo y en minúscula`

| Tipo | Uso | Ejemplo |
| ---- | --- | ------- |
| `feat` | Funcionalidad nueva | `feat(drv_spi): agregar SPI_TransmitReceive con timeout` |
| `fix` | Corrección de un error | `fix(ll_spi): esperar BSY=0 antes de subir CS` |
| `refactor` | Cambio interno sin alterar el comportamiento | `refactor(drv_gpio): unificar configuración de pines` |
| `docs` | README, guías, diagramas | `docs: agregar esquemático de conexión` |
| `test` | Código de prueba o validación | `test(dev_adxl345): verificar lectura de DEVID` |
| `chore` | Configuración del proyecto o del repo | `chore: actualizar .gitignore` |

Nombres de rama: `feature/<modulo>`, `fix/<problema>`, `docs/<tema>`, en minúscula y con guiones (`feature/dev-adxl345`).

---

## 6. Qué **no** hacer

- `git push --force` sobre `main` o sobre la rama de otro.
- Commits de archivos generados (`Debug/`, `*.elf`, `.metadata/`); el `.gitignore` los excluye, no lo desactiven.
- Mezclar dos tareas en una rama o en un commit.
- Hacer el merge de un PR propio sin la aprobación del otro equipo.
- Dejar una rama semanas sin integrar `main`: más conflictos al final.

---
## Enlaces
- [README del proyecto](README.md)
- [[proyectos_personal|Proyectos Electrónica]]
