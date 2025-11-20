#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

#define CANT_PROCESOS 8
#define TRANS_POR_PROCESO 5
#define ARCH_LOG "log_alt_procesos.txt"
#define SEMAFORO_NOMBRE "/sem_control_saldo"

// Función para escribir en el archivo de log
void anadir_log(int fd, const char *msg) {
    write(fd, msg, strlen(msg));
}

// Función ejecutada por cada proceso
void ejecutar_operaciones(char etiqueta, sem_t *sem, int fd_log, int *saldo) {
    char linea[256];

    snprintf(linea, sizeof(linea),
             ">>> Proceso %c: iniciando operaciones...\n",
             etiqueta);
    anadir_log(fd_log, linea);

    for (int i = 0; i < TRANS_POR_PROCESO; i++) {

        int mov = (rand() % 201) - 100; // -100 a +100

        sem_wait(sem);     // bloquear saldo
        *saldo += mov;
        int actual = *saldo;
        sem_post(sem);     // liberar saldo

        snprintf(linea, sizeof(linea),
                 "Proceso %c | Operación %d | Cambio: %+d | Saldo parcial: %d\n",
                 etiqueta, i + 1, mov, actual);
        anadir_log(fd_log, linea);
    }

    snprintf(linea, sizeof(linea),
             "<<< Proceso %c finalizó.\n\n",
             etiqueta);
    anadir_log(fd_log, linea);
}

int main() {
    srand(time(NULL));

    // --- Medir tiempo ---
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    // --- Abrir archivo log ---
    int fd = open(ARCH_LOG, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Error al crear log");
        exit(1);
    }

    // --- Crear memoria compartida para el saldo ---
    int *saldo = mmap(NULL, sizeof(int),
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS,
                      -1, 0);

    if (saldo == MAP_FAILED) {
        perror("Error en mmap");
        exit(1);
    }

    *saldo = 1000;

    // --- Crear semáforo ---
    sem_t *sem = sem_open(SEMAFORO_NOMBRE, O_CREAT | O_EXCL, 0600, 1);
    if (sem == SEM_FAILED) {
        perror("Error creando semáforo");
        exit(1);
    }

    // --- Crear procesos ---
    for (int i = 0; i < CANT_PROCESOS; i++) {
        pid_t p = fork();

        if (p < 0) {
            perror("Error en fork");
            exit(1);
        }
        else if (p == 0) {
            // Hijo
            char grupo = 'A' + i;
            ejecutar_operaciones(grupo, sem, fd, saldo);

            close(fd);
            sem_close(sem);
            exit(0);
        }
    }

    // --- Esperar a todos los hijos ---
    for (int i = 0; i < CANT_PROCESOS; i++) {
        wait(NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);

    double tiempo = (t1.tv_sec - t0.tv_sec)
                  + (t1.tv_nsec - t0.tv_nsec) / 1e9;

    // --- Escribir resumen ---
    char resumen[200];
    snprintf(resumen, sizeof(resumen),
             "\nSaldo final: %d\nTiempo total: %.5f segundos\n",
             *saldo, tiempo);
    write(fd, resumen, strlen(resumen));

    // --- Cerrar recursos ---
    close(fd);
    sem_close(sem);
    sem_unlink(SEMAFORO_NOMBRE);
    munmap(saldo, sizeof(int));

    printf("Ejecutado correctamente. Revisa '%s'\n", ARCH_LOG);
    return 0;
}
