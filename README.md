Aquí tienes el `README.md` final y unificado, que utiliza los nombres de archivo exactos de tu proyecto (`openmp-datoslento.cpp`, `openmp.cpp`, etc.) para que las instrucciones de compilación y ejecución sean correctas.

-----

# Práctico Final: Paralelización de Cross-Fading de Imágenes

Este proyecto es la resolución del Práctico Final Integrador para la cátedra de **Computación Avanzada**. El objetivo es implementar y analizar el rendimiento de un algoritmo de *cross-fading* de imágenes, comparando una implementación secuencial con dos estrategias de paralelización (Paralelismo de Datos y División de Tareas) utilizando OpenMP y MPI.

El análisis demostró que el problema es **fuertemente I/O-Bound** (limitado por el disco), y que la estrategia de **División de Tareas** (usada en `openmp.cpp` y `mpi.cpp`) fue la única que logró un escalado de rendimiento exitoso.

## 🛠️ Prerrequisitos (Entorno de Windows)

1.  **Compilador C++ (g++):** **MinGW-w64 (de WinLibs)** con soporte para **hilos POSIX** (necesario para OpenMP).
2.  **Microsoft MPI (MS-MPI):** Se debe instalar tanto el **Runtime (`msmpisetup.exe`)** como el **SDK (`msmpisdk.msi`)**.
3.  **Bibliotecas `stb_image`:** `stb_image.h` y `stb_image_write.h` deben estar en la misma carpeta que los archivos `.cpp`.

## 📁 Estructura del Proyecto

  * `sequential.cpp`: Implementación secuencial base.
  * `openmp-datoslento.cpp`: **Estrategia 1 (Paralelismo de Datos)**. Versión lenta que solo paraleliza el cálculo de píxeles.
  * `openmp.cpp`: **Estrategia 2 (División de Tareas)**. Versión rápida y optimizada que paraleliza el bucle de *frames*.
  * `mpi.cpp`: **Estrategia 2 (División de Tareas)**. Implementación con MPI.
  * `stb_image.h`, `stb_image_write.h`: Bibliotecas de imagen.
  * `input_...png`: Imágenes de entrada.
  * `output_seq/`: Carpeta de salida para el secuencial.
  * `output_omp/`: Carpeta de salida para `openmp-datoslento.cpp`.
  * `output_omp_opt/`: Carpeta de salida para `openmp.cpp`.
  * `output_mpi/`: Carpeta de salida para `mpi.cpp`.

## 🚀 Compilación y Ejecución

**¡IMPORTANTE\!** Antes de compilar, asegúrate de que las variables `width`, `height` y `input_filename` dentro de cada archivo `.cpp` coincidan con la imagen de entrada que quieres probar.

### 1\. Preparar Carpetas de Salida

(Solo necesitas hacerlo una vez)

```bash
mkdir output_seq
mkdir output_omp
mkdir output_omp_opt
mkdir output_mpi
```

### 2\. Secuencial

```bash
# Compilar (Crea sequential.exe)
g++ sequential.cpp -o sequential -O3 -std=c++17

# Ejecutar
./sequential
```

### 3\. OpenMP (Estrategia 1 - Paralelismo de Datos)

Esta es la versión lenta (`openmp-datoslento.cpp`) que demuestra el cuello de botella de I/O.

```bash
# Compilar (Crea openmp-datoslento.exe)
g++ openmp-datoslento.cpp -o openmp-datoslento -O3 -std=c++17 -fopenmp

# Ejecutar
./openmp-datoslento
```

### 4\. OpenMP (Estrategia 2 - División de Tareas)

Esta es la versión rápida y optimizada (`openmp.cpp`).

```bash
# Compilar (Crea openmp.exe)
g++ openmp.cpp -o openmp -O3 -std=c++17 -fopenmp

# Ejecutar
./openmp
```

### 5\. MPI (Estrategia 2 - División de Tareas)

Esta implementación requiere un comando de compilación especial para enlazar las bibliotecas de MS-MPI.

```bash
# Compilar (Crea mpi.exe)
g++ mpi.cpp -o mpi -O3 -std=c++17 -I"C:\Program Files (x86)\Microsoft SDKs\MPI\Include" -L"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" -lmsmpi

# Ejecutar (Ejemplo con 12 procesos)
mpiexec -n 12 ./mpi
```
