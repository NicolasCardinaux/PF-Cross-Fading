#include <iostream>
#include <vector>
#include <string>
#include <omp.h>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

unsigned char to_grayscale(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
}

int main() {
    const int width = 7952;
    const int height = 5304;
    const char* input_filename = "input_7952x5304.png";
    const int NUM_THREADS = 12; // Tus 12 hilos
    
    int w, h, channels;
    unsigned char *image_color = stbi_load(input_filename, &w, &h, &channels, 3);
    if (!image_color || w != width || h != height) {
        std::cerr << "Error cargando la imagen." << std::endl;
        return 1;
    }
    channels = 3;
    omp_set_num_threads(NUM_THREADS);

    std::cout << "Iniciando OpenMP OPTIMIZADO (" << NUM_THREADS << " hilos) para " << width << "x" << height << std::endl;
    double start_time = omp_get_wtime();

    size_t img_size = width * height * channels;
    unsigned char *image_gray = (unsigned char*)malloc(img_size);
    // ... (manejo de error) ...

    // === 2. CREAR VERSIÓN GRIS (Sigue igual) ===
    #pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            size_t idx = (y * width + x) * channels;
            unsigned char r = image_color[idx + 0];
            unsigned char g = image_color[idx + 1];
            unsigned char b = image_color[idx + 2];
            unsigned char gray = to_grayscale(r, g, b);
            image_gray[idx + 0] = gray;
            image_gray[idx + 1] = gray;
            image_gray[idx + 2] = gray;
        }
    }

    // === 3. GENERAR FRAMES (¡AHORA PARALELIZADO POR FRAME!) ===
    const int total_frames = 96;

    // ¡AQUÍ ESTÁ EL CAMBIO!
    // El bucle de frames ahora se divide entre los 12 hilos.
    #pragma omp parallel for
    for (int frame = 0; frame < total_frames; ++frame) {
        
        // Estas variables ahora son "privadas" para cada hilo
        double P = 1.0 - (double)frame / (double)(total_frames - 1);
        unsigned char *image_result = (unsigned char*)malloc(img_size);
        
        if (!image_result) {
             std::cerr << "Error de memoria en hilo para frame " << frame << std::endl;
             // 'continue' ahora salta este frame para este hilo específico
             continue; 
        }

        // Este bucle 'y' AHORA ES SECUENCIAL (para este hilo)
        // Cada hilo hace el cálculo completo de su frame asignado
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                size_t idx = (y * width + x) * channels;
                image_result[idx + 0] = (unsigned char)(image_color[idx + 0] * P + image_gray[idx + 0] * (1.0 - P));
                image_result[idx + 1] = (unsigned char)(image_color[idx + 1] * P + image_gray[idx + 1] * (1.0 - P));
                image_result[idx + 2] = (unsigned char)(image_color[idx + 2] * P + image_gray[idx + 2] * (1.0 - P));
            }
        }

        // ¡EL GUARDADO AHORA OCURRE EN PARALELO!
        // Cada hilo guarda su propio archivo
        char filename[100];
        sprintf(filename, "output_omp_opt/frame_%03d.png", frame); // Guarda en la nueva carpeta
        stbi_write_png(filename, width, height, channels, image_result, width * channels);
        free(image_result);
    } // --- Fin del #pragma omp parallel for ---

    double end_time = omp_get_wtime();
    std::cout << "Tiempo OpenMP OPTIMIZADO: " << (end_time - start_time) << " segundos" << std::endl;

    stbi_image_free(image_color);
    free(image_gray);

    std::cout << "Proceso OpenMP OPTIMIZADO completado." << std::endl;
    return 0;
}