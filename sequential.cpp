#include <iostream>
#include <vector>
#include <string>
#include <chrono> // Para medir el tiempo

// Definiciones para que STB implemente el código
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Fórmula de luminancia para escala de grises
unsigned char to_grayscale(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
}

int main() {
    const int width = 7952;
    const int height = 5304;
    const char* input_filename = "input_7952x5304.png";
    
    int w, h, channels;
    // Cargar imagen original
    unsigned char *image_color = stbi_load(input_filename, &w, &h, &channels, 3);
    if (!image_color || w != width || h != height) {
        std::cerr << "Error cargando la imagen " << input_filename << ". Asegurate que exista y tenga las dimensiones correctas." << std::endl;
        return 1;
    }
    channels = 3; // Forzamos 3 canales (RGB)

    std::cout << "Iniciando procesamiento secuencial para " << width << "x" << height << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    size_t img_size = width * height * channels;
    
    // 1. Crear versión en escala de grises
    unsigned char *image_gray = (unsigned char*)malloc(img_size);
    if (!image_gray) {
        std::cerr << "Error asignando memoria para imagen gris." << std::endl;
        stbi_image_free(image_color);
        return 1;
    }

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

    // 2. Generar frames del cross-fading
    const int total_frames = 96; // 4 segundos a 24 fps
    for (int frame = 0; frame < total_frames; ++frame) {
        // P va de 1.0 (todo color) a 0.0 (todo gris)
        double P = 1.0 - (double)frame / (double)(total_frames - 1);
        
        unsigned char *image_result = (unsigned char*)malloc(img_size);
        if (!image_result) {
            std::cerr << "Error asignando memoria para frame " << frame << std::endl;
            continue; // Salta este frame
        }

        // Aplicar cross-fading
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                size_t idx = (y * width + x) * channels;
                // result = int(image1 * P + image2 * (1-P))
                image_result[idx + 0] = (unsigned char)(image_color[idx + 0] * P + image_gray[idx + 0] * (1.0 - P));
                image_result[idx + 1] = (unsigned char)(image_color[idx + 1] * P + image_gray[idx + 1] * (1.0 - P));
                image_result[idx + 2] = (unsigned char)(image_color[idx + 2] * P + image_gray[idx + 2] * (1.0 - P));
            }
        }

        // Guardar el frame
        char filename[100];
        sprintf(filename, "output_seq/frame_%03d.png", frame); // Guarda en una carpeta "output_seq"
        stbi_write_png(filename, width, height, channels, image_result, width * channels);
        
        free(image_result); // Liberar memoria del frame
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;
    std::cout << "Tiempo secuencial: " << diff.count() << " segundos" << std::endl;

    // Liberar memoria principal
    stbi_image_free(image_color);
    free(image_gray);

    std::cout << "Proceso secuencial completado." << std::endl;
    return 0;
}