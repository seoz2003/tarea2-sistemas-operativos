#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "barrera.h"

// Estructura para pasar parámetros a las hebras
typedef struct {
    int tid;              // ID de la hebra
    int N;                // Número total de hebras
    int E;                // Número de etapas
    barrera_t *barrera;   // Puntero a la barrera compartida
} thread_args_t;

/**
 * Función que ejecuta cada hebra
 * Simula trabajo y sincroniza en cada etapa usando la barrera
 */
void* thread_function(void* arg) {
    thread_args_t *args = (thread_args_t*)arg;
    int tid = args->tid;
    int E = args->E;
    barrera_t *barrera = args->barrera;

    // Ejecutar E etapas
    for (int etapa = 0; etapa < E; etapa++) {
        // a) Simular trabajo con tiempo aleatorio (50-150 ms)
        int trabajo_us = 50000 + (rand() % 100000);
        usleep(trabajo_us);

        // b) Imprimir antes de esperar en la barrera
        printf("[Hebra %d] esperando en etapa %d\n", tid, etapa);
        fflush(stdout);

        // c) Llamar a wait() - sincronización en la barrera
        barrera_wait(barrera);

        // d) Imprimir después de pasar la barrera
        printf("[Hebra %d] pasó barrera en etapa %d\n", tid, etapa);
        fflush(stdout);

        // Pequeña pausa para visualizar mejor la sincronización
        usleep(10000);
    }

    return NULL;
}

/**
 * Programa principal que verifica el comportamiento de la barrera
 */
int main(int argc, char *argv[]) {
    int N = 5;  // Número de hebras por defecto
    int E = 4;  // Número de etapas por defecto

    // Leer parámetros de línea de comandos
    if (argc >= 2) {
        N = atoi(argv[1]);
        if (N <= 0) {
            fprintf(stderr, "Error: El número de hebras debe ser positivo\n");
            return 1;
        }
    }
    
    if (argc >= 3) {
        E = atoi(argv[2]);
        if (E <= 0) {
            fprintf(stderr, "Error: El número de etapas debe ser positivo\n");
            return 1;
        }
    }

    printf("========================================\n");
    printf("  VERIFICACIÓN DE BARRERA REUTILIZABLE\n");
    printf("========================================\n");
    printf("Número de hebras: %d\n", N);
    printf("Número de etapas: %d\n", E);
    printf("========================================\n\n");

    // Inicializar semilla para números aleatorios
    srand(time(NULL));

    // Inicializar la barrera
    barrera_t barrera;
    if (barrera_init(&barrera, N) != 0) {
        fprintf(stderr, "Error al inicializar la barrera\n");
        return 1;
    }

    // Crear arreglos para hebras y argumentos
    pthread_t *threads = malloc(N * sizeof(pthread_t));
    thread_args_t *args = malloc(N * sizeof(thread_args_t));

    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Error al asignar memoria\n");
        barrera_destroy(&barrera);
        return 1;
    }

    // Crear N hebras
    for (int i = 0; i < N; i++) {
        args[i].tid = i;
        args[i].N = N;
        args[i].E = E;
        args[i].barrera = &barrera;

        if (pthread_create(&threads[i], NULL, thread_function, &args[i]) != 0) {
            fprintf(stderr, "Error al crear hebra %d\n", i);
            barrera_destroy(&barrera);
            free(threads);
            free(args);
            return 1;
        }
    }

    // Esperar a que todas las hebras terminen
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n========================================\n");
    printf("  VERIFICACIÓN COMPLETADA\n");
    printf("========================================\n");
    printf("Todas las hebras completaron las %d etapas exitosamente.\n", E);
    printf("La barrera se reutilizó correctamente en cada etapa.\n");
    printf("========================================\n");

    // Limpiar recursos
    barrera_destroy(&barrera);
    free(threads);
    free(args);

    return 0;
}
