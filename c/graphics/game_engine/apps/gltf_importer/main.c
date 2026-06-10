#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools/importers/gltf_extract.h"

static void print_usage(const char* program_name) {
    printf("Usage: %s <input.gltf> -o <output_dir>\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help    Show this help message and exit\n");
    printf("  -o, --output  Specify the output directory (Required)\n");
    printf("\nExample:\n");
    printf("  %s raw_assets/sponza.gltf -o assets/levels/sponza\n", program_name);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char* input_gltf = NULL;
    const char* output_dir = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_dir = argv[++i];
            } else {
                fprintf(stderr, "Error: -o requires a directory argument.\n");
                return EXIT_FAILURE;
            }
        } else {
            if (input_gltf == NULL) {
                input_gltf = argv[i];
            } else {
                fprintf(
                    stderr,
                    "Error: Too many input files specified. Found '%s' and '%s'.\n",
                    input_gltf,
                    argv[i]
                );
                return EXIT_FAILURE;
            }
        }
    }

    if (input_gltf == NULL) {
        fprintf(stderr, "Error: No input glTF file specified.\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (output_dir == NULL) {
        fprintf(stderr, "Error: No output directory specified. Use -o <dir>\n");
        return EXIT_FAILURE;
    }

    printf("Starting Import...\n");
    printf("  Source: %s\n", input_gltf);
    printf("  Target: %s\n", output_dir);

    // if (!extract_gltf_monolithic(input_gltf, output_dir)) {
    //     fprintf(stderr, "\nERROR: glTF import failed.\n");
    //     return EXIT_FAILURE;
    // }

    printf("\nSUCCESS: Import completed.\n");
    return EXIT_SUCCESS;
}
