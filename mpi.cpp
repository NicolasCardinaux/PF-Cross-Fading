#include <iostream>
#include <vector>
#include <string>
#include <mpi.h> // <-- 1. Incluir MPI
#include <chrono>

// STB se define e incluye solo si somos el proceso 0
// o si todos los procesos necesitan escribir archivos.
// En esta estrategia, TODOS escriben, así que todos lo incluyen.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


unsigned char to_grayscale(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // <-- 2. Iniciar MPI

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Mi ID
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Cuántos somos

    int width, height, channels;
    unsigned char *image_color = nullptr;
    unsigned char *image_gray = nullptr;
    size_t img_size;

    double start_time;
    if (rank == 0) {
        const int req_width = 7952;
        const int req_height = 5304;
        const char* input_filename = "input_7952x5304.png";
        
        std::cout << "Iniciando MPI (" << size << " procesos) para " << req_width << "x" << req_height << std::endl;
        start_time = MPI_Wtime(); // Timer de MPI

        image_color = stbi_load(input_filename, &width, &height, &channels, 3);
        if (!image_color || width != req_width || height != req_height) {
            std::cerr << "Error cargando imagen. Abortando." << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        channels = 3;
        img_size = width * height * channels;

        // Proceso 0 crea la imagen gris
        image_gray = (unsigned char*)malloc(img_size);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                size_t idx = (y * width + x) * channels;
                unsigned char gray = to_grayscale(image_color[idx], image_color[idx+1], image_color[idx+2]);
                image_gray[idx + 0] = gray;
                image_gray[idx + 1] = gray;
                image_gray[idx + 2] = gray;
            }
        }
        std::cout << "Proceso 0: Imagen cargada y convertida a gris." << std::endl;
    }

    // <-- 3. Broadcast de datos
    
    // Compartir dimensiones
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        // Otros procesos asignan memoria
        img_size = width * height * channels;
        image_color = (unsigned char*)malloc(img_size);
        image_gray = (unsigned char*)malloc(img_size);
    }

    // Compartir datos de las imágenes
    MPI_Bcast(image_color, img_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(image_gray, img_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // <-- 4. División de trabajo (frames)
    const int total_frames = 96;
    int frames_per_proc = total_frames / size;
    int start_frame = rank * frames_per_proc;
    int end_frame = (rank + 1) * frames_per_proc;
    
    // El último proceso se queda con el resto
    if (rank == size - 1) {
        end_frame = total_frames;
    }

    // if (rank == 0) printf("Proceso %d: frames %d a %d\n", rank, start_frame, end_frame - 1);
    
    // <-- 5. Cada proceso genera sus frames
    for (int frame = start_frame; frame < end_frame; ++frame) {
        double P = 1.0 - (double)frame / (double)(total_frames - 1);
        unsigned char *image_result = (unsigned char*)malloc(img_size);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                size_t idx = (y * width + x) * channels;
                image_result[idx + 0] = (unsigned char)(image_color[idx + 0] * P + image_gray[idx + 0] * (1.0 - P));
                image_result[idx + 1] = (unsigned char)(image_color[idx + 1] * P + image_gray[idx + 1] * (1.0 - P));
                image_result[idx + 2] = (unsigned char)(image_color[idx + 2] * P + image_gray[idx + 2] * (1.0 - P));
            }
        }

        char filename[100];
        sprintf(filename, "output_mpi/frame_%03d.png", frame); // Guarda en "output_mpi"
        stbi_write_png(filename, width, height, channels, image_result, width * channels);
        free(image_result);
    }

    // Liberar memoria
    free(image_color);
    free(image_gray);

    MPI_Barrier(MPI_COMM_WORLD); // Esperar que todos terminen

    if (rank == 0) {
        double end_time = MPI_Wtime();
        std::cout << "Tiempo MPI: " << (end_time - start_time) << " segundos" << std::endl;
        std::cout << "Proceso MPI completado." << std::endl;
    }

    MPI_Finalize(); // <-- 6. Finalizar MPI
    return 0;
}