#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*Tamanho total de memória*/
#define MAX_MEMORY_KB 2048
#define MAX_MEMORY_BYTES (MAX_MEMORY_KB * 1024)

/*Tamanho das variáveis aceitáveis*/
/*Luiz isso era para ser assim? Não sei se entendi corretamente*/
#define INTEIRO_MEMORY_BYTES sizeof(int)    /* Normalmente 4 bytes */
#define DECIMAL_MEMORY_BYTES  sizeof(float)  /* Normalmente 4 bytes */
#define TEXTO_EACH_CHAR_MEMORY_BYTES sizeof(char)   /* Sempre 1 byte */

/*Declaração de funções*/
void message_error(const char *erro, int line_number); /*função para retorno de erro*/
int rules_principal(char *line, int line_number); /*regras para principal()*/
int rules_funcao(char *line, int line_number); /*regras para funcao __xxx()*/

int main(){
    /*carregar documento de entrada*/
    FILE *file = fopen("exemplo_correto.txt", "r");
    char line[256];

    if (file != NULL) {
        int line_number = 1; /*número da linha em questão*/
        long start_pos = ftell(file);  /*Posição inicial (0)*/
        size_t memory = 0; /*memória*/
        size_t line_size = 0; /*tamanho de cada linha que irei ler*/
        int control = 0;
        int cont_principal = 0;

        if (fgets(line, sizeof(line), file)) {
            /*estamos no inicio do arquivo, então tem que começar com principal ou uma função*/
            printf("Linha %d: %s", line_number, line); /*printa linha por linha*/
            /* Detecta e ignora BOM: Arquivos UTF-8 podem começar com um caractere especial invisível (BOM) que tem
            o valor 0xEF 0xBB 0xBF em hexadecimal. Isso está fazendo com que seu primeiro caractere não seja como esperado.*/
            if (strlen(line) >= 3 && (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
                memmove(line, line + 3, strlen(line) - 2);
            }

            if (line[0] == 'p') {
                control = rules_principal(line, line_number);
                if (control == 0 ){
                    cont_principal++;
                }
            } else if (line[0] == 'f') {
                control = rules_funcao(line, line_number);
            } else {
                printf("%c \n", line[0]);
                message_error("Tem que iniciar com função ou principal", line_number);
                control = 1;
            }
        }

        while (fgets(line, sizeof(line), file) && control == 0) {  /*Aqui só printa, mas podemos aproveitar se quiser*/
            line_number++;
            printf("Linha %d: %s", line_number, line); /*printa linha por linha*/

            line_size = strlen(line) + 1; /*+1 para o terminador nulo*/
            memory =+ line_size;
            printf("Memória usada: %zu/%zu bytes\n", memory, MAX_MEMORY_BYTES);

            /*Verifica se ultrapassa o limite*/
            if (memory > MAX_MEMORY_BYTES) {
                printf("Erro: Memória excedida! Limite é %d KB\n", MAX_MEMORY_KB);
                break;
            }

            if (line[0] == 'p') {
                control = rules_principal(line, line_number);
                cont_principal++;
                if (cont_principal > 1 ){
                    message_error("Principal tem que ser único", line_number);
                    break;
                }
            } else {
                continue;
            }


            memory =- line_size; /*remove da memória o total da linha toda, após processá-la*/

            if (control = 1) {
                break;
            }

        }

        /*Verificação final de arquivo*/
        if (feof(file) && cont_principal==0) {
            message_error("Módulo Principal Inexistente", line_number);
        }

        fclose(file);
    } else {
        fprintf(stderr, "Não foi possível abrir o arquivo!\n");
    }

    return 0;
}

void message_error(const char *erro, int line_number) {
    printf("Erro na linha %d: %s\n", line_number, erro);
}

int rules_principal(char *line, int line_number) {
    const char *expected = "principal";
    int parenteses_control_open = 0; /*controle do parênteses*/
    int found_parentheses = 0;
    int found_curly_brace = 0;  /*Controla a chave { */
    int i;

    /* Verifica se começa com "principal" */
    for(i = 0; i < 9; i++) {
        if (line[i] != expected[i]) {
            message_error("Módulo principal escrito incorretamente", line_number);
            return 1;
        }
    }

     /* Verifica restante da linha */
    for(i = 9; line[i] != '\0'; i++) {
        char c = line[i];

        /* Ignora espaços antes dos parênteses */
        if (!found_parentheses && isspace(c)) {
            continue;
        }

        /* Primeiro não-espaço após "principal" deve ser '(' */
        if (!found_parentheses) {
            if (c == '(') {
                parenteses_control_open++;
                found_parentheses = 1;
            } else {
                message_error("Esperado '(' após 'principal'", line_number);
                return 1;
            }
        }
        /* Já encontramos o '(' */
        else {
            /* Dentro dos parênteses: só permite espaços */
            if (parenteses_control_open == 1) {
                if (c == ')') {
                    parenteses_control_open--;
                } else if (!isspace(c)) {
                    message_error("Parênteses deve conter apenas espaços", line_number);
                    return 1;
                }
            }
            /* Após fechar parênteses */
            else if (parenteses_control_open == 0) {
                /* Se ainda não encontramos a chave */
                if (!found_curly_brace) {
                    /* Permite espaços entre o ')' e a '{' */
                    if (isspace(c)) {
                        continue;
                    }
                    /* Encontrou a chave de abertura */
                    else if (c == '{') {
                        found_curly_brace = 1;
                    }
                    /* Qualquer outro caractere é erro */
                    else {
                        message_error("Esperado '{' após parênteses", line_number);
                        return 1;
                    }
                }
                /* Após encontrar a chave */
                else {
                    /* Só permite espaços ou quebra de linha após a chave */
                    if (!isspace(c) && c != '\n') {
                        message_error("Texto após '{' não permitido", line_number);
                        return 1;
                    }
                }
            }
        }
    }

    /* Verificação final de parênteses */
    if (parenteses_control_open != 0) {
        message_error("Parênteses não fechado corretamente", line_number);
        return 1;
    }

    /* Verificação da chave (opcional dependendo dos requisitos) */
    if (!found_curly_brace) {
        message_error("Esperado '{' após parênteses", line_number);
        return 1;
    }

    printf("Principal ok\n");
    return 0;
}

int rules_funcao(char *line, int line_number) {
    const char *expected = "funcao";
    int parenteses_control_open = 0;

    for(int i = 0; line[i] != '\0'; i++) {
        if (line[i] != expected[i] && i < 6) {
            message_error("Módulo funcao escrito incorretamente", line_number);
            return 1;
        } else {
            if (line[i] == ' ' || line[i] == '(') {
                if(line[i] == '(') {
                    parenteses_control_open++;
                }
            }
        }
    }

    printf("Funcao ok\n");
    return 0;
}
