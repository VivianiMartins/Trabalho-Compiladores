#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*Declaração de funções*/
void extract_plain_text(FILE *rtf_file, char *output_buffer, int buffer_size);
int carregarNaMemoria(int Memory, int MaxMemory, int size);

int main(){
    /*carregar documento de entrada*/
    FILE *file = fopen("exemplo.txt", "r");
    int Memory = 0;
    int MaxMemory = 2048; /*memória precisa ser parametrizada, então acredito que seja isso*/
    char line[256];
    Memory = carregarNaMemoria(Memory, MaxMemory, sizeof(file)); /*Exemplo de uso de memória*/
    if (Memory == -1){
        return 0;
    }

    if (file != NULL) {
        while(fgets(line, sizeof(line), file)){ /*Precisamos Definir qual o máximo*/
            /*RESTANTE DO CODIGO*/
            /*verificar cada linha*/
            printf("%s", line); /*Por enquanto, está apenas printando linha por linha*/
        }

        fclose(file);
    } else {
        fprintf(stderr, "Não foi possível abrir o arquivo!\n");
    }

    return 0;
}

int carregarNaMemoria(int Memory, int MaxMemory, int size){
    if (Memory+size <= 0.9*MaxMemory){
        return (Memory += size);
    } else {
        if ((Memory)+size<(MaxMemory*0.99)){
            printf("Alerta! Mais de 90% da Memória disponível foi utilizada");
            return (Memory += size);
        } else {
            printf("Memória cheia!!! Não foi possível carregar os bits na memória");
            return (-1);
        }

    }
};

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
