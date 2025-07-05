#include <stdio.h>
#include <stdlib.h>
#include "funcoes.h"

int main()
{
    /*carregar documento de entrada*/
    FILE *rtf_file = fopen("exemplo.txt", "r");

    if (!rtf_file) {
        perror("Error opening RTF file");
        return 1;
    }

    char plain_text_buffer[1024];
    extract_plain_text(rtf_file, plain_text_buffer, sizeof(plain_text_buffer));

    printf("Extracted Plain Text:\n%s\n", plain_text_buffer);

    fclose(rtf_file);



    /*memória*/

    /*verfificar cada linha*/

    printf("Hello world!\n");
    return 0;
}
