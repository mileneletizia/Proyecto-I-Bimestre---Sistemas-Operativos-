Sistema Concurrente de Transacciones Bancarias en C
Procesos vs Hilos — Comparación de Rendimiento

Descripción del Proyecto

Este repositorio implementa un sistema de simulación bancaria que procesa transacciones concurrentes y asegura la consistencia del saldo mediante sincronización con semáforos.

Se desarrollan dos enfoques diferentes:

🔹 Versión con Procesos: utilizando fork() y semáforos POSIX
🔹 Versión con Hilos: utilizando pthread y sincronización equivalente

Ambas versiones trabajan con el mismo conjunto de transacciones para poder comparar su rendimiento de forma objetiva.

⚠️ Problemática

Los bancos reales procesan miles de operaciones al mismo tiempo.
Sin un control adecuado, podrían ocurrir:

Condiciones de carrera

Saldos corruptos

Resultados inconsistentes

Este proyecto demuestra cómo manejar correctamente la concurrencia utilizando IPC (Inter-Process Communication) y multithreading.

📂 Estructura del Repositorio
├── procesos.c
├── hilos.c
├── Transacciones.txt
├── TiempoProcesos.txt   (generado al ejecutar)
├── TiempoHilos.txt      (generado al ejecutar)
└── README.md

🧰 Requisitos

El proyecto está pensado para ejecutarse en Ubuntu/Linux.

Instalar dependencias:

sudo apt update
sudo apt install build-essential


Requisitos adicionales:

GCC

Librería pthread

Librería rt (para semáforos en procesos)

Soporte de semáforos POSIX

🔧 Compilación
✔️ Compilar versión basada en procesos
gcc -o procesos procesos.c -pthread -lrt

✔️ Compilar versión basada en hilos
gcc -o hilos hilos.c -pthread

📄 Archivos necesarios

Asegúrate de tener en el mismo directorio:

procesos.c

hilos.c

Transacciones.txt

Si el archivo se descargó con doble extensión:

mv Transacciones.txt.txt Transacciones.txt

🚀 Ejecución
./procesos
./hilos

📑 Resultados

Cada implementación genera un archivo:

Archivo	Descripción
TiempoProcesos.txt	Resultados de la versión con fork()
TiempoHilos.txt	Resultados de la versión con pthread

Estos archivos incluyen:

✔️ Saldo final después de aplicar todas las transacciones

✔️ Tiempo total de ejecución

✔️ Registro de cada operación procesada

🔍 Verificación

Deberías obtener:

El mismo saldo final en ambos programas

Tiempos diferentes (los hilos suelen ser más rápidos)

Variaciones en el rendimiento dependen del hardware y carga del sistema.

📊 Objetivo del Proyecto

Evaluar:

Diferencias prácticas entre procesos e hilos

Costos de creación y sincronización

Comportamiento bajo carga concurrente realista

📝 Licencia

Este proyecto es de uso académico. Puedes modificarlo y adaptarlo libremente.
