#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

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
char* garantir_quebra_linha_apos_ponto_virgula(const char *arquivo_entrada);
int verificarVariavel(char line[], int posicao, int line_number);

int main(){
    /*carregar documento de entrada e pré-processando*/
     /*carregar documento de entrada e pré-processando*/
    FILE *file = fopen("exemplo_correto_funcao.txt", "r");
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
        int cont_principal = 0;
        /*palavras reservadas*/
        const char *principal = "principal";
        const char *funcao = "funcao";
        const char *inteiro = "inteiro";

        while (fgets(line, sizeof(line), file)) { /*Coloquei em loop pra ficar verificando*/
            /*estamos no inicio do arquivo, então tem que começar com principal ou uma função*/
            printf("Linha %d: %s", line_number, line); /*printa linha por linha*/

            /* Detecta e ignora BOM: Arquivos UTF-8 podem começar com um caractere especial invisível (BOM) que tem
            o valor 0xEF 0xBB 0xBF em hexadecimal. Isso está fazendo com que seu primeiro caractere não seja como esperado.*/
            if (strlen(line) >= 3 && (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
                memmove(line, line + 3, strlen(line) - 2);
            }

            /*sempre irá iniciar com principal ou uma função*/
            if (line[0] == 'p') {
                /*Checando se é principal e sua regras*/
                int parenteses_control_open_principal = 0; /*controle do parênteses*/
                int found_parentheses = 0;
                int found_curly_brace = 0;  /*Controla a chave { */

                /* Verifica se começa com "principal" */
                for(int i = 0; i < 9; i++) {
                    if (line[i] != principal[i]) {
                        message_error("Módulo principal escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
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
                            parenteses_control_open_principal++;
                            found_parentheses = 1;
                        } else {
                            message_error("Esperado '(' após 'principal'", line_number);
                            return 1; /*O código PARA quando encontra erro*/
                        }
                    } else {/* Já encontramos o '(' */
                        /* Dentro dos parênteses: só permite espaços */
                        if (parenteses_control_open_principal == 1) {
                            if (c == ')') {
                                parenteses_control_open_principal--;
                            } else if (!isspace(c)) {
                                message_error("Parênteses deve conter apenas espaços", line_number);
                                return 1; /*O código PARA quando encontra erro*/
                            }
                        } else if (parenteses_control_open_principal == 0) { /* Após fechar parênteses */
                            /* Se ainda não encontramos a chave */
                            if (!found_curly_brace) {
                                /* Permite espaços entre o ')' e a '{' */
                                if (isspace(c)) {
                                    continue;
                                } else if (c == '{') {  /* Encontrou a chave de abertura */
                                    found_curly_brace = 1;
                                } else { /* Qualquer outro caractere é erro */
                                    message_error("Esperado '{' após parênteses", line_number);
                                    return 1; /*O código PARA quando encontra erro*/
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
                if (parenteses_control_open_principal != 0) {
                    message_error("Parênteses não fechado corretamente", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }

                /* Verificação da chave (opcional dependendo dos requisitos) */
                if (!found_curly_brace) {
                    message_error("Esperado '{' após parênteses", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
                cont_principal++;

                if (cont_principal > 1) {
                    message_error("Módulo principal tem que ser único", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
                printf("principal ok\n");
                /*fim da checagem se é principal e sua regras*/



            } else if (line[0] == 'f') {
                /*Checando se é funcao __xxx(){retorna} e sua regras*/
                int parenteses_control_open_funcao = 0;
                const char *retorno = "retorno";
                bool underscore_name_control = false;
                bool after_underscore_name_control = false;
                bool parameter_control = false;
                bool parenteses_parameter_control = false;

                /* Verifica se começa com "funcao" */
                for(int i = 0; i < 6; i++) {
                    if (line[i] != funcao[i] && i < 6) {
                        message_error("Módulo funcao escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }

                /* Verifica restante da linha */
                for(int i = 6; line[i] != '\0'; i++) {
                     /*verificar __*/
                     if ((line[i] == ' ' || line[i] == '_') && !underscore_name_control && !after_underscore_name_control) {
                        if (line[i] == '_' && line[i+1] == '_') {
                            underscore_name_control = true;
                            i++;
                            continue;
                        } else{
                            if (line[i] != ' ' && !underscore_name_control) {
                                message_error("Nome da função tem que iniciar com __", line_number);
                                return 1; /*O código PARA quando encontra erro*/
                            }
                        }

                     }
                     /* Verificar nome após __ */
                     if (underscore_name_control && !after_underscore_name_control) {
                        if (line[i] >= 'a' && line[i] <= 'z') { /*verifica se está entre a...z*/
                            do{
                                i++;
                            }while (isalnum((unsigned char)line[i])); /*Verifica se é um caractere alfanumérico (letra maiúscula/minúscula ou dígito decimal).*/
                                i--; /*volta um*/
                                after_underscore_name_control = true;
                            continue;
                        } else {
                            message_error("Nome da função tem que iniciar com __alfanumérico", line_number);
                            return 1; /*O código PARA quando encontra erro*/
                        }
                     }

                    if (parenteses_control_open_funcao == 0 && after_underscore_name_control && !parenteses_parameter_control) {
                        if (line[i] == ' ' || line[i] == '(') { /*nome ok, abre parênteses*/
                            if(line[i] == '(') {
                                parenteses_control_open_funcao++;
                            }
                        }
                    } else { /*pode ou não ter parâmetros*/
                        if (line[i] == ' ' || line[i] == '!') {
                            if (line[i] == '!') {
                                if (line[i+1] >= 'a' && line[i+1] <= 'z') { /*verifica se está entre a...z*/
                                    do{
                                        i++;
                                    }while (isalnum((unsigned char)line[i])); /*verifica se o restante é alfanumerico*/

                                    if (line[i] == ',' || line[i-1] == ',') { /*tem mais parâmetros*/
                                        printf("tem mais parâmetros\n");
                                    } else {
                                         i--; /*volta um*/
                                        continue;
                                    }


                                } else {
                                    message_error("Parâmetro da função tem que iniciar com !a..z", line_number);
                                    return 1; /*O código PARA quando encontra erro*/
                                }
                            }
                            /*1.4.2.Após o nome deve-se conter necessariamente o “(“ e “)” (abre e fecha parênteses);
                            1.4.2.1. Dentro dos parênteses pode conter parâmetros;
                            1.4.2.1.1. Se ocorrerem, devem ser informados tipo de dados e nome da variável;
                            1.4.2.1.1.1. Para os tipos e nome de variáveis ver item 2;
                            1.4.2.1.2. Os parâmetros não devem ser declarados dentro da funçao;
                            1.4.3.Não existe limitação de quantidade de parâmetros na funcao(), porém se houver mais de 01 (um) deverão ser separados por vírgula (somente
                            uma);*/
                        } else if (line[i] == ')') {
                            printf("fechou parênteses\n");
                        }
                    }
                }

                printf("Funcao ok\n");
                /*Fim da checagem se é funcao __xxx(){retorna} e sua regras*/
            } else {
                if (line_number == 1) {
                    printf("%c \n", line);
                    message_error("Tem que iniciar com função ou principal", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
                /*aqui continua as verificações*/
                if (line[0] == 'i'){
                    for(int i = 0; line[i] != '\0'; i++) {
                        if (line[i] != inteiro[i] && i < 6) {
                            message_error("Inteiro escrito incorretamente", line_number);
                            return 1; /*O código PARA quando encontra erro*/
                            }
                        }
                    int declaracaoInteiro = 1; /* EU preciso usar depois, pra ver se aconteceu a declaração de fato */
                    /* Verifica restante da linha */
                    for(int i = 7; line[i] != '\0'; i++) {
                        char c = line[i];
                        if (isspace(c)) {
                            /* Ignora, não há nada a fazer */
                        } else if (c=='!'){
                            i++;
                            if (line[i] >= 'a' && line[i] <= 'z') {
                                    do{
                                        i++;
                                    }while (isalnum((unsigned char)line[i])); /*verifica se o restante é alfanumerico*/

                                    if (line[i] == ',') { /*tem mais parâmetros*/
                                        if(verificarVariavel(line, i+1, line_number) == 1){
                                            return 1;
                                        } else {
                                            break;
                                        }
                                    }
                            } else {
                                message_error("Variáveis precisam começar com letra minúscula.\n", line_number);
                                return 1;
                            }

                        } else {
                                message_error("Falta '!' antes da variável.\n", line_number);
                                printf("%c", c);
                                return 1;
                        }
                    }
                    printf("Inteiro ok\n");
            }
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

/* Função para declaração de variável*/
int verificarVariavel(char line[], int posicao, int line_number) {
    for(int i = posicao; line[i] != '\0'; i++) {
            char c = line[i];
            if (isspace(c)) {
            /* Ignora, não há nada a fazer */
            } else if (c=='!'){
                i++;
                if (line[i] >= 'a' && line[i] <= 'z') {
                    do{
                        i++;
                    }while (isalnum((unsigned char)line[i])); /*verifica se o restante é alfanumerico*/
                    if (line[i] == ',' && (isspace(line[i+1]))) { /*tem mais parâmetros que precisam ser verificados*/
                        return verificarVariavel(line, i+1, line_number);
                    } else if (line[i] == ';' && line[i+1] == '\0'){
                        return 0;
                    } else if (line[i] == ';' && line[i+1] == '\n'){
                        return 0;
                    } else if (isspace(line[i])){
                        do{
                        i++;
                        }while (isspace(line[i])); /*pula espaços*/
                        if (line[i] == '='){
                            i++;
                            do{
                                i++;
                            }while (line[i]!=';'&&line[i]!='\0'&&line[i]!='\n'); /*pula atribuição até encontra ';'*/
                            if (line[i] != ';'){
                                i++;
                                message_error("Não foi encontrado ';' \n", line_number);
                                return 1;
                            } else if (isspace(line[i-1])){
                                message_error("Falta algo depois de '=' \n", line_number);
                                return 1;
                            } else {
                                return 0;
                            }

                        }
                    }{
                        message_error("Variáveis só podem conter alfanuméricos.\n", line_number);
                        printf(line[i]);
                        printf(line[i]);
                        printf(line[i]);
                        printf("\n\n");
                        return 1;
                    }
                } else {
                    message_error("Variáveis precisam começar com letra minúscula.\n", line_number);
                    return 1;
                }

            } else {
                message_error("Falta '!' antes da variável.\n", line_number);
                return 1;
            }
    }
    message_error("Declaração de variável não encontrada.\n", line_number);
    return 1;
}
