#ifndef BARRERA_H
#define BARRERA_H

#include <pthread.h>

/**
 * Estructura que representa una barrera reutilizable
 * Una barrera sincroniza N hebras en puntos de encuentro
 */
typedef struct {
    int N;                    // Número total de hebras que deben sincronizarse
    int count;                // Número de hebras que han llegado en la etapa actual
    int etapa;                // Identificador de la etapa actual
    pthread_mutex_t mutex;    // Mutex para exclusión mutua del estado interno
    pthread_cond_t cond;      // Variable de condición para espera
} barrera_t;

/**
 * Inicializa una barrera para sincronizar N hebras
 * @param barrera Puntero a la estructura barrera_t
 * @param N Número de hebras que se sincronizarán
 * @return 0 en éxito, -1 en error
 */
int barrera_init(barrera_t *barrera, int N);

/**
 * Destruye una barrera y libera sus recursos
 * @param barrera Puntero a la estructura barrera_t
 * @return 0 en éxito, -1 en error
 */
int barrera_destroy(barrera_t *barrera);

/**
 * Operación de espera en la barrera
 * Bloquea la hebra hasta que todas las N hebras hayan llegado
 * La última hebra en llegar despierta a todas las demás
 * @param barrera Puntero a la estructura barrera_t
 * @return 0 en éxito, -1 en error
 */
int barrera_wait(barrera_t *barrera);

#endif // BARRERA_H
