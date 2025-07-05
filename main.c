#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*Declaração de funções*/
void extract_plain_text(FILE *rtf_file, char *output_buffer, int buffer_size);

int main(){
    /*carregar documento de entrada*/
    FILE *file = fopen("exemplo.txt", "r");
    char line[256];

    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {  /*Aqui só printa, mas podemos aproveitar se quiser*/
            printf("%s", line); /*printa linha por linha*/
        }

        int Memory = 0; /*memória*/

        while(Memory<1024){ /*Precisamos Definir qual o máximo*/
            /*RESTANTE DO CODIGO*/
            /*verificar cada linha*/

            Memory++;
        }

        fclose(file);
    } else {
        fprintf(stderr, "Não foi possível abrir o arquivo!\n");
    }

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
