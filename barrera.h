#ifndef BARRERA_H
#define BARRERA_H
#include <pthread.h>

//Estructura que representa una barrera reutilizable
typedef struct {
    int N;                    
    int count;                
    int etapa;                
    pthread_mutex_t mutex;    
    pthread_cond_t cond;      
} barrera_t;


int barrera_init(barrera_t *barrera, int N);
int barrera_destroy(barrera_t *barrera);
int barrera_wait(barrera_t *barrera);

#endif // BARRERA_H
