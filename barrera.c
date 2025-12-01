#include "barrera.h"
#include <stdio.h>
#include <errno.h>

//Inicializa una barrera reutilizable para N hebras
 
int barrera_init(barrera_t *barrera, int N) {
    if (barrera == NULL || N <= 0) {
        return -1;
    }
    barrera->N = N;
    barrera->count = 0;
    barrera->etapa = 0;

    // Inicializar mutex
    if (pthread_mutex_init(&barrera->mutex, NULL) != 0) {
        perror("Error al inicializar mutex");
        return -1;
    }
    // Inicializar variable de condición
    if (pthread_cond_init(&barrera->cond, NULL) != 0) {
        perror("Error al inicializar variable de condición");
        pthread_mutex_destroy(&barrera->mutex);
        return -1;
    }
    return 0;
}

//Destruye la barrera
int barrera_destroy(barrera_t *barrera) {
    if (barrera == NULL) {
        return -1;
    }

    // Destruir mutex y variable de condición
    int ret1 = pthread_mutex_destroy(&barrera->mutex);
    int ret2 = pthread_cond_destroy(&barrera->cond);

    if (ret1 != 0 || ret2 != 0) {
        perror("Error al destruir barrera");
        return -1;
    }
    return 0;
}

/*
 Operación de espera en la barrera
 Implementa el patrón: lock → modificar estado / decidir → wait/broadcast → unlock
 */
int barrera_wait(barrera_t *barrera) {
    if (barrera == NULL) {
        return -1;
    }

    // Adquirir exclusión mutua
    pthread_mutex_lock(&barrera->mutex);

    // Capturar la etapa actual en variable local
    int etapa_local = barrera->etapa;

    // Incrementar contador de hebras que han llegado
    barrera->count++;

    // Verificar si es la ultima hebra en llegar
    if (barrera->count == barrera->N) {
        barrera->etapa++; //Incrementar etapa
        barrera->count = 0; //Resetear contador para la siguiente etapa
        pthread_cond_broadcast(&barrera->cond);  //Despertar hebras en espera 
    } else {
        // Esperar mientras la etapa no haya cambiado
        while (etapa_local == barrera->etapa) {
            pthread_cond_wait(&barrera->cond, &barrera->mutex);
        }
    }

    // Liberar exclusión mutua
    pthread_mutex_unlock(&barrera->mutex);

    return 0;
}
