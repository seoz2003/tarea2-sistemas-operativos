# Tarea 2 - Sistemas Operativos UDEC 2025

Este proyecto contiene la implementación de dos partes:

## Parte I: Sincronización con Barrera Reutilizable

Implementación de una barrera reutilizable usando primitivas de sincronización de pthreads.

### Archivos

- `barrera.h` - Definición de la interfaz de la barrera
- `barrera.c` - Implementación de la barrera usando `pthread_mutex_t` y `pthread_cond_t`
- `main.c` - Programa de verificación que prueba la barrera con múltiples hebras

### Compilación

```bash
make barrera_test
```

O compilar manualmente:
```bash
gcc -Wall -Wextra -std=c99 -pthread -c barrera.c
gcc -Wall -Wextra -std=c99 -pthread -c main.c
gcc -pthread -o barrera_test main.o barrera.o
```

### Ejecución

**Con valores por defecto (5 hebras, 4 etapas):**
```bash
.\barrera_test.exe
```

O con make:
```bash
make run-barrera
```

**Con parámetros personalizados:**
```bash
.\barrera_test.exe <N_hebras> <N_etapas>
```

Ejemplo con 8 hebras y 5 etapas:
```bash
.\barrera_test.exe 8 5
```

O con make:
```bash
make run-barrera-custom
```

### Verificación

El programa debe mostrar que:
1. Todas las hebras imprimen "esperando en etapa X" antes de que cualquiera imprima "pasó barrera en etapa X"
2. La barrera se reutiliza correctamente en múltiples etapas
3. No hay deadlocks ni condiciones de carrera

---

## Parte II: Simulador de Memoria Virtual

Simulador de paginación con algoritmo de reemplazo Reloj (Clock).

### Archivos

- `sim.c` - Simulador de memoria virtual con traducción de direcciones
- `trace_test.txt` - Archivo de prueba pequeño (10 referencias)
- `trace1.txt` - Archivo de traza para experimentos (tamaño de marco: 8 bytes)
- `trace2.txt` - Archivo de traza para experimentos (tamaño de marco: 4096 bytes)

### Compilación

```bash
make sim
```

O compilar manualmente:
```bash
gcc -Wall -Wextra -std=c99 -o sim sim.c
```

### Uso

```bash
.\sim.exe <Nmarcos> <tamaño_marco> [--verbose] <archivo_traza>
```

**Parámetros:**
- `Nmarcos`: Número de marcos físicos disponibles
- `tamaño_marco`: Tamaño del marco en bytes (debe ser potencia de 2)
- `--verbose`: (Opcional) Muestra detalles de cada traducción
- `archivo_traza`: Archivo con direcciones virtuales (una por línea)

### Ejemplos de Ejecución

**Prueba básica:**
```bash
.\sim.exe 4 4096 trace_test.txt
```

**Con salida detallada:**
```bash
.\sim.exe 4 4096 --verbose trace_test.txt
```

O con make:
```bash
make run-sim-test
make run-sim-test-verbose
```

### Experimentos

**Para trace1.txt (tamaño de marco: 8 bytes):**
```bash
.\sim.exe 8 8 trace1.txt
.\sim.exe 16 8 trace1.txt
.\sim.exe 32 8 trace1.txt
```

O con make:
```bash
make run-trace1-8
make run-trace1-16
make run-trace1-32
```

**Para trace2.txt (tamaño de marco: 4096 bytes):**
```bash
.\sim.exe 8 4096 trace2.txt
.\sim.exe 16 4096 trace2.txt
.\sim.exe 32 4096 trace2.txt
```

O con make:
```bash
make run-trace2-8
make run-trace2-16
make run-trace2-32
```

**Ejecutar todos los experimentos:**
```bash
make run-all-experiments
```

### Formato de Archivo de Traza

El archivo de traza debe contener una dirección virtual por línea. Se aceptan:
- Direcciones en decimal: `4096`
- Direcciones en hexadecimal: `0x1000`
- Líneas vacías y comentarios (líneas que empiezan con `#`) son ignorados

Ejemplo:
```
0x1000
4096
0x2000
8192
```

### Salida

**Modo normal:**
```
========================================
  SIMULADOR DE MEMORIA VIRTUAL
========================================
Número de marcos:     8
Tamaño de marco:      4096 bytes
Algoritmo:            Reloj
Archivo de traza:     trace2.txt
Modo verbose:         No
========================================

========================================
  ESTADÍSTICAS DEL SIMULADOR
========================================
Referencias totales:  100
Fallos de página:     25
Tasa de fallos:       25.00%
========================================
```

**Modo verbose (incluye cada traducción):**
```
DV=0x1000, npv=1, offset=0, FALLO, marco=0, DF=0x0
DV=0x2000, npv=2, offset=0, FALLO, marco=1, DF=0x1000
DV=0x1000, npv=1, offset=0, HIT, marco=0, DF=0x0
...
```

---

## Compilación de Todo el Proyecto

Para compilar ambas partes:
```bash
make all
```

## Limpieza

Para eliminar archivos compilados:
```bash
make clean
```

---

## Requisitos

- Compilador GCC con soporte para C99
- Biblioteca pthread (incluida en MinGW-w64 en Windows)
- Windows con PowerShell (para usar el Makefile proporcionado)

**Nota para Linux:** El Makefile está configurado para Windows. Para Linux, reemplaza:
- `del /Q` con `rm -f`
- `.exe` con extensión vacía

---

## Estructura del Proyecto

```
Tarea 2 SO/
├── barrera.h           # Interfaz de la barrera
├── barrera.c           # Implementación de la barrera
├── main.c              # Verificación de la barrera
├── sim.c               # Simulador de memoria virtual
├── trace_test.txt      # Traza de prueba pequeña
├── trace1.txt          # Traza para experimentos (8 bytes)
├── trace2.txt          # Traza para experimentos (4096 bytes)
├── Makefile            # Sistema de construcción
└── README.md           # Este archivo
```

---

## Algoritmos Implementados

### Barrera Reutilizable
- Usa un **monitor** con mutex y variable de condición
- La última hebra en llegar:
  1. Incrementa el contador de etapa
  2. Resetea el contador de hebras
  3. Despierta a todas las hebras con `pthread_cond_broadcast()`
- Las demás hebras esperan con `pthread_cond_wait()` hasta que cambie la etapa

### Algoritmo del Reloj (Clock)
- Mantiene un puntero que recorre los marcos de forma circular
- Cada página tiene un bit de referencia
- Al buscar víctima:
  - Si `ref_bit = 0`: seleccionar como víctima
  - Si `ref_bit = 1`: dar segunda oportunidad (poner `ref_bit = 0`) y avanzar
- Cuando se accede a una página, se pone `ref_bit = 1`

---

## Análisis de Resultados

### Parte I: Barrera
- **Verificar** que ninguna hebra pase a la siguiente etapa antes de que todas lleguen
- **Observar** el comportamiento de sincronización en las impresiones

### Parte II: Simulador
- **Comparar** la tasa de fallos con diferentes números de marcos (8, 16, 32)
- **Esperar** que la tasa de fallos disminuya al aumentar el número de marcos
- **Analizar** el comportamiento del algoritmo del Reloj vs otros algoritmos

---

## Autor

Tarea 2 - Sistemas Operativos  
Universidad de Concepción 2025
