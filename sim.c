#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_PAGES 1000000
#define MAX_LINE 256

/**
 * Entrada en la tabla de páginas
 */
typedef struct {
    int valid;           // 1 si la página está en memoria, 0 si no
    int marco;           // Número de marco físico asignado
    int ref_bit;         // Bit de referencia para algoritmo del Reloj
} page_table_entry_t;

/**
 * Estructura del simulador
 */
typedef struct {
    int num_marcos;                           // Número total de marcos disponibles
    int page_size;                            // Tamaño del marco/página en bytes
    int page_bits;                            // Bits para desplazamiento (offset)
    uint64_t mask;                            // Máscara para extraer offset
    
    page_table_entry_t *page_table;           // Tabla de páginas
    int *marcos;                              // Mapeo: marco -> npv
    int clock_hand;                           // Puntero del reloj
    
    // Estadísticas
    long long referencias;                    // Número total de referencias
    long long fallos;                         // Número de fallos de página
    
    int verbose;                              // Modo verbose
} simulator_t;

/**
 * Calcula log2 de un número (para obtener bits de offset)
 */
int log2_int(int n) {
    int bits = 0;
    while (n > 1) {
        n >>= 1;
        bits++;
    }
    return bits;
}

/**
 * Inicializa el simulador
 */
simulator_t* sim_init(int num_marcos, int page_size, int verbose) {
    simulator_t *sim = malloc(sizeof(simulator_t));
    if (!sim) {
        perror("Error al asignar memoria para simulador");
        return NULL;
    }
    
    sim->num_marcos = num_marcos;
    sim->page_size = page_size;
    sim->page_bits = log2_int(page_size);
    sim->mask = (1ULL << sim->page_bits) - 1;
    sim->clock_hand = 0;
    sim->referencias = 0;
    sim->fallos = 0;
    sim->verbose = verbose;
    
    // Inicializar tabla de páginas (suficientemente grande)
    sim->page_table = calloc(MAX_PAGES, sizeof(page_table_entry_t));
    if (!sim->page_table) {
        perror("Error al asignar tabla de páginas");
        free(sim);
        return NULL;
    }
    
    // Inicializar mapeo de marcos (-1 = libre)
    sim->marcos = malloc(num_marcos * sizeof(int));
    if (!sim->marcos) {
        perror("Error al asignar array de marcos");
        free(sim->page_table);
        free(sim);
        return NULL;
    }
    
    for (int i = 0; i < num_marcos; i++) {
        sim->marcos[i] = -1;  // Marco libre
    }
    
    return sim;
}

/**
 * Libera recursos del simulador
 */
void sim_destroy(simulator_t *sim) {
    if (sim) {
        free(sim->page_table);
        free(sim->marcos);
        free(sim);
    }
}

/**
 * Encuentra un marco libre o devuelve -1 si no hay
 */
int find_free_frame(simulator_t *sim) {
    for (int i = 0; i < sim->num_marcos; i++) {
        if (sim->marcos[i] == -1) {
            return i;
        }
    }
    return -1;
}

/**
 * Algoritmo del Reloj para encontrar víctima
 * Busca una página con ref_bit = 0, dando una segunda oportunidad a las que tienen ref_bit = 1
 */
int clock_evict(simulator_t *sim) {
    while (1) {
        int marco = sim->clock_hand;
        int npv = sim->marcos[marco];
        
        if (npv == -1) {
            // Marco libre (no debería pasar si se llama correctamente)
            int victim = sim->clock_hand;
            sim->clock_hand = (sim->clock_hand + 1) % sim->num_marcos;
            return victim;
        }
        
        // Revisar bit de referencia
        if (sim->page_table[npv].ref_bit == 0) {
            // Víctima encontrada
            int victim = sim->clock_hand;
            sim->clock_hand = (sim->clock_hand + 1) % sim->num_marcos;
            return victim;
        } else {
            // Dar segunda oportunidad: poner ref_bit = 0
            sim->page_table[npv].ref_bit = 0;
            sim->clock_hand = (sim->clock_hand + 1) % sim->num_marcos;
        }
    }
}

/**
 * Procesa una dirección virtual
 */
void process_address(simulator_t *sim, uint64_t vaddr) {
    sim->referencias++;
    
    // Descomponer dirección virtual
    uint64_t offset = vaddr & sim->mask;
    uint64_t npv = vaddr >> sim->page_bits;
    
    int hit = 0;
    int marco = -1;
    uint64_t paddr = 0;
    
    // Verificar si la página está en memoria
    if (sim->page_table[npv].valid) {
        // HIT
        hit = 1;
        marco = sim->page_table[npv].marco;
        sim->page_table[npv].ref_bit = 1;  // Marcar como referenciada
    } else {
        // FALLO
        sim->fallos++;
        
        // Buscar marco libre
        marco = find_free_frame(sim);
        
        if (marco == -1) {
            // No hay marco libre, usar algoritmo del Reloj
            marco = clock_evict(sim);
            
            // Invalidar la página que estaba en este marco
            int old_npv = sim->marcos[marco];
            if (old_npv != -1) {
                sim->page_table[old_npv].valid = 0;
            }
        }
        
        // Asignar marco a la nueva página
        sim->marcos[marco] = npv;
        sim->page_table[npv].valid = 1;
        sim->page_table[npv].marco = marco;
        sim->page_table[npv].ref_bit = 1;
    }
    
    // Calcular dirección física
    paddr = ((uint64_t)marco << sim->page_bits) | offset;
    
    // Imprimir en modo verbose
    if (sim->verbose) {
        printf("DV=0x%llx, npv=%llu, offset=%llu, %s, marco=%d, DF=0x%llx\n",
               (unsigned long long)vaddr,
               (unsigned long long)npv,
               (unsigned long long)offset,
               hit ? "HIT" : "FALLO",
               marco,
               (unsigned long long)paddr);
    }
}

/**
 * Procesa archivo de trazas
 */
void process_trace_file(simulator_t *sim, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo %s\n", filename);
        return;
    }
    
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        // Eliminar espacios y saltos de línea
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\n' || *p == '\0' || *p == '#') continue;
        
        uint64_t vaddr;
        
        // Leer en decimal o hexadecimal
        if (strncmp(p, "0x", 2) == 0 || strncmp(p, "0X", 2) == 0) {
            sscanf(p, "%llx", (unsigned long long*)&vaddr);
        } else {
            sscanf(p, "%llu", (unsigned long long*)&vaddr);
        }
        
        process_address(sim, vaddr);
    }
    
    fclose(file);
}

/**
 * Imprime estadísticas finales
 */
void print_statistics(simulator_t *sim) {
    printf("\n========================================\n");
    printf("  ESTADÍSTICAS DEL SIMULADOR\n");
    printf("========================================\n");
    printf("Referencias totales:  %lld\n", sim->referencias);
    printf("Fallos de página:     %lld\n", sim->fallos);
    
    if (sim->referencias > 0) {
        double tasa = (double)sim->fallos / sim->referencias * 100.0;
        printf("Tasa de fallos:       %.2f%%\n", tasa);
    }
    
    printf("========================================\n");
}

/**
 * Programa principal
 */
int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <Nmarcos> <tamañomarco> [--verbose] <archivo_traza>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s 8 4096 traza.txt\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s 8 4096 --verbose traza.txt\n", argv[0]);
        return 1;
    }
    
    int num_marcos = atoi(argv[1]);
    int page_size = atoi(argv[2]);
    int verbose = 0;
    char *trace_file = NULL;
    
    // Verificar si hay flag --verbose
    if (argc == 5 && strcmp(argv[3], "--verbose") == 0) {
        verbose = 1;
        trace_file = argv[4];
    } else if (argc == 4) {
        trace_file = argv[3];
    } else {
        fprintf(stderr, "Error: Argumentos inválidos\n");
        return 1;
    }
    
    // Validar parámetros
    if (num_marcos <= 0 || page_size <= 0) {
        fprintf(stderr, "Error: Los parámetros deben ser positivos\n");
        return 1;
    }
    
    // Verificar que page_size es potencia de 2
    if ((page_size & (page_size - 1)) != 0) {
        fprintf(stderr, "Error: El tamaño del marco debe ser potencia de 2\n");
        return 1;
    }
    
    printf("========================================\n");
    printf("  SIMULADOR DE MEMORIA VIRTUAL\n");
    printf("========================================\n");
    printf("Número de marcos:     %d\n", num_marcos);
    printf("Tamaño de marco:      %d bytes\n", page_size);
    printf("Algoritmo:            Reloj\n");
    printf("Archivo de traza:     %s\n", trace_file);
    printf("Modo verbose:         %s\n", verbose ? "Sí" : "No");
    printf("========================================\n\n");
    
    // Inicializar simulador
    simulator_t *sim = sim_init(num_marcos, page_size, verbose);
    if (!sim) {
        return 1;
    }
    
    // Procesar archivo de trazas
    process_trace_file(sim, trace_file);
    
    // Imprimir estadísticas
    print_statistics(sim);
    
    // Limpiar
    sim_destroy(sim);
    
    return 0;
}
