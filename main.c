#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*Declaração de funções*/
void extract_plain_text(FILE *rtf_file, char *output_buffer, int buffer_size);

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


/*função para extrair texto*/
void extract_plain_text(FILE *rtf_file, char *output_buffer, int buffer_size) {
    int ch;
    int in_control_word = 0;
    int brace_level = 0;
    int buffer_index = 0;

    while ((ch = fgetc(rtf_file)) != EOF) {
        if (ch == '\\') {
            in_control_word = 1;
        } else if (in_control_word) {
            if (!isalpha(ch)) {
                in_control_word = 0;
                if (ch == '{') {
                    brace_level++;
                } else if (ch == '}') {
                    brace_level--;
                } else {
                    if (ch != '{' && ch != '}') {
                        if (buffer_index < buffer_size - 1) {
                            output_buffer[buffer_index++] = ch;
                        }
                    }
                }
            }
        } else if (ch == '{') {
            brace_level++;
        } else if (ch == '}') {
            brace_level--;
        } else {
            if (buffer_index < buffer_size - 1) {
                output_buffer[buffer_index++] = ch;
            }
        }
    }
    output_buffer[buffer_index] = '\0';
}
