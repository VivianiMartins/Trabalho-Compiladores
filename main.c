#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

/*Tamanho total de memória*/
#define MAX_MEMORY_KB 2048
#define MAX_MEMORY_BYTES (MAX_MEMORY_KB * 1024)

/*Tamanho das variáveis aceitáveis*/
#define INTEIRO_MEMORY_BYTES sizeof(int)          /* Normalmente 4 bytes */
#define DECIMAL_MEMORY_BYTES sizeof(float)        /* Normalmente 4 bytes */
#define TEXTO_EACH_CHAR_MEMORY_BYTES sizeof(char) /* Sempre 1 byte */

/*Declaração de funções*/
int carregarNaMemoria(int Memory, int MaxMemory, int size);
void message_error(const char *erro, int line_number); /*função para retorno de erro*/
char *garantir_quebra_linha_apos_ponto_virgula(const char *arquivo_entrada);
int verificarVariavelInteira(char line[], int posicao, int line_number); /*função para tratar parte de inteiro*/
int verificarVariavelTexto(char line[], int posicao, int line_number);   /*função para tratar parte de texto*/
int verificarVariavelDecimal(char line[], int posicao, int line_number); /*função para tratar parte de decimal*/
int verificarLeia(char line[], int posicao, int line_number);            /*função para tratar parte de leia*/
int is_smart_quote(const char *str, int pos, int length);               /*Função para verificar tipos de aspas duplas diferentes*/
/* Estrutura para retornar dois valores em verificarParametro*/
typedef struct
{
    int posicao;
    int sucesso;
} Resultado;
Resultado verificarParametroFuncao(char line[], int posicao, int line_number); /*função para tratar parametro das funcoes*/
Resultado verificarParametrosPara(char line[], int posicao, int line_number);  /*função para tratar parametros de para*/
Resultado verificarParametrosSe(char line[], int posicao, int line_number, int len);    /*função para tratar parametros de se*/
verificarOperacaoMatematicaMain(char line[], int posicao, int line_number);/*função para operacoes em geral no main*/
verificarOperacaoMatematica(char line[], int posicao, int line_number, int flagTemPonto);/*função para operacoes matematicas no inteiro e no decimall*/
int main()
{
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
    if (file != NULL)
    {
        int line_number = 1;          /*número da linha em questão*/
        long start_pos = ftell(file); /*Posição inicial (0)*/
        size_t memory = 0;            /*memória*/
        size_t line_size = 0;         /*tamanho de cada linha que irei ler*/
        int cont_principal = 0;
        /*palavras reservadas*/
        const char *principal = "principal";
        const char *para = "para";
        const char *funcao = "funcao";
        const char *inteiro = "inteiro";
        const char *texto = "texto";
        const char *decimal = "decimal";
        const char *leia = "leia";
        const char *retorno = "retorno";
        const char *escreva = "escreva";
        const char *se = "se";
        const char *senao = "senao";

        while (fgets(line, sizeof(line), file))
        { /*Coloquei em loop pra ficar verificando*/
            /*estamos no inicio do arquivo, então tem que começar com principal ou uma função*/
            printf("Linha %d: %s", line_number, line); /*printa linha por linha*/

            /* Detecta e ignora BOM: Arquivos UTF-8 podem começar com um caractere especial invisível (BOM) que tem
            o valor 0xEF 0xBB 0xBF em hexadecimal. Isso está fazendo com que seu primeiro caractere não seja como esperado.*/
            if (strlen(line) >= 3 && (unsigned char)line[0] == 0xEF &&
                (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
            {
                memmove(line, line + 3, strlen(line) - 2);
            }

            if (isspace((unsigned char)line[0]))
            {
                /*Verifica se  uma linha apenas com espaço ou vazia*/
                int i = 0;
                while (line[i] != '\0' && isspace((unsigned char)line[i]))
                {
                    i++;
                }
                /* Se encontrou conteúdo, remove espaços iniciais */
                if (line[i] != '\0')
                {
                    /* Move o conteúdo para o início da string */
                    int j = 0;
                    while (line[i] != '\0')
                    {
                        line[j++] = line[i++];
                    }
                    line[j] = '\0';
                }
                /* Se linha é totalmente vazia (só espaços), limpa a string */
                else
                {
                    line[0] = '\0';
                }
            }

            if (line[0] == 'p')
            {
                bool is_principal = false;
                bool is_para = false;
                /* Verifica se começa com "principal" ou "para" */
                if (line[1] == principal[1])
                {
                    is_principal = true;
                }
                else if (line[1] == para[1])
                {
                    is_para = true;
                }

                if (is_principal)
                {
                    /*Checando se é principal*/
                    int i = 0;
                    /* Verifica se principal(){ */
                    for (i; i < 9; i++)
                    {
                        if (line[i] != principal[i])
                        {
                            message_error("Módulo principal escrito incorretamente", line_number);
                            return 1; /*O código PARA quando encontra erro*/
                        }
                    }
                    int parenteses_control_open_principal = 0; /*controle do parênteses*/
                    int found_parentheses_principal = 0;
                    int found_curly_brace_principal = 0; /*Controla a chave { */
                    /* Verifica restante da linha */
                    for (i; line[i] != '\0'; i++)
                    {
                        char c = line[i];
                        /* Ignora espaços antes dos parênteses */
                        if (!found_parentheses_principal && isspace(c))
                        {
                            continue;
                        }
                        /* Primeiro não-espaço após "principal" deve ser '(' */
                        if (!found_parentheses_principal)
                        {
                            if (c == '(')
                            {
                                parenteses_control_open_principal++;
                                found_parentheses_principal = 1;
                            }
                            else
                            {
                                message_error("Esperado '(' após 'principal'", line_number);
                                return 1; /*O código PARA quando encontra erro*/
                            }
                        }
                        else
                        { /* Já encontramos o '(' */
                            /* Dentro dos parênteses: só permite espaços */
                            if (parenteses_control_open_principal == 1)
                            {
                                if (c == ')')
                                {
                                    parenteses_control_open_principal--;
                                }
                                else if (!isspace(c))
                                {
                                    message_error("Parênteses deve conter apenas espaços", line_number);
                                    return 1; /*O código PARA quando encontra erro*/
                                }
                            }
                            else if (parenteses_control_open_principal == 0)
                            { /* Após fechar parênteses */
                                /* Se ainda não encontramos a chave */
                                if (!found_curly_brace_principal)
                                {
                                    /* Permite espaços entre o ')' e a '{' */
                                    if (isspace(c))
                                    {
                                        continue;
                                    }
                                    else if (c == '{')
                                    { /* Encontrou a chave de abertura */
                                        found_curly_brace_principal = 1;
                                    }
                                    else
                                    { /* Qualquer outro caractere é erro */
                                        message_error("Esperado '{' após parênteses", line_number);
                                        return 1; /*O código PARA quando encontra erro*/
                                    }
                                }
                                else
                                { /* Após encontrar a chave e não tiver terminado, passar p frente*/
                                    break;
                                }
                            }
                        }
                    }

                    /* Verificação final de parênteses */
                    if (parenteses_control_open_principal != 0)
                    {
                        message_error("Parênteses não fechado corretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }

                    /* Verificação da chave (opcional dependendo dos requisitos) */
                    if (!found_curly_brace_principal)
                    {
                        message_error("Esperado '{' após parênteses", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                    cont_principal++;

                    if (cont_principal > 1)
                    {
                        message_error("Módulo principal tem que ser único", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                    printf("principal ok\n");
                    /*fim da checagem se é principal*/
                }
                else if (is_para)
                {
                    /*Checando se é para*/
                    int i = 0;
                    /* Verifica para(){ */
                    for (i; i < 4; i++)
                    {
                        if (line[i] != para[i])
                        {
                            message_error("Módulo para escrito incorretamente", line_number);
                            return 1; /*O código PARA quando encontra erro*/
                        }
                    }
                    int parenteses_control_open_para = 0; /*controle do parênteses*/
                    bool found_parentheses_para = false;
                    bool found_curly_brace_para = false; /*Controla a chave { */
                    bool parameter_control_para = false;
                    bool parenteses_parameter_control_para = false;
                    /* Verifica restante da linha */
                    for (i; line[i] != '\0'; i++)
                    {
                        /* Verificar parênteses*/
                        if (parenteses_control_open_para == 0 && !parenteses_parameter_control_para)
                        {
                            if (isspace((unsigned char)line[i]) || line[i] == '(')
                            { /*nome ok, abre parênteses*/
                                if (line[i] == '(')
                                {
                                    parenteses_control_open_para++;
                                    parenteses_parameter_control_para = true;
                                }
                            }
                        }
                        else if (parenteses_parameter_control_para)
                        { /*verificar o que tem no parênteses*/
                            if (isspace((unsigned char)line[i]) || line[i] == '!')
                            {
                                if (line[i] == '!')
                                {
                                    Resultado res = verificarParametrosPara(line, i, line_number);
                                    i = res.posicao;
                                    if (res.sucesso == 1)
                                    {
                                        message_error("Parâmetros de para tem que iniciar com !a..z", line_number);
                                        return 1;
                                    }
                                    else
                                    {
                                        continue;
                                    }
                                }
                            }
                            else if (line[i] == ')')
                            {
                                parenteses_control_open_para--;
                            }
                            else if (parenteses_control_open_para == 0)
                            { /* Após fechar parênteses */
                                /* Se ainda não encontramos a chave */
                                if (!found_curly_brace_para)
                                {
                                    if (isspace((unsigned char)line[i]))
                                    {
                                        continue;
                                    }
                                    else if (line[i] == '{')
                                    { /* Encontrou a chave de abertura */
                                        found_curly_brace_para = true;
                                    }
                                    else
                                    { /* Qualquer outro caractere é erro */
                                        message_error("Esperado '{' após parênteses", line_number);
                                        return 1; /*O código PARA quando encontra erro*/
                                    }
                                }
                                else
                                { /* Após encontrar a chave e não tiver terminado, passar p frente*/
                                    break;
                                }
                            }
                        }
                    }

                    printf("para ok\n");
                    /*fim da checagem se é para*/
                } else {
                    message_error("Principal ou para incorreto", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
            }
            else if (line[0] == 'f')
            {
                /*Checando se é funcao __xxx(){*/
                int i = 0;
                /* Verifica se começa com "funcao" */
                for (i; i < 6; i++)
                {
                    if (line[i] != funcao[i])
                    {
                        message_error("Módulo funcao escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }
                int parenteses_control_open_funcao = 0;
                bool underscore_name_control = false;
                bool after_underscore_name_control = false;
                bool parameter_control = false;
                bool parenteses_parameter_control = false;
                bool funcao_found_curly_brace = false; /*Controla a chave { */
                /* Verifica restante da linha */
                for (i; line[i] != '\0'; i++)
                {
                    /*verificar __*/
                    if ((isspace((unsigned char)line[i]) || line[i] == '_') && !underscore_name_control && !after_underscore_name_control)
                    {
                        if (line[i] == '_' && line[i + 1] == '_')
                        {
                            underscore_name_control = true;
                            i++;
                            continue;
                        }
                        else
                        {
                            if (line[i] != ' ' && !underscore_name_control)
                            {
                                message_error("Nome da função tem que iniciar com __", line_number);
                                return 1; /*O código PARA quando encontra erro*/
                            }
                        }
                    }
                    /* Verificar nome após __ */
                    if (underscore_name_control && !after_underscore_name_control)
                    {
                        if (line[i] >= 'a' && line[i] <= 'z')
                        { /*verifica se está entre a...z*/
                            while (isalnum((unsigned char)line[i]))
                            {
                                i++;
                            } /*Verifica se é um caractere alfanumérico (letra maiúscula/minúscula ou dígito decimal).*/

                            if (!(isspace((unsigned char)line[i]) || line[i] == '('))
                            {
                                message_error("Nome da função escrito com caracter inválido", line_number);
                                return 1; /*O código PARA quando encontra erro*/
                            }

                            i--; /*volta um*/
                            after_underscore_name_control = true;
                            continue;
                        }
                        else
                        {
                            message_error("Nome da função tem que iniciar com __letra minscula", line_number);
                            return 1; /*O código PARA quando encontra erro*/
                        }
                    }
                    /* Verificar parênteses após nome*/
                    if (parenteses_control_open_funcao == 0 && after_underscore_name_control && !parenteses_parameter_control)
                    {
                        if (isspace((unsigned char)line[i]) || line[i] == '(')
                        { /*nome ok, abre parênteses*/
                            if (line[i] == '(')
                            {
                                parenteses_control_open_funcao++;
                                parenteses_parameter_control = true;
                            }
                        }
                    }
                    else if (parenteses_parameter_control)
                    { /*pode ou não ter parâmetros*/
                        if (isspace((unsigned char)line[i]) || line[i] == '!')
                        {
                            if (line[i] == '!')
                            {
                                Resultado res = verificarParametroFuncao(line, i, line_number);
                                i = res.posicao;
                                if (res.sucesso == 1)
                                {
                                    return 1;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                        }
                        else if (line[i] == ')')
                        {
                            parenteses_control_open_funcao--;
                        }
                        else if (parenteses_control_open_funcao == 0)
                        { /* Após fechar parênteses */
                            /* Se ainda não encontramos a chave */
                            if (!funcao_found_curly_brace)
                            {
                                if (isspace((unsigned char)line[i]))
                                {
                                    continue;
                                }
                                else if (line[i] == '{')
                                { /* Encontrou a chave de abertura */
                                    funcao_found_curly_brace = true;
                                }
                                else
                                { /* Qualquer outro caractere é erro */
                                    message_error("Esperado '{' após parênteses", line_number);
                                    return 1; /*O código PARA quando encontra erro*/
                                }
                            }
                            else
                            { /* Após encontrar a chave e não tiver terminado, passar p frente*/
                                break;
                            }
                        }
                    }
                }
                printf("Funcao ok\n");
                /*Fim da checagem se é funcao __xxx(){*/
            }
            else if (line[0] == 'i')
            {
                for (int i = 0; line[i] != '\0'; i++)
                {
                    if (line[i] != inteiro[i] && i < 7)
                    {
                        message_error("Inteiro escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }
                /* Verifica restante da linha */
                if (verificarVariavelInteira(line, 7, line_number) == 1)
                {
                    return 1;
                }
                printf("Inteiro ok\n");
            }
            else if (line[0] == 't')
            {
                for (int i = 0; line[i] != '\0'; i++)
                {
                    if (line[i] != texto[i] && i < 5)
                    {
                        message_error("Texto escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }
                if (verificarVariavelTexto(line, 5, line_number) == 1)
                {
                    return 1;
                }
                printf("texto ok\n");
            }
            else if (line[0] == 'd')
            {
                for (int i = 0; line[i] != '\0'; i++)
                {
                    if (line[i] != decimal[i] && i < 7)
                    {
                        message_error("Decimal escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }
                if (verificarVariavelDecimal(line, 7, line_number) == 1)
                {
                    return 1;
                }
                printf("decimal ok\n");
            }
            else if (line[0] == 'l')
            {
                for (int i = 0; line[i] != '\0'; i++)
                {
                    if (line[i] != leia[i] && i < 4)
                    {
                        message_error("Leia escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }
                int aux = 0;
                while (isspace(line[4 + aux]))
                {
                    aux++;
                }
                if (line[4 + aux] != '(')
                {
                    message_error("Falta '(' depois de leia", line_number);
                }
                if (verificarLeia(line, 5 + aux, line_number) == 1)
                { /*Funciona, mas deixa = passar*/
                    return 1;
                }
                for (int i = 4; line[i] != '\0'; i++)
                {
                    if (line[i] == '=')
                    {
                        message_error("Não é permitido atribuições no leia", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }
                printf("leia ok\n");
            }
            else if (line[0] == 'r')
            {
                /* Verifica se retorno !variavel;*/
                int i = 0;
                for (i; i < 7; i++)
                {
                    if (line[i] != retorno[i])
                    {
                        message_error("Retorno escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }
                bool has_variable = false;
                /*verificar !variavel ou espaços*/
                for (i; line[i] != '\0'; i++)
                {
                    if (isspace(line[i]))
                    {
                        continue;
                    }
                    /* Encontrou não-espaço: deve ser '!' */
                    if (line[i] == '!' && !has_variable)
                    {
                        has_variable = true;
                        i++;
                        break;
                    }
                    else
                    {
                        message_error("Esperado '!' antes da variável", line_number);
                        return 1;
                    }
                }

                if (has_variable)
                {
                    /* Verificar primeiro caractere (obrigatoriamente a-z) */
                    if (line[i] < 'a' || line[i] > 'z')
                    {
                        message_error("Após '!' deve haver letra minúscula (a-z)\n", line_number);
                        return 1;
                    }
                    else
                    {
                        i++;
                    }
                    /* Verificar caracteres subsequentes (opcionais a-z, A-Z, 0-9) */
                    while (isalnum((unsigned char)line[i]))
                    {
                        i++;
                    }
                }
                /*verifica se tem algo depois de retorno ou variavel, pode ser espaço ou ;*/
                while (line[i] != '\0')
                {
                    if (isspace((unsigned char)line[i]) || line[i] == ';')
                    {
                        if (line[i] == ';')
                        {
                            i++;
                            while (line[i] != '\0')
                            {
                                if (!isspace((unsigned char)line[i]))
                                {
                                    message_error("Caracteres inválidos após ';'", line_number);
                                    return 1;
                                }
                                i++;
                            }
                            break;
                        }
                        i++;
                    }
                    else
                    {
                        message_error("Caractere inválido após variável. Esperado espaço ou ';'", line_number);
                        return 1;
                    }
                }
                printf("retorno ok\n");
                /*Fim da verificação de retorno !variavel;*/
            }
            else if (line[0] == 'e')
            {
                /*Checando se é escreva("texto")*/
                int i = 0;
                /* Verifica se começa com "escreva" */
                for (i; i < 7; i++)
                {
                    if (line[i] != escreva[i])
                    {
                        message_error("Módulo escreva escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }
                int parenteses_control_open_escreva = 0;
                int aspas_control_open_escreva = 0;
                int len = strlen(line);
                bool aspas_control = false;
                /* Verifica restante da linha */
                for (i; line[i] != '\0'; i++)
                {
                    /* Verificar parênteses*/
                    if (parenteses_control_open_escreva == 0)
                    {
                        if (isspace((unsigned char)line[i]) || line[i] == '(')
                        { /*abre parênteses*/
                            if (line[i] == '(')
                            {
                                parenteses_control_open_escreva++;
                            }
                        }
                    }
                    else if (parenteses_control_open_escreva >= 1)
                    { /*tem que ter aspas*/
                        int quote_bytes = is_smart_quote(line, i, len); /*função para verificar as aspas diferentes*/

                        if (isspace((unsigned char)line[i]) || line[i] == '"' || quote_bytes > 0 && !aspas_control)
                        {
                            if (line[i] == '"' || quote_bytes > 0)
                            {
                                 if (quote_bytes == 3) {
                                    i = i + 3;
                                 } else {
                                    i++;
                                 }
                                aspas_control_open_escreva ++;
                                while (!aspas_control)
                                {
                                    quote_bytes = is_smart_quote(line, i, len); /*função para verificar as aspas diferentes*/

                                    if (line[i] == '"' || quote_bytes > 0)
                                    {
                                        aspas_control = true;
                                        aspas_control_open_escreva ++;

                                        if (aspas_control_open_escreva > 2)
                                        {
                                            message_error("Use apenas duas aspas", line_number);
                                            return 1;
                                        }
                                    }
                                    else if(line[i] == '/0' || line[i] == ')')
                                    {
                                        message_error("Precisa fechar as aspas\n", line_number);
                                        return 1;
                                    }
                                    i++;
                                }
                                continue;
                            }
                        }

                        else if (line[i] == ',' && aspas_control)
                        { /* Tem parâmetro */
                            i++;
                            while(isspace((unsigned char)line[i])) i++;

                            if (isspace((unsigned char)line[i]) || line[i] == '!')
                            {
                                if (line[i] == '!')
                                {
                                    Resultado res = verificarParametroFuncao(line, i, line_number);
                                    i = res.posicao;
                                    if (res.sucesso == 1)
                                    {
                                        return 1;
                                    }
                                    else
                                    {
                                        continue;
                                    }
                                }
                            }
                        }
                        else if (line[i] == ')')
                        {
                            parenteses_control_open_escreva--;
                        }
                        else if (isspace((unsigned char)line[i]) || line[i] == ';')
                        {
                            continue;
                        }
                    }
                }
                printf("escreva ok\n");
                /*Fim da checagem escreva("texto") */
            }
            if (line[0] == 's')
            {
                bool is_se = false;
                bool is_senao = false;
                int len = strlen(line);
                /* Verifica se começa com "se" ou "senao" */
                if (strncmp(line, se, 2) == 0) {
                    /* Verificar se é senao (5 caracteres) */
                    if (len >= 3 && strncmp(line, senao, 3) == 0) {
                        is_senao = true;
                    } else {
                        is_se = true;
                    }
                }

                if (is_se)
                {
                    /*Checando se é se*/
                    int i = 2;
                    /* Verifica se se(){ */
                    if (!strncmp(line, se, 2) == 0) {
                        message_error("Módulo se incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }

                    int parenteses_control_open_se = 0;

                    /* Verifica restante da linha */
                    for (i; line[i] != '\0'; i++)
                    {
                        /* Verificar parênteses*/
                        if (parenteses_control_open_se == 0)
                        {
                            if (isspace((unsigned char)line[i]) || line[i] == '(')
                            { /*abre parênteses*/
                                if (line[i] == '(')
                                {
                                    parenteses_control_open_se++;
                                }
                            }
                        }
                        else if (parenteses_control_open_se >= 1)
                        {
                            while(isspace((unsigned char)line[i]))i++;

                            int quote_bytes = is_smart_quote(line, i, len); /*função para verificar as aspas diferentes*/

                            if (line[i] == '!' || line[i] == '"' || quote_bytes>0)
                            {
                                Resultado res = verificarParametrosSe(line, i, line_number, len);
                                i = res.posicao;
                                if (res.sucesso == 1)
                                {
                                    return 1;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                            else if ((line[i] == '|' && line[i+i] == '|') || (line[i] == '&' && line[i] == '&'))
                            {
                                i++;
                                while(isspace((unsigned char)line[i]))i++;

                                int quote_bytes = is_smart_quote(line, i, len); /*função para verificar as aspas diferentes*/

                                if (line[i] == '!' || line[i] == '"' || quote_bytes>0)
                                {
                                    Resultado res = verificarParametrosSe(line, i, line_number, len);
                                    i = res.posicao;
                                    if (res.sucesso == 1)
                                    {
                                        return 1;
                                    }
                                    else
                                    {
                                        continue;
                                    }
                                }
                            }
                            else if (line[i] == ')')
                            {
                                parenteses_control_open_se--;
                            }
                            else if (isspace((unsigned char)line[i]) || line[i] == '{')
                            {
                                continue;
                            }
                        }
                    }

                    printf("se ok\n");
                    /*fim da checagem se é se*/
                }
                else if (is_senao)
                {
                    /*Checando se é senao*/
                    int i = 5;
                    if (strncmp(line, senao, 5) != 0) {
                        message_error("Módulo senao incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                    while (isspace((unsigned char)line[i])){i++;}


                     /* Se houver conteúdo após "senao" */
                    if (line[i] != '\0') {
                        /*criar função para tratar todos os casos*/
                        /*enviar a linha e a posição i*/
                    }

                    printf("senao ok\n");
                    /*fim da checagem se é senao*/
                }
                else
                {
                    message_error("Comando deve ser 'se' ou 'senao'", line_number);
                    return 1;
                }


            }
            else if (line[0] == '}' || line[0] == '\0')
            { /*Aqui tudo que pode estar sozinho na linha - condição final, que se não for atendida retorna erro*/
                printf("Fechamento de chaves ou vazio ok\n"); /* mudei pq achei confuso*/
                /*Luiz: Se ela não for atendida, ela não retorna erro. A gente não escreveu nenhum print pra isso.*/
                /*Luiz: Você só está aceitando tudo que tenha } no ínicio ou fim de txt*/
            }
            else if(line[0]=='!')/*mudando valores de variáveis, atribuições*/
            { /*Vou fazer uma função só pra isso, e aí adicionar aqui e no inteiro e decimal*/
                if(verificarOperacaoMatematicaMain==1){
                    return 1;
                } else{
                    printf("Operação com variável ok\n");
                }
            }

            line_number++;
        }

        fclose(file);
    }
    else
    {
        fprintf(stderr, "Não foi possível abrir o arquivo!\n");
    }

    return 0;
}

int carregarNaMemoria(int Memory, int MaxMemory, int size)
{
    if (Memory + size <= 0.9 * MaxMemory)
    {
        return (Memory += size);
    }
    else
    {
        if ((Memory) + size < (MaxMemory * 0.99))
        {
            printf("Alerta! Mais de 90% da Memória disponível foi utilizada");
            return (Memory += size);
        }
        else
        {
            printf("Memória cheia!!! Não foi possível carregar os bits na memória");
            return -1;
        }
    }
};

void message_error(const char *erro, int line_number)
{
    printf("Erro na linha %d: %s\n", line_number, erro);
}

/* Função que garante que todo ';' seja seguido por '\n' e retorna o nome do arquivo de saída*/
char *garantir_quebra_linha_apos_ponto_virgula(const char *arquivo_entrada)
{
    FILE *entrada = fopen(arquivo_entrada, "r");
    if (entrada == NULL)
    {
        perror("Erro ao abrir arquivo de entrada");
        exit(EXIT_FAILURE);
    }

    /* Criar nome do arquivo de saída baseado no nome de entrada*/
    char *arquivo_saida = malloc(strlen(arquivo_entrada) + 5); // +5 para "_out" e null terminator*/
    strcpy(arquivo_saida, arquivo_entrada);

    /* Adicionar "_out" antes da extensão (se houver)*/
    char *ponto = strrchr(arquivo_saida, '.');
    if (ponto != NULL)
    {
        /* Tem extensão, insere "_out" antes do ponto*/
        memmove(ponto + 4, ponto, strlen(ponto) + 1);
        memcpy(ponto, "_out", 4);
    }
    else
    {
        /* Sem extensão, apenas adiciona "_out" no final*/
        strcat(arquivo_saida, "_out");
    }

    FILE *saida = fopen(arquivo_saida, "w");
    if (saida == NULL)
    {
        perror("Erro ao criar arquivo de saída");
        fclose(entrada);
        free(arquivo_saida);
        exit(EXIT_FAILURE);
    }

    int caractere_atual;
    int proximo_caractere;

    while ((caractere_atual = fgetc(entrada)) != EOF)
    {
        if (caractere_atual == ';')
        {
            fputc(';', saida);

            /* Verifica o próximo caractere sem consumi-lo*/
            proximo_caractere = fgetc(entrada);

            if (proximo_caractere != '\n' && proximo_caractere != EOF)
            {
                fputc('\n', saida);
                fputc(proximo_caractere, saida);
            }
            else
            {
                /* Já tem '\n', apenas escreve*/
                fputc(proximo_caractere, saida);
            }
        }
        else
        {
            fputc(caractere_atual, saida);
        }
    }

    fclose(entrada);
    fclose(saida);

    return arquivo_saida;
}

/* Função para declaração de variável inteira*/
int verificarVariavelInteira(char line[], int posicao, int line_number)
{
    for (int i = posicao; line[i] != '\0'; i++)
    {
        char c = line[i];
        if (isspace(c))
        {
            /* Ignora, não há nada a fazer */
        }
        else if (c == '!')
        {
            i++;
            if (line[i] >= 'a' && line[i] <= 'z')
            {
                while (isalnum((unsigned char)line[i]))
                {
                    i++;
                }; /*verifica se o restante é alfanumerico*/
                if (line[i] == ',' && (isspace(line[i + 1])))
                { /*tem mais parâmetros que precisam ser verificados*/
                    return verificarVariavelInteira(line, i + 1, line_number);
                }
                else if (line[i] == ';' && line[i + 1] == '\0')
                {
                    return 0;
                }
                else if (line[i] == ';' && line[i + 1] == '\n')
                {
                    return 0;
                }
                else if (line[i] == ';' && isspace(line[i + 1]))
                { /*tô ignorando espaços que aparecem depois*/
                    return 0;
                }
                else if (isspace(line[i]))
                {
                    do
                    {
                        i++;
                    } while (isspace(line[i])); /*pula espaços*/
                    if (line[i] == '=')
                    {
                        i++;
                        do
                        {
                            i++;
                        } while (isspace(line[i]));
                        int result = verificarOperacaoMatematica(line, i, line_number, 0);
                        if(result == 1)
                        {
                            return 1;
                        } else if (result ==0){
                            return 0;
                        } else {
                            message_error("Não pode atribuir um valor decimal a uma variável inteira. \n", line_number);
                            return 1;
                        }
                    }
                }
                else
                {
                    message_error("Variáveis só podem conter alfanuméricos.\n", line_number);
                    return 1;
                }
            }
            else
            {
                message_error("Variáveis precisam começar com letra minúscula.\n", line_number);
                return 1;
            }
        }
        else
        {
            message_error("Falta '!' antes da variável.\n", line_number);
            return 1;
        }
    }
    message_error("Declaração de variável não encontrada.\n", line_number);
    return 1;
}

/* Função para declaração de variável de texto*/
int verificarVariavelTexto(char line[], int posicao, int line_number)
{
    for (int i = posicao; line[i] != '\0'; i++)
    {
        char c = line[i];
        if (isspace(c))
        {
            /* Ignora, não há nada a fazer */
        }
        else if (c == '!')
        {
            i++;
            if (line[i] >= 'a' && line[i] <= 'z')
            {
                while (isalnum((unsigned char)line[i]))
                {
                    i++;
                }; /*verifica se o restante é alfanumerico*/
                if (line[i] == '[')
                {
                    i++;
                    if (line[i] == '0' || !isdigit(line[i]))
                    {
                        message_error("O tamanho de um texto precisa ser um número e maior que zero\n", line_number);
                        return 1;
                    }

                    while (isdigit(line[i]))
                    {
                        i++;
                    }
                    if (line[i] != ']')
                    {
                        message_error("Não foi encontrado ']' após a variável de texto. O tamanho foi escrito incorretamente. Só são permitidos números inteiros entre '[' e ']' \n", line_number);
                        return 1;
                    }
                    i++;
                    if (line[i] == ',' && (isspace(line[i + 1])))
                    { /*tem mais parâmetros que precisam ser verificados*/
                        return verificarVariavelTexto(line, i + 1, line_number);
                    }
                    else if (line[i] == ';' && line[i + 1] == '\0')
                    {
                        return 0;
                    }
                    else if (line[i] == ';' && line[i + 1] == '\n')
                    {
                        return 0;
                    }
                    else if (line[i] == ';' && isspace(line[i + 1]))
                    { /*tô ignorando espaços que aparecem depois*/
                        return 0;
                    }
                    else if (isspace(line[i]))
                    {
                        do
                        {
                            i++;
                        } while (isspace(line[i])); /*pula espaços*/
                        if (line[i] == '=') /*vou daar a responsabilidade pra uma função*/
                        {
                            do{
                                i++;
                            }while(isspace(line[i]));

                            if (line[i] == '"' || (is_smart_quote(line, i, strlen(line)) > 0))
                            {
                                i++;
                                while(line[i]!='\0'||line[i]!='\n'){
                                    i++;
                                    if(line[i] == '"' || (is_smart_quote(line, i, strlen(line)) > 0)){
                                        i++;
                                        if(line[i]==';')
                                        {
                                            i++;
                                            while(isspace(line[i]))
                                            {
                                            i++;
                                            }
                                            if(line[i]!='\0'&&line[i]!='\n'){
                                                message_error("Só podem conter espaços depois de ponto e vírgula\n", line_number);
                                                return 1;
                                            } else {
                                                return 0;
                                            }
                                        }
                                    }
                                }
                                message_error("Erro: falta fechar aspas e/ou ponto e vírgula\n", line_number);
                                return 1;
                            }
                            else
                            {
                                message_error("Não foi encontrada aspas no texto\n", line_number);
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        message_error("Algo depois de ']' está incorreto. Tem certeza que digitou corretamente?", line_number);
                        return 1;
                    }
                }
                else
                {
                    message_error("Declaração incorreta. falta '[' ou foram usados não alfanuméricos. \n", line_number);
                    return 1;
                }
            }
            else
            {
                message_error("Variáveis precisam começar com letra minúscula.\n", line_number);
                return 1;
            }
        }
        else
        {
            message_error("Falta '!' antes da variável.\n", line_number);
            return 1;
        }
    }
    message_error("Declaração de variável não encontrada.\n", line_number);
    return 1;
}

/* Função para declaração de variável de decimais*/
int verificarVariavelDecimal(char line[], int posicao, int line_number)
{
    for (int i = posicao; line[i] != '\0'; i++)
    {
        char c = line[i];
        if (isspace(c))
        {
            /* Ignora, não há nada a fazer */
        }
        else if (c == '!')
        {
            i++;
            if (line[i] >= 'a' && line[i] <= 'z')
            {
                while (isalnum((unsigned char)line[i]))
                {
                    i++;
                }; /*verifica se o restante é alfanumerico*/
                if (line[i] == '[')
                {
                    i++;
                    if (line[i] == '0' || !isdigit(line[i]))
                    {
                        message_error("O tamanho de um decimal precisa ser um número e maior que zero antes do '.'\n", line_number);
                        return 1;
                    }

                    while (isdigit(line[i]))
                    {
                        i++;
                    }
                    if (line[i] != '.')
                    {
                        message_error("Não foi encontrado '.' após na variável decimal. O tamanho foi escrito incorretamente. \n", line_number);
                        return 1;
                    }
                    i++;
                    if (line[i] == '0' || !isdigit(line[i]))
                    {
                        message_error("O tamanho de um decimal precisa ser um número e maior que zero antes do '.'\n", line_number);
                        return 1;
                    }

                    while (isdigit(line[i]))
                    {
                        i++;
                    }
                    if (line[i] != ']')
                    {
                        message_error("Não foi encontrado ']' após na variável decimal. O tamanho foi escrito incorretamente. Só são permitidos números e pontos entre '[' e ']'. \n", line_number);
                        return 1;
                    }
                    i++;
                    if (line[i] == ',' && (isspace(line[i + 1])))
                    { /*tem mais parâmetros que precisam ser verificados*/
                        return verificarVariavelDecimal(line, i + 1, line_number);
                    }
                    else if (line[i] == ';' && line[i + 1] == '\0')
                    {
                        return 0;
                    }
                    else if (line[i] == ';' && line[i + 1] == '\n')
                    {
                        return 0;
                    }
                    else if (line[i] == ';' && isspace(line[i + 1]))
                    { /*tô ignorando espaços que aparecem depois*/
                        return 0;
                    }
                    else if (isspace(line[i]))
                    {
                        do
                        {
                            i++;
                        } while (isspace(line[i])); /*pula espaços*/
                        if (line[i] == '=')
                        {
                            i++;
                            if (verificarOperacaoMatematica(line,i,line_number,0)==1){
                                return 1;
                            } else {
                                return 0;
                            }
                        }
                    }
                    else
                    {
                        message_error("Algo depois de ']' está incorreto. Tem certeza que digitou corretamente?", line_number);
                        return 1;
                    }
                }
                else
                {
                    message_error("Declaração incorreta. falta '[' ou foram usados não alfanuméricos. \n", line_number);
                    return 1;
                }
            }
            else
            {
                message_error("Variáveis precisam começar com letra minúscula.\n", line_number);
                return 1;
            }
        }
        else
        {
            message_error("Falta '!' antes da variável.\n", line_number);
            return 1;
        }
    }
    message_error("Declaração de variável não encontrada.\n", line_number);
    return 1;
}

/* Função para verificação de variável dentro de leia*/
int verificarLeia(char line[], int posicao, int line_number)
{
    int i = posicao;
    int len = strlen(line);
    int variaveis = 0;

    while (1)
    {
        while (i < len && isspace(line[i]))
            i++; /* ignora espaços*/
        if (i >= len)
            break;

        if (line[i] != '!')
        {
            if (variaveis > 0 && line[i] == ')')
                break;
            message_error("Esperado '!' antes da variável que deveria aparecer depois de ',' ou '('.\n", line_number);
            return 1;
        }
        i++;

        if (i >= len || !(line[i] >= 'a' && line[i] <= 'z'))
        {
            message_error("Variáveis devem começar com letra minúscula.\n", line_number);
            return 1;
        }
        i++;

        while (i < len && isalnum(line[i]))
            i++;
        variaveis++;

        while (i < len && isspace(line[i]))
            i++;
        if (i >= len)
            break;

        if (line[i] == ')')
            break;
        if (line[i] != ',')
        {
            message_error("Esperado ',' ou ')' após variável.\n", line_number);
            return 1;
        }
        i++;
    }

    if (i >= len || line[i] != ')')
    {
        message_error("Esperado ')' após variáveis.\n", line_number);
        return 1;
    }
    i++;

    while (i < len && isspace(line[i]))
        i++;

    if (i >= len || line[i] != ';')
    {
        message_error("Esperado ';' após comando.\n", line_number);
        return 1;
    }
    i++;

    while (i < len && isspace(line[i]) && line[i] != '\n')
        i++;

    if (i < len && line[i] != '\0' && line[i] != '\n')
    {
        message_error("Caracteres extras após ';'.\n", line_number);
        return 1;
    }

    return 0;
}

/*função para tratar parametro das funcoes*/
Resultado verificarParametroFuncao(char line[], int posicao, int line_number)
{
    int i = posicao;

    while (isspace((unsigned char)line[i]))
        i++; /* Ignorar espaços iniciais */

    /* Verificar marcador obrigatório '!' */
    if (line[i] != '!')
    {
        message_error("Esperado '!' antes da variável\n", line_number);
        return (Resultado){i, 1};
    }
    i++; /* Avançar após o '!' */

    /* Verificar primeiro caractere (obrigatoriamente a-z) */
    if (line[i] < 'a' || line[i] > 'z')
    {
        message_error("Após '!' deve haver letra minúscula (a-z)\n", line_number);
        return (Resultado){i, 1};
    }
    i++; /* Avançar após a primeira letra */

    /* Verificar caracteres subsequentes (opcionais a-z, A-Z, 0-9) */
    while (isalnum((unsigned char)line[i]))
    {
        i++;
    }

    if (!(isalnum((unsigned char)line[i]) || line[i] == '(' || line[i] == ')' || line[i] == ','))
    {
        message_error("Nome do parâmetro escrito com caracter inválido", line_number);
        return (Resultado){i, 1};
    }

    /* Ignorar espaços após variável */
    while (isspace((unsigned char)line[i]))
        i++;

    /* Verificar se há próximo parâmetro */
    if (line[i] == ',')
    {
        i++;                                                   /* Avançar a vírgula */
        return verificarParametroFuncao(line, i, line_number); /* Chamada recursiva */
    }

    return (Resultado){i, 0}; /* Sucesso */
}

/*função para tratar parametros de para*/
Resultado verificarParametrosPara(char line[], int posicao, int line_number)
{
    int i = posicao;

    while (isspace((unsigned char)line[i]))
        i++; /* Ignorar espaços iniciais */

    /* Verificar marcador obrigatório '!' */
    if (line[i] != '!')
    {
        message_error("Esperado '!' antes da variável\n", line_number);
        return (Resultado){i, 1};
    }
    i++; /* Avançar após o '!' */

    /* Verificar primeiro caractere (obrigatoriamente a-z) */
    if (line[i] < 'a' || line[i] > 'z')
    {
        message_error("Após '!' deve haver letra minúscula (a-z)\n", line_number);
        return (Resultado){i, 1};
    }
    i++; /* Avançar após a primeira letra */

    /* Verificar caracteres subsequentes (opcionais a-z, A-Z, 0-9) */
    while (isalnum((unsigned char)line[i]))
    {
        i++;
    }

    while (isspace((unsigned char)line[i]))
        i++; /* Ignorar espaços iniciais */

    /* Verificar operadores (=, +, -, <=, ==, >=, etc) */
    int has_operator = 0;

    /* Operadores de 2 caracteres (<=, ==, >=, !=) */
    if ((line[i] == '<' && line[i + 1] == '=') ||
        (line[i] == '>' && line[i + 1] == '=') ||
        (line[i] == '=' && line[i + 1] == '=') ||
        (line[i] == '<' && line[i + 1] == '>'))
    {
        i += 2; // Avança ambos caracteres
        has_operator = 1;
    }
    else if (line[i] == '=' || line[i] == '+' || line[i] == '-' ||
             line[i] == '<' || line[i] == '>')
    { /* Operadores de 1 caractere (=, +, -, <, >) */
        i++;
        has_operator = 1;
    }

    /* Se encontrou operador, verificar valor */
    if (has_operator)
    {
        while (isspace((unsigned char)line[i]))
            i++; /* Ignorar espaços */

        /* Verificar se é número */
        if (isdigit((unsigned char)line[i]))
        {
            while (isdigit((unsigned char)line[i]))
                i++;
        }
        /* Verificar se é outra variável */
        else if (line[i] == '!')
        {
            /* Chamada recursiva para verificar próxima variável */
            Resultado res = verificarParametrosPara(line, i, line_number);
            if (res.sucesso != 0)
                return res;
            i = res.posicao;
        }
        else
        {
            message_error("Valor inválido após operador\n", line_number);
            return (Resultado){i, 1};
        }
    }

    while (isspace((unsigned char)line[i]))
        i++; /* Ignorar espaços finais */

    if (line[i] == ';' || line[i] == ')' || line[i] == '\0')
    {
        return (Resultado){i, 0}; /* Sucesso */
    }
    else
    {
        message_error("Caractere inválido no parâmetro\n", line_number);
        return (Resultado){i, 1};
    }
}


int verificarOperacaoMatematica(char line[], int posicao, int line_number, int flagTemPonto) /*verifica depois de =*/
{
    for (int i = posicao; line[i] != '\0'; i++)
    {
        while (isspace(line[i])){
            i++;
        };
        if (line[i] == '!')
        {
            /*regra para variáveis*/
                i++;
                if (line[i] >= 'a' && line[i] <= 'z')
                {
                    while (isalnum((unsigned char)line[i]))
                    {
                        i++;
                    }; /*verifica se o restante é alfanumerico*/
                    if(isspace(line[i])){
                        i++;
                        while (isspace(line[i])){
                                i++;
                        }
                    }
                        if (line[i] == '+' && line[i+1] == '+')
                        { /*verificar se tem apenas uma variável de fato*/
                            if (line[i+2]!=';'){
                                message_error("Falta ponto e virgula após a contração. \n", line_number);
                                return 1;
                            }
                            i--;
                            while (isalnum((unsigned char)line[i])){
                                i--;
                            }
                            if(line[i]!='!'){
                                message_error("Contrações só são permitidas para variáveis. \n", line_number);
                                return 1;
                            } else {
                                i--;
                                while (isspace(line[i])){
                                    i--;
                                }
                                if(line[i]=='='){
                                    if(flagTemPonto==0){
                                        return 0;
                                    } else {
                                        return (-1);
                                    }
                                }
                            }
                        } else if (line[i] == '-' && line[i+1] == '-')
                        { /*verificar se tem apenas uma variável de fato*/
                            if (line[i+2]!=';'){
                                message_error("Falta ponto e virgula após a contração. \n", line_number);
                                return 1;
                            }
                            i--;
                            while (isalnum((unsigned char)line[i])){
                                i--;
                            }
                            while (isspace(line[i])){
                                i--;
                            }
                            if(line[i]!='!'){
                                message_error("Contrações só são permitidas para variáveis. \n", line_number);
                                return 1;
                            } else {
                                i--;
                                while (isspace(line[i])){
                                    i--;
                                }
                                if(line[i]=='='){
                                    if(flagTemPonto==0){
                                        return 0;
                                    } else {
                                        return (-1);
                                    }
                                }
                            }
                        } else if(line[i]=='+'||line[i]=='-'||line[i]=='['||line[i]==']')
                        {
                            return verificarOperacaoMatematica(line,i,line_number,flagTemPonto);
                        }
                        else if (line[i] == ';' && line[i + 1] == '\0')
                        {
                            if(flagTemPonto==0){
                                return 0;
                            } else {
                                return (-1);
                            }
                        }
                        else if (line[i] == ';' && line[i + 1] == '\n')
                        {
                            if(flagTemPonto==0){
                                return 0;
                            } else {
                                return (-1);
                            }
                        }
                        else if (line[i] == ';' && isspace(line[i + 1]))
                        {
                            while(isspace(line[i+1])){
                                i++;
                            }
                            if(line[i+1]!='\n'&&line[i+1]!='\0'){
                                message_error("Depois de ';' foi encontrado algo além de espaços", line_number);
                            }
                            if(flagTemPonto==0){
                                return 0;
                            } else {
                                return (-1);
                            }
                        }

                        else
                        {
                            message_error("Só são permitidos caracteres alfanuméricos nas variáveis\n", line_number);
                            return 1;
                        }
                    }
                else
                {
                    message_error("Variáveis precisam começar com letra minúscula.\n", line_number);
                    return 1;
                }
        }
        else if (isdigit(line[i]))
        {
            if(line[i]=='0' && isdigit(line[i+1])){
                message_error("Só aceitamos números decimais. O único número que pode começar com zero é zero.\n", line_number);
                return 1;
            }

            while (line[i] != ';' && line[i] != '\0' && line[i] != '\n')
            {
                if (!isdigit(line[i]))
                {
                    if(line[i]=='.'){
                        flagTemPonto = 1;
                        i++;
                        if(isdigit(line[i])){
                            while (line[i] != ';' && line[i] != '\0' && line[i] != '\n'){
                                i++;
                            };
                            break;
                        }else if (line[i] == ']'||line[i] == '['||line[i] == '+'||line[i] == '-'){
                            return verificarOperacaoMatematica(line, i+1, line_number, flagTemPonto);
                        }else{
                            message_error("Falta um número depois do ponto", line_number);
                            return 1;
                        }
                    }else if (line[i] == ']'||line[i] == '['||line[i] == '+'||line[i] == '-'){
                        return verificarOperacaoMatematica(line, i+1, line_number, flagTemPonto);
                    }
                }
                i++;
            };
            if (line[i] == ';')
            {
                do
                {
                    i++;
                } while (isspace(line[i + 1]));
                if (!line[i + 1] == '\n' && !line[i + 1] == '\0')
                {
                    message_error("É necessário que a declaração termine com ';', sem nada após\n", line_number);
                    return 1;
                }
                else
                {
                    if(flagTemPonto==1){
                        return (-1);
                    }
                    return 0;
                }
            } else {
                message_error("Não foi encontrado ponto e vírgula após seu número\n", line_number);
                return 1;
            }
        }
        else if (line[i] == '-')
        {
            i++;
            if (line[i] == '-')
            {
                i++;
                if (line[i] == '!')
                {
            /*regra para variáveis*/
                i++;
                if (line[i] >= 'a' && line[i] <= 'z')
                {
                    while (isalnum((unsigned char)line[i]))
                    {
                        i++;
                    }; /*verifica se o restante é alfanumerico*/
                    if(isspace(line[i])){
                        i++;
                        while (isspace(line[i])){
                                i++;
                        }
                    }
                        if (line[i] == ';' && line[i + 1] == '\0')
                        {
                            if(flagTemPonto==0){
                                return 0;
                            } else {
                                return (-1);
                            }
                        }
                        else if (line[i] == ';' && line[i + 1] == '\n')
                        {
                            if(flagTemPonto==0){
                                return 0;
                            } else {
                                return (-1);
                            }
                        }
                        else if (line[i] == ';' && isspace(line[i + 1]))
                        {
                            while(isspace(line[i+1])){
                                i++;
                            }
                            if(line[i+1]!='\n'&&line[i+1]!='\0'){
                                message_error("Depois de ';' foi encontrado algo além de espaços", line_number);
                            }
                            if(flagTemPonto==0){
                                return 0;
                            } else {
                                return (-1);
                            }
                        }

                        else
                        {
                            message_error("Algo está incorreto. Tem certeza que digitou corretamente?", line_number);
                            return 1;
                        }
                    }
                else
                {
                    message_error("Variáveis precisam começar com letra minúscula.\n", line_number);
                    return 1;
                }
                }
                else
                {
                    message_error("Contração feita de forma incorreta\n", line_number);
                    return 1;
                }
            }
        }
        else if (line[i] == '+')
        {
            i++;
            if (line[i] == '+')
            {
                i++;
                if (line[i] == '!')
                {
            /*regra para variáveis*/
                i++;
                if (line[i] >= 'a' && line[i] <= 'z')
                {
                    while (isalnum((unsigned char)line[i]))
                    {
                        i++;
                    }; /*verifica se o restante é alfanumerico*/
                    if(isspace(line[i])){
                        i++;
                        while (isspace(line[i])){
                                i++;
                        }
                    }
                        if (line[i] == ';' && line[i + 1] == '\0')
                        {
                            if(flagTemPonto==0){
                                return 0;
                            } else {
                                return (-1);
                            }
                        }
                        else if (line[i] == ';' && line[i + 1] == '\n')
                        {
                            if(flagTemPonto==0){
                                return 0;
                            } else {
                                return (-1);
                            }
                        }
                        else if (line[i] == ';' && isspace(line[i + 1]))
                        {
                            while(isspace(line[i+1])){
                                i++;
                            }
                            if(line[i+1]!='\n'&&line[i+1]!='\0'){
                                message_error("Depois de ';' foi encontrado algo além de espaços", line_number);
                            }
                            if(flagTemPonto==0){
                                return 0;
                            } else {
                                return (-1);
                            }
                        }

                        else
                        {
                            message_error("Algo está incorreto. Tem certeza que digitou corretamente?", line_number);
                            return 1;
                        }
                    }
                else
                {
                    message_error("Variáveis precisam começar com letra minúscula.\n", line_number);
                    return 1;
                }
                }
                else
                {
                    message_error("Contração feita de forma incorreta\n", line_number);
                    return 1;
                }
            }
        }
        else if (line[i] == '['||line[i]==']')
        {
            return verificarOperacaoMatematica(line, i + 1, line_number, flagTemPonto);
        }
        else if (line[i] == '\0' || line[i] == '\n')
        {
            message_error("Falta algo depois de '='. Você pode ter esquecido o ponto e vírgula ou alguma parte da atribuição \n", line_number);
            return 1;
        }
        else if (line[i] == ';' && line[i-1] == ']')
        {
            if(flagTemPonto==1){
                return (-1);
            }
            return 0;
        }
        else
        {
            message_error("Variaveis ou numeros depois de '=' escritos de forma incorreta \n", line_number);
            printf("%c %c %c", line[i-2], line[i-1], line[i]);
            return 1;
        }
    }
    return 1;
}

/*Função para verificar tipos de aspas duplas diferentes*/
int is_smart_quote(const char *str, int pos, int length)
{
    /* Verifica se há pelo menos 3 bytes disponíveis */
    if (pos + 2 >= length) {
        return 0;
    }

    /* Verifica usando comparação de strings segura */
    if (strncmp(str + pos, "\xE2\x80\x9C", 3) == 0) {  // “
        return 3;
    }
    if (strncmp(str + pos, "\xE2\x80\x9D", 3) == 0) {  // ”
        return 3;
    }
    return 0;
}

/*função para tratar parametros de se*/
Resultado verificarParametrosSe(char line[], int posicao, int line_number, int len)
{
    int i = posicao;
    int aspas_control_open_se= 0;
    bool aspas_control = false;

    while (isspace((unsigned char)line[i]))
        i++; /* Ignorar espaços iniciais */

    /* Verificar iniício */
    int quote_bytes = is_smart_quote(line, i, len); /*função para verificar as aspas diferentes*/
    /*Se tem aspas, comparação é texto*/
    if (line[i] == '"' || quote_bytes > 0 && !aspas_control)
    {
        if (line[i] == '"' || quote_bytes > 0)
        {
            if (quote_bytes == 3)
            {
                i = i + 3;
            }
            else
            {
                i++;
            }
            aspas_control_open_se ++;
            while (!aspas_control)
            {
                quote_bytes = is_smart_quote(line, i, len); /*função para verificar as aspas diferentes*/

                if (line[i] == '"' || quote_bytes > 0)
                {
                    aspas_control = true;
                    aspas_control_open_se ++;

                    if (aspas_control_open_se > 2)
                    {
                        message_error("Use apenas duas aspas", line_number);
                        return (Resultado){i, 1};
                    }
                }
                else if(line[i] == '/0' || line[i] == ')')
                {
                    message_error("Precisa fechar as aspas\n", line_number);
                    return (Resultado){i, 1};
                }
                i++;
             }
        }
    }
    else if (line[i] == '!')
    {
        i++;
        /* Verificar primeiro caractere (obrigatoriamente a-z) */
        if (line[i] < 'a' || line[i] > 'z')
        {
            message_error("Após '!' deve haver letra minúscula (a-z)\n", line_number);
            return (Resultado){i, 1};
        }
        i++; /* Avançar após a primeira letra */

        /* Verificar caracteres subsequentes (opcionais a-z, A-Z, 0-9) */
        while (isalnum((unsigned char)line[i])){i++;}
    }
    else {
        message_error("Esperado variável ou texto\n", line_number);
        return (Resultado){i, 1};
    }

    while (isspace((unsigned char)line[i])){i++;} /* Ignorar espaços iniciais */

    /* Verificar operadores */
    int has_operator = 0;

    /* Operadores de 2 caracteres (==, <>, <=, >=) */
    if ((line[i] == '<' && line[i + 1] == '=') ||
        (line[i] == '>' && line[i + 1] == '=') ||
        (line[i] == '=' && line[i + 1] == '=') ||
        (line[i] == '<' && line[i + 1] == '>'))
    {
        i += 2; // Avança ambos caracteres
        has_operator = 1;
    }
    else if (line[i] == '<' || line[i] == '>')
    { /* Operadores de 1 caractere (<, >) */
        i++;
        has_operator = 1;
    }

    /* Se encontrou operador, verificar valor */
    if (has_operator)
    {
        while (isspace((unsigned char)line[i])){i++;} /* Ignorar espaços */

        /* Verificar se é número */
        if (isdigit((unsigned char)line[i]))
        {
            while (isdigit((unsigned char)line[i]))
                i++;
        }
        /* Verificar se é outra variável ou texto */
        int quote_bytes = is_smart_quote(line, i, len); /*função para verificar as aspas diferentes*/
        if (line[i] == '!' || line[i] == '"' || quote_bytes > 0)
        {
            /* Chamada recursiva para verificar próxima variável */
            Resultado res = verificarParametrosPara(line, i, line_number);
            if (res.sucesso != 0)
                return res;
            i = res.posicao;
        }
        else
        {
            message_error("Valor inválido após operador\n", line_number);
            return (Resultado){i, 1};
        }
    }

    while (isspace((unsigned char)line[i]))
        i++; /* Ignorar espaços finais */

    if (line[i] == '&' || line[i] == '|' || line[i] == ')' || line[i] == '\0')
    {
        return (Resultado){i, 0}; /* Sucesso */
    }
    else
    {
        message_error("Caractere inválido no parâmetro\n", line_number);
        return (Resultado){i, 1};
    }
}

int verificarOperacaoMatematicaMain(char line[], int posicao, int line_number)
{
    for (int i = posicao; line[i] != '\0'; i++)
    {
        char c = line[i];
        if (isspace(c))
        {
            /* Ignora, não há nada a fazer */
        }
        else if (c == '!')
        {
            i++;
            if (line[i] >= 'a' && line[i] <= 'z')
            {
                while (isalnum((unsigned char)line[i]))
                {
                    i++;
                }; /*verifica se o restante é alfanumerico*/
                if (line[i] == ',' && (isspace(line[i + 1])))
                { /*tem mais parâmetros que precisam ser verificados*/
                    return verificarOperacaoMatematicaMain(line, i + 1, line_number);
                }
                else if (line[i] == ';' && line[i + 1] == '\0')
                {
                    return 0;
                }
                else if (line[i] == ';' && line[i + 1] == '\n')
                {
                    return 0;
                }
                else if (line[i] == ';' && isspace(line[i + 1]))
                { /*tô ignorando espaços que aparecem depois*/
                    return 0;
                }
                else if (isspace(line[i]))
                {
                    do
                    {
                        i++;
                    } while (isspace(line[i])); /*pula espaços*/
                    if (line[i] == '=')
                    {
                        i++;
                        do
                        {
                            i++;
                        } while (isspace(line[i]));
                        if (line[i] == '"' || (is_smart_quote(line, i, strlen(line)) > 0))
                        {
                            i++;
                            while(line[i]!='\0'||line[i]!='\n'){
                                i++;
                                if(line[i] == '"' || (is_smart_quote(line, i, strlen(line)) > 0)){
                                    i++;
                                    if(line[i]==';')
                                    {
                                        i++;
                                        while(isspace(line[i]))
                                        {
                                        i++;
                                        }
                                        if(line[i]!='\0'||line[i]!='\n'){
                                            message_error("Só podem conter espaços depois de ponto e vírgula\n", line_number);
                                            return 1;
                                        } else {
                                            return 0;
                                        }
                                    }
                                }
                            }
                            message_error("Erro: falta fechar aspas e/ou ponto e vírgula\n", line_number);
                            return 1;
                        }
                        else if(verificarOperacaoMatematica(line, i, line_number, 0) == 1)
                        {
                            return 1;
                        } else{
                            return 0;
                        }
                    }
                }
                else
                {
                    message_error("Variáveis só podem conter alfanuméricos.\n", line_number);
                    return 1;
                }
            }
            else
            {
                message_error("Variáveis precisam começar com letra minúscula.\n", line_number);
                return 1;
            }
        }
        else
        {
            message_error("Falta '!' antes da variável.\n", line_number);
            return 1;
        }
    }
    message_error("Declaração de variável não encontrada.\n", line_number);
    return 1;
}
