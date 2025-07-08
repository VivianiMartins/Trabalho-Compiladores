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
int carregarNaMemoria(int Memory, int MaxMemory, int size);
void message_error(const char *erro, int line_number); /*função para retorno de erro*/
int rules_funcao(char *line, int line_number); /*regras para funcao __xxx()*/
char* garantir_quebra_linha_apos_ponto_virgula(const char *arquivo_entrada);

int main(){
    /*carregar documento de entrada e pré-processando*/
    FILE *file = fopen("exemplo_correto.txt", "r");
    /*
    char *exemploFormatado = garantir_quebra_linha_apos_ponto_virgula("exemplo_correto.txt");
    if (exemploFormatado == NULL) {
        printf("Erro ao processar o arquivo de entrada!\n");
        return 1;
    }
    FILE *file = fopen(exemploFormatado, "r");*/

    char line[256];
    if (file != NULL) {
        int line_number = 1; /*número da linha em questão*/
        long start_pos = ftell(file);  /*Posição inicial (0)*/
        size_t memory = 0; /*memória*/
        size_t line_size = 0; /*tamanho de cada linha que irei ler*/
        const char *principal = "principal";
        int cont_principal = 0;

        while (fgets(line, sizeof(line), file)) { /*Coloquei em loop pra ficar verificando*/
            /*estamos no inicio do arquivo, então tem que começar com principal ou uma função*/
            printf("Linha %d: %s", line_number, line); /*printa linha por linha*/

            /* Detecta e ignora BOM: Arquivos UTF-8 podem começar com um caractere especial invisível (BOM) que tem
            o valor 0xEF 0xBB 0xBF em hexadecimal. Isso está fazendo com que seu primeiro caractere não seja como esperado.*/
            if (strlen(line) >= 3 && (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
                memmove(line, line + 3, strlen(line) - 2);
            }

            if (line[0] == 'p') {
                /*Checando se é principal e sua regras*/
                int parenteses_control_open = 0; /*controle do parênteses*/
                int found_parentheses = 0;
                int found_curly_brace = 0;  /*Controla a chave { */

                /* Verifica se começa com "principal" */
                for(int i = 0; i < 9; i++) {
                    if (line[i] != principal[i]) {
                        message_error("Módulo principal escrito incorretamente", line_number);
                        break;
                    } else {
                        cont_principal++;
                    }
                }

                 /* Verifica restante da linha */
                for(int i = 9; line[i] != '\0'; i++) {
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
                            break;
                        }
                    } else {/* Já encontramos o '(' */
                        /* Dentro dos parênteses: só permite espaços */
                        if (parenteses_control_open == 1) {
                            if (c == ')') {
                                parenteses_control_open--;
                            } else if (!isspace(c)) {
                                message_error("Parênteses deve conter apenas espaços", line_number);
                                break;
                            }
                        } else if (parenteses_control_open == 0) { /* Após fechar parênteses */
                            /* Se ainda não encontramos a chave */
                            if (!found_curly_brace) {
                                /* Permite espaços entre o ')' e a '{' */
                                if (isspace(c)) {
                                    continue;
                                } else if (c == '{') {  /* Encontrou a chave de abertura */
                                    found_curly_brace = 1;
                                } else { /* Qualquer outro caractere é erro */
                                    message_error("Esperado '{' após parênteses", line_number);
                                    break;
                                }
                            } else {  /* Após encontrar a chave */
                                /* Só permite espaços ou quebra de linha após a chave */
                                if (!isspace(c) && c != '\n') {
                                    continue;
                                } else {
                                     /*caso tenha algo depois de {, que não esteja na linha abaixo*/
                                }
                            }
                        }
                    }
                }

                /* Verificação final de parênteses */
                if (parenteses_control_open != 0) {
                    message_error("Parênteses não fechado corretamente", line_number);
                    break;
                }

                /* Verificação da chave (opcional dependendo dos requisitos) */
                if (!found_curly_brace) {
                    message_error("Esperado '{' após parênteses", line_number);
                    break;
                }

                /*continua com as regras até sair da função principal*/
                while (fgets(line, sizeof(line), file) ) {
                    printf("Linha %d: %s", line_number, line);
                    if (line[0] == '}'){ /*não necessariamente vai estar na posição 0*/
                        break;
                    }
                    line_number++;
                }

            } else if (line[0] == 'f') {
                int control = rules_funcao(line, line_number);
                /*continua com as regras até sair da função*/
                while (fgets(line, sizeof(line), file) && control == 0) {
                    printf("Linha %d: %s", line_number, line);
                    if (line[0] == '}'){ /*não necessariamente vai estar na posição 0*/
                        break;
                    }
                    line_number++;
                }
            } else {
                printf("%c \n", line[0]);
                message_error("Tem que iniciar com função ou principal", line_number);
            }
            line_number++;
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

int carregarNaMemoria(int Memory, int MaxMemory, int size){
    if (Memory+size <= 0.9*MaxMemory){
        return (Memory += size);
    } else {
        if ((Memory)+size<(MaxMemory*0.99)){
            printf("Alerta! Mais de 90% da Memória disponível foi utilizada");
            return (Memory += size);
        } else {
            printf("Memória cheia!!! Não foi possível carregar os bits na memória");
            return -1;
        }

    }
};

void message_error(const char *erro, int line_number) {
    printf("Erro na linha %d: %s\n", line_number, erro);
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

/* Função que garante que todo ';' seja seguido por '\n' e retorna o nome do arquivo de saída*/
char* garantir_quebra_linha_apos_ponto_virgula(const char *arquivo_entrada) {
    FILE *entrada = fopen(arquivo_entrada, "r");
    if (entrada == NULL) {
        perror("Erro ao abrir arquivo de entrada");
        exit(EXIT_FAILURE);
    }

    /* Criar nome do arquivo de saída baseado no nome de entrada*/
    char *arquivo_saida = malloc(strlen(arquivo_entrada) + 5); // +5 para "_out" e null terminator*/
    strcpy(arquivo_saida, arquivo_entrada);

    /* Adicionar "_out" antes da extensão (se houver)*/
    char *ponto = strrchr(arquivo_saida, '.');
    if (ponto != NULL) {
        /* Tem extensão, insere "_out" antes do ponto*/
        memmove(ponto + 4, ponto, strlen(ponto) + 1);
        memcpy(ponto, "_out", 4);
    } else {
        /* Sem extensão, apenas adiciona "_out" no final*/
        strcat(arquivo_saida, "_out");
    }

    FILE *saida = fopen(arquivo_saida, "w");
    if (saida == NULL) {
        perror("Erro ao criar arquivo de saída");
        fclose(entrada);
        free(arquivo_saida);
        exit(EXIT_FAILURE);
    }

    int caractere_atual;
    int proximo_caractere;

    while ((caractere_atual = fgetc(entrada)) != EOF) {
        if (caractere_atual == ';') {
            fputc(';', saida);

            /* Verifica o próximo caractere sem consumi-lo*/
            proximo_caractere = fgetc(entrada);

            if (proximo_caractere != '\n' && proximo_caractere != EOF) {
                fputc('\n', saida);
                fputc(proximo_caractere, saida);
            } else {
                /* Já tem '\n', apenas escreve*/
                fputc(proximo_caractere, saida);
            }
        } else {
            fputc(caractere_atual, saida);
        }
    }

    fclose(entrada);
    fclose(saida);

    return arquivo_saida;
}
