#include <stdio.h>    
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CANT_GRUPOS             8
#define TRANS_X_GRUPO           5
#define ARCHIVO_LOG             "log_hilos_alt.txt"

// Saldo compartido por todos los hilos
int saldo_compartido = 1000;
pthread_mutex_t mtx_saldo;

// Estructura para pasar datos a cada hilo
typedef struct {
    char id_grupo;
    int fd_log;
} InfoHilo;

// Función auxiliar para escribir en el log
void escribir_log(int fd, const char *mensaje) {
    write(fd, mensaje, strlen(mensaje));
}

// Función que ejecutará cada hilo
void *rutina_transacciones(void *arg) {
    InfoHilo *info = (InfoHilo *)arg;
    char buffer[256];
    int cambio;

    // Mensaje de inicio
    snprintf(buffer, sizeof(buffer),
             "Grupo %c: iniciando transacciones...\n",
             info->id_grupo);
    escribir_log(info->fd_log, buffer);

    for (int i = 0; i < TRANS_X_GRUPO; i++) {
        // Valor aleatorio entre -100 y +100
        cambio = (rand() % 201) - 100;

        // Bloquear el acceso al saldo mientras se actualiza
        pthread_mutex_lock(&mtx_saldo);
        saldo_compartido += cambio;
        int saldo_actual = saldo_compartido;
        pthread_mutex_unlock(&mtx_saldo);

        snprintf(buffer, sizeof(buffer),
                 "Grupo %c -> Transaccion %d: %+d | Saldo parcial: %d\n",
                 info->id_grupo, i + 1, cambio, saldo_actual);
        escribir_log(info->fd_log, buffer);
    }

    snprintf(buffer, sizeof(buffer),
             "Grupo %c: finalizo sus transacciones.\n\n",
             info->id_grupo);
    escribir_log(info->fd_log, buffer);

    pthread_exit(NULL);
}

int main(void) {
    pthread_t hilos[CANT_GRUPOS];
    InfoHilo info[CANT_GRUPOS];
    struct timespec t_inicio, t_fin;

    // Semilla para números aleatorios
    srand((unsigned int)time(NULL));

    // Inicializar mutex
    if (pthread_mutex_init(&mtx_saldo, NULL) != 0) {
        perror("Error al inicializar el mutex");
        return 1;
    }

    // Abrir archivo de log
    int fd_log = open(ARCHIVO_LOG, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd_log < 0) {
        perror("Error al abrir el archivo de log");
        pthread_mutex_destroy(&mtx_saldo);
        return 1;
    }

    // Tiempo de inicio
    clock_gettime(CLOCK_MONOTONIC, &t_inicio);

    // Crear hilos
    for (int i = 0; i < CANT_GRUPOS; i++) {
        info[i].id_grupo = 'A' + i;  // A, B, C, ...
        info[i].fd_log = fd_log;

        if (pthread_create(&hilos[i], NULL, rutina_transacciones, &info[i]) != 0) {
            perror("Error al crear hilo");
            close(fd_log);
            pthread_mutex_destroy(&mtx_saldo);
            return 1;
        }
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < CANT_GRUPOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Tiempo final
    clock_gettime(CLOCK_MONOTONIC, &t_fin);

    double duracion = (t_fin.tv_sec - t_inicio.tv_sec)
                    + (t_fin.tv_nsec - t_inicio.tv_nsec) / 1e9;

    // Escribir resumen final
    char resumen[200];
    int n = snprintf(resumen, sizeof(resumen),
                     "\nSaldo final: %d\nTiempo total de ejecucion: %.4f segundos\n",
                     saldo_compartido, duracion);
    write(fd_log, resumen, (size_t)n);

    // Cerrar y limpiar
    close(fd_log);
    pthread_mutex_destroy(&mtx_saldo);

    printf("Ejecucion completada. Revisa el archivo: %s\n", ARCHIVO_LOG);
    return 0;
}
