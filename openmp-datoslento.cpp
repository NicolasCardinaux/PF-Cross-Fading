#include <iostream>
#include <vector>
#include <string>
#include <omp.h> // <-- 1. Incluir OpenMP
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
    
    int w, h, channels;
    unsigned char *image_color = stbi_load(input_filename, &w, &h, &channels, 3);
    if (!image_color || w != width || h != height) {
        std::cerr << "Error cargando la imagen." << std::endl;
        return 1;
    }
    channels = 3;

    // <-- 2. Configurar número de hilos
    const int NUM_THREADS = 12; // Prueba con 2, 4, 8, etc.
    omp_set_num_threads(NUM_THREADS);

    std::cout << "Iniciando OpenMP (" << NUM_THREADS << " hilos) para " << width << "x" << height << std::endl;
    double start_time = omp_get_wtime(); // <-- Usar timer de OpenMP

    size_t img_size = width * height * channels;
    unsigned char *image_gray = (unsigned char*)malloc(img_size);
    // ... (manejo de error) ...

    // 1. Crear versión gris (Paralelizado)
    #pragma omp parallel for // <-- 3. Paralelizar el bucle 'y'
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

    // 2. Generar frames (Paralelizado)
    const int total_frames = 96;
    for (int frame = 0; frame < total_frames; ++frame) {
        double P = 1.0 - (double)frame / (double)(total_frames - 1);
        unsigned char *image_result = (unsigned char*)malloc(img_size);
        // ... (manejo de error) ...
        
        #pragma omp parallel for // <-- 4. Paralelizar el bucle 'y'
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                size_t idx = (y * width + x) * channels;
                image_result[idx + 0] = (unsigned char)(image_color[idx + 0] * P + image_gray[idx + 0] * (1.0 - P));
                image_result[idx + 1] = (unsigned char)(image_color[idx + 1] * P + image_gray[idx + 1] * (1.0 - P));
                image_result[idx + 2] = (unsigned char)(image_color[idx + 2] * P + image_gray[idx + 2] * (1.0 - P));
            }
        }

        char filename[100];
        sprintf(filename, "output_omp/frame_%03d.png", frame); // Guarda en "output_omp"
        stbi_write_png(filename, width, height, channels, image_result, width * channels);
        free(image_result);
    }

    double end_time = omp_get_wtime();
    std::cout << "Tiempo OpenMP: " << (end_time - start_time) << " segundos" << std::endl;

    stbi_image_free(image_color);
    free(image_gray);

    std::cout << "Proceso OpenMP completado." << std::endl;
    return 0;
}
