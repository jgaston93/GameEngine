#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char *argv[])
{
    const uint32_t num_values_per_row = 10;
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(argv[1], &width, &height, &nrChannels, 0);
    uint32_t num_values = width * height * nrChannels;

    uint32_t str_len = strlen(argv[1]);
    for(uint32_t i = str_len - 1; i > 0; i--)
    {
        if(argv[1][i] == '.')
        {
            argv[1][i] = '\0';
            break;
        }
    }

    char* texture_name;
    
    for(uint32_t i = str_len - 1; i > 0; i--)
    {
        if(argv[1][i] == '/')
        {
            texture_name = &argv[1][i+1];
            break;
        }
    }
    texture_name = strupr(texture_name);

    printf("#ifndef %s_H\n", texture_name);
    printf("#define %s_H\n", texture_name);
    printf("\n");
    printf("#include <stdint.h>\n");
    printf("\n");
    
    texture_name = strlwr(texture_name);
    printf("uint32_t %s_width = %d;\n", texture_name, width);
    printf("uint32_t %s_height = %d;\n", texture_name, height);
    printf("uint32_t %s_num_channels = %d;\n", texture_name, nrChannels);
    printf("\n");
    printf("uint8_t %s_data[] = {\n", texture_name);

    for(uint32_t i = 0; i < num_values; i++)
    {
        printf("    0x%X", data[i]);
        if(((i + 1) % num_values_per_row) == 0)
        {
            printf(",\n");
        }
        else
        {
            printf(", ");
        }
    }

    printf("};\n");
    printf("\n");

    texture_name = strupr(texture_name);
    printf("#endif // %s\n", texture_name);

    stbi_image_free(data);
    return 0;
}