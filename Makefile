CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
LDFLAGS = -pthread

# Targets principales
all: barrera_test sim

# Parte I: Barrera reutilizable
barrera_test: main.o barrera.o
	$(CC) $(LDFLAGS) -o barrera_test main.o barrera.o

main.o: main.c barrera.h
	$(CC) $(CFLAGS) -c main.c

barrera.o: barrera.c barrera.h
	$(CC) $(CFLAGS) -c barrera.c

# Parte II: Simulador de memoria virtual
sim: sim.c
	$(CC) $(CFLAGS) -o sim sim.c

# Limpieza
clean:
	del /Q *.o barrera_test.exe sim.exe 2>nul || echo Limpieza completada

# Ejecuciones de prueba - Parte I
run-barrera:
	.\barrera_test.exe

run-barrera-custom:
	.\barrera_test.exe 8 5

# Ejecuciones de prueba - Parte II
run-sim-test:
	.\sim.exe 4 4096 trace_test.txt

run-sim-test-verbose:
	.\sim.exe 4 4096 --verbose trace_test.txt

# Experimentos con trace1.txt (tamaño marco = 8)
run-trace1-8:
	@echo ========== trace1.txt con 8 marcos ==========
	.\sim.exe 8 8 trace1.txt

run-trace1-16:
	@echo ========== trace1.txt con 16 marcos ==========
	.\sim.exe 16 8 trace1.txt

run-trace1-32:
	@echo ========== trace1.txt con 32 marcos ==========
	.\sim.exe 32 8 trace1.txt

# Experimentos con trace2.txt (tamaño marco = 4096)
run-trace2-8:
	@echo ========== trace2.txt con 8 marcos ==========
	.\sim.exe 8 4096 trace2.txt

run-trace2-16:
	@echo ========== trace2.txt con 16 marcos ==========
	.\sim.exe 16 4096 trace2.txt

run-trace2-32:
	@echo ========== trace2.txt con 32 marcos ==========
	.\sim.exe 32 4096 trace2.txt

# Ejecutar todos los experimentos
run-all-experiments: run-trace1-8 run-trace1-16 run-trace1-32 run-trace2-8 run-trace2-16 run-trace2-32

.PHONY: all clean run-barrera run-barrera-custom run-sim-test run-sim-test-verbose \
        run-trace1-8 run-trace1-16 run-trace1-32 \
        run-trace2-8 run-trace2-16 run-trace2-32 \
        run-all-experiments
