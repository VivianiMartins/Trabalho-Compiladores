#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

/*----------------------------------------------------------------------------------------------------------*/
/*Tamanho total de memória*/
#define MAX_MEMORY_KB 2048
#define MAX_MEMORY_BYTES (MAX_MEMORY_KB * 1024)

/*Tamanho das variáveis aceitáveis*/
#define INTEIRO_MEMORY_BYTES sizeof(int)          /* Normalmente 4 bytes */
#define DECIMAL_MEMORY_BYTES sizeof(float)        /* Normalmente 4 bytes */
#define TEXTO_EACH_CHAR_MEMORY_BYTES sizeof(char) /* Sempre 1 byte */

/*----------------------------------------------------------------------------------------------------------*/
/*Structs*/
/* Estrutura para retornar dois valores em funções controladoras do compilador*/
typedef struct
{
    int posicao;
    int sucesso;
} Resultado;

/*Estrutura para representar um elemento na pilha de balanceamento*/
typedef struct No
{
    char delimitador; /* Caractere do delimitador ('{', '(', '[', '"') */
    int linha, pos;   /* Linha onde foi encontrado, Posição na linha */
    struct No* next;  /* Próximo elemento */
} No;

/* Estrutura para funções declaradas */
typedef struct Funcao {
    char nome[64];
    int linha_declaracao;
    int num_parametros;
    char parametros[10][64]; /* até 10 parâmetros*/
    struct Funcao* proxima;
} Funcao;

/* Estrutura para árvore da tabela de símbolos*/
typedef struct Node {
    char *nome;
    char *tipo;
    float tamanho;
    char *valor;
    struct Node *esq;
    struct Node *dir;
    int altura;
} Node;
/*----------------------------------------------------------------------------------------------------------*/
/*Declaração de funções*/
int varredura_principal(FILE *file,char *line , int *line_number, int *cont_principal, int is_function, int is_se, int is_para); /*função que irá varrer cada linha, testar cada*/
int carregarNaMemoria(int Memory, int MaxMemory, int size);
void message_error(const char *erro, int *line_number); /*função para retorno de erro*/
char *garantir_quebra_linha_apos_ponto_virgula(const char *arquivo_entrada);
int verificarVariavelInteira(char line[], int posicao, int *line_number); /*função para tratar parte de inteiro*/
int verificarVariavelTexto(char line[], int posicao, int *line_number);   /*função para tratar parte de texto*/
int verificarVariavelDecimal(char line[], int posicao, int *line_number); /*função para tratar parte de decimal*/
int verificarLeia(char line[], int posicao, int *line_number);            /*função para tratar parte de leia*/
int is_smart_quote(const char *str, int pos, int length);               /*Função para verificar tipos de aspas duplas diferentes*/
int verificarOperacaoMatematicaMain(char line[], int posicao, int *line_number);/*função para operacoes em geral no main*/
int verificarOperacaoMatematica(char line[], int posicao, int *line_number, int flagTemPonto);/*função para operacoes matematicas no inteiro e no decimall*/
int verificarBalanceamento(FILE* file); /*função para verificar duplo balanceamento*/
int varredura_mesma_linha(char *line, int *line_number, int i); /*função que irá varrer o restante da linha, em alguns casos*/

/*funções que utilizam a struct Resultado*/
Resultado verificarParametroFuncao(char line[], int posicao, int *line_number); /*função para tratar parametro das funcoes*/
Resultado verificarParametrosPara(char line[], int posicao, int *line_number);  /*função para tratar parametros de para*/
Resultado verificarParametrosSe(char line[], int posicao, int *line_number, int len); /*função para tratar parametros de se*/

/*funções que utilizam a struct Funcao*/
int validar_nome_variavel(char *s); /* Função auxiliar para validar nomes de variáveis nos parâmetros */
Funcao* encontrar_funcoes(FILE *file);/* Função para encontrar funções no arquivo */

/*Funções para tabela de símbolos*/
Node* criar_no(const char *nome, const char *tipo, float tamanho, const char *valor);
int altura(Node *n);
int max_int(int a, int b);
Node* rotacao_direita(Node *y);
Node* rotacao_esquerda(Node *x);
int fator_balanceamento(Node *n);
Node* inserir_no(Node *raiz, const char *nome, const char *tipo, float tamanho, const char *valor);
Node* buscar_no(Node *raiz, const char *nome);
Node* min_valor_no(Node *n);
Node* remover_no(Node *raiz, const char *nome);
Node* alterar_no(Node *raiz, const char *nome_antigo, const char *novo_nome, const char *novo_tipo, int novo_tamanho, const char *novo_valor);
void inorder(Node *raiz);
void liberar_arvore(Node *raiz);
char* duplicar_string(const char *s);
char *substring(const char *str, int inicio, int tamanho);
int extrair_e_printar_palavras(char *line, int posicao_atual, Node *encontrado, int *line_number);
int extrair_e_atualizar_palavras(char *line, int posicao_atual, Node *encontrado, int *line_number);

/*----------------------------------------------------------------------------------------------------------*/
/*Criação da varíavel global da tabela de símbolos*/
Node *raiz = NULL;

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

    /*Inserindo palavras reservadas*/
    raiz = inserir_no(raiz, "principal", "reservada", 0, "");
    raiz = inserir_no(raiz, "funcao", "reservada", 0, "");
    raiz = inserir_no(raiz, "leia", "reservada", 0, "");
    raiz = inserir_no(raiz, "escreva", "reservada", 0, "");
    raiz = inserir_no(raiz, "se", "reservada", 0, "");
    raiz = inserir_no(raiz, "senao", "reservada", 0, "");
    raiz = inserir_no(raiz, "para", "reservada", 0, "");

    if (file != NULL)
    {
        int balanceado = verificarBalanceamento(file);  /*verificação do duplo balanceamento - SINTÁTICO*/
        if (balanceado != 0) {
            return 1;
        }
        printf("Duplo balanceamento ok\n\n");
        rewind(file); /* Volta para o início do arquivo para reprocessar*/

        /*guardando funções existentes*/
        Funcao *lista_funcoes = encontrar_funcoes(file);
        /* Contar quantas funções foram encontradas */
        int count_funcoes = 0;
        Funcao *temp = lista_funcoes;
        while (temp != NULL) {
            count_funcoes++;
            temp = temp->proxima;
        }
        /* Imprimir apenas se houver funções */
        if (count_funcoes > 0) {
            printf("Funcoes encontradas (%d):\n", count_funcoes);
            Funcao *atual = lista_funcoes;
            while (atual != NULL) {
                printf("- %s (linha %d, %d parametros)\n", atual->nome, atual->linha_declaracao, atual->num_parametros);
                for (int i = 0; i < atual->num_parametros; i++) {
                    printf("  Param %d: %s\n", i+1, atual->parametros[i]);
                }
                atual = atual->proxima;
            }
            printf("\n"); /* linha em branco após a listagem */
        }
        rewind(file);

        char line[256];
        int line_number = 1;          /*número da linha em questão*/
        long start_pos = ftell(file); /*Posição inicial (0)*/
        size_t memory = 0;            /*memória*/
        size_t line_size = 0;         /*tamanho de cada linha que irei ler*/
        int cont_principal = 0;         /*controle de principal - SINTÁTICO*/

        int resultado_final = varredura_principal(file, line ,&line_number, &cont_principal, 0, 0, 0);


        if (resultado_final == 0)
        {
            if(cont_principal == 0)
            {
                message_error("Módulo principal inexistente", &line_number);
                fclose(file); return 1;
            }
            printf("Análise léxica e sintática ok\n\n");
        } else
        {
            fclose(file); return 1;
        }


        fclose(file);
    }
    else
    {
        fprintf(stderr, "Não foi possível abrir o arquivo!\n");
        return 1;
    }

    printf("Árvore (inorder): ");
    inorder(raiz);

    return 0;
}

/*----------------------------------------------------------------------------------------------------------*/
/*funções*/
/*função que irá varrer cada linha, testar cada*/
int varredura_principal(FILE *file,char *line , int *line_number, int *cont_principal, int is_function, int is_se, int is_para) {
    /*palavras reservadas - LÉXICO*/
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

    /*controladores para função*/
    bool retorno_control = false;
    bool curly_control = false;

    while (fgets(line, 256, file))
    { /*Coloquei em loop pra ficar verificando*/
        printf("Linha %d: %s", (*line_number), line); /*printa linha por linha*/

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
                /* Verifica se principal(){ - LÉXICO*/
                for (i; i < 9; i++)
                {
                    if (line[i] != principal[i])
                    {
                        message_error("Módulo principal escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }

                /*SINTÁTICO*/
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
                (*cont_principal)++; /*encontrou um principal*/

                if ((*cont_principal) > 1)
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
                /* Verifica para(){  - LÉXICO*/
                for (i; i < 4; i++)
                {
                    if (line[i] != para[i])
                    {
                        message_error("Módulo para escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }

                /*SINTÁTICO*/
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
                message_error("Esperado: Principal ou Para", line_number);
                return 1; /*O código PARA quando encontra erro*/
            }
        }
        else if (line[0] == 'f' && !is_function && !is_se && !is_para) /*Isso será feito antes e separado, para que já fique salvo o nome da função?*/
        {/*percorrer novamente verificando as funções, se estiverem ok, salvar o nome delas, qual linha está*/
            /*Checando se é funcao __xxx(){*/
            int i = 0;
            /* Verifica se começa com "funcao"  - LÉXICO*/
            for (i; i < 6; i++)
            {
                if (line[i] != funcao[i])
                {
                    message_error("Módulo funcao escrito incorretamente", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
            }

            /*SINTÁTICO*/ /*AQUI TEM QUE TERMINAR DE FAZER*/
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
                                if (line[i] == ')')
                                {
                                    parenteses_control_open_funcao--;
                                }
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
                            if (isspace((unsigned char)line[i])){
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
                        if (funcao_found_curly_brace)
                        { /*estamos dentor da função, verificar tudo o que tem*/
                            (*line_number)++; /*tem que fazer isso para contar a próxima linha*/
                            int dentroFuncao = varredura_principal(file, line, line_number, cont_principal, 1, 0, 0);

                            if (dentroFuncao != 0)
                            {
                                message_error("Função construída incorretamente", line_number);
                                return 1;
                            }
                        }
                    }
                }
            }
            printf("Funcao ok\n\n");
            /*Fim da checagem se é funcao __xxx(){*/
        }
        else if (line[0] == 'i')
        {
            /*LÉXICO*/
            for (int i = 0; line[i] != '\0'; i++)
            {
                if (line[i] != inteiro[i] && i < 7)
                {
                    message_error("Inteiro escrito incorretamente", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
            }
            /* Verifica restante da linha - SINTÁTICO*/
            if (verificarVariavelInteira(line, 7, line_number) == 1)
            {
                return 1;
            }
            printf("Inteiro ok\n");
        }
        else if (line[0] == 't')
        {
            /*LÉXICO*/
            for (int i = 0; line[i] != '\0'; i++)
            {
                if (line[i] != texto[i] && i < 5)
                {
                    message_error("Texto escrito incorretamente", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
            }
            /*SINTÁTICO*/
            if (verificarVariavelTexto(line, 5, line_number) == 1)
            {
                return 1;
            }
            printf("texto ok\n");
        }
        else if (line[0] == 'd')
        {
            /*LÉXICO*/
            for (int i = 0; line[i] != '\0'; i++)
            {
                if (line[i] != decimal[i] && i < 7)
                {
                    message_error("Decimal escrito incorretamente", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
            }
            /*SINTÁTICO*/
            if (verificarVariavelDecimal(line, 7, line_number) == 1)
            {
                return 1;
            }
            printf("decimal ok\n");
        }
        else if (line[0] == 'l')
        {
            /*LÉXICO*/
            for (int i = 0; line[i] != '\0'; i++)
            {
                if (line[i] != leia[i] && i < 4)
                {
                    message_error("Leia escrito incorretamente", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
            }
            /*SINTÁTICO*/
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
        else if (line[0] == 'e')
        {
            /*Checando se é escreva("texto")*/
            int i = 0;
            /* Verifica se começa com "escreva" - LÉXICO*/
            for (i; i < 7; i++)
            {
                if (line[i] != escreva[i])
                {
                    message_error("Módulo escreva escrito incorretamente", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
            }
            /*SINTÁTICO*/
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

                    if (isspace((unsigned char)line[i]) || line[i] == '"' ||  quote_bytes > 0 && !aspas_control)
                    {

                        if (quote_bytes > 0 || line[i] == '"')
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
                                if (quote_bytes > 0)
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
                        } else {
                            if (aspas_control_open_escreva <2 )
                            {
                                message_error("Duas aspas necessárias\n", line_number);
                                return 1;
                            }

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
        else if (line[0] == 's')
        {
            bool is_se_text = false;
            bool is_senao = false;
            int len = strlen(line);
            /* Verifica se começa com "se" ou "senao" */
            if (strncmp(line, se, 2) == 0) {
                /* Verificar se é senao (5 caracteres) */
                if (len >= 3 && strncmp(line, senao, 3) == 0) {
                    is_senao = true;
                } else {
                    is_se_text = true;
                }
            }
            if (is_se_text)
            {
                /*Checando se é se*/
                int i = 2;
                /* Verifica se se(){ - LÉXICO*/
                if (!strncmp(line, se, 2) == 0) {
                    message_error("Módulo se incorretamente", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
                /*SINTÁTICO*/
                int parenteses_control_open_se = 0;
                bool se_found_curly_brace = false;
                bool parenteses_fechou = false;

                while(isspace((unsigned char)line[i]))i++;
                /* Verifica restante da linha */
                for (i; line[i] != '\0'; i++)
                {
                    /* Verificar parênteses*/
                    if (parenteses_control_open_se == 0 && !parenteses_fechou)
                    {
                        if (line[i] == '(')
                        { /*abre parênteses*/
                            parenteses_control_open_se++;
                        }
                    }
                    else if (parenteses_control_open_se >= 1 && !parenteses_fechou)
                    {
                        while(isspace((unsigned char)line[i]))i++;

                        int quote_bytes = is_smart_quote(line, i, len); /*função para verificar as aspas diferentes*/
                        if (line[i] == '!' || line[i] == '"' || quote_bytes > 0)
                        {
                            Resultado res = verificarParametrosSe(line, i, line_number, len);
                            i = res.posicao;
                            i = i - 1; /*volta um*/
                            if (res.sucesso == 1)
                            {
                                return 1;
                            }

                        }
                        else if (line[i] == ')')
                        {
                            parenteses_control_open_se--;
                            parenteses_fechou = 1;
                        }
                    }
                    else if (parenteses_control_open_se == 0 && parenteses_fechou)
                    { /* Após fechar parênteses */
                        while(isspace((unsigned char)line[i])) i++;

                        int dentroSe = -1;

                        if (line[i] == '{')
                        { /* Encontrou a chave de abertura */
                            se_found_curly_brace = true;
                            (*line_number)++; /*tem que fazer isso para contar a próxima linha*/
                             dentroSe = varredura_principal(file, line, line_number, cont_principal, 0, 1, 0);
                        }
                        else
                        { /* Aqui tenho que verificar caso não tenha { */
                            dentroSe = varredura_mesma_linha(line,line_number, i);
                        }

                        if (dentroSe != 0)
                        {
                            message_error("Se construído incorretamente", line_number);
                            return 1;
                        } else {
                            break; /*dentro do se verificado*/
                        }

                    }
                }
                printf("se ok\n");
                /*fim da checagem se é se*/
            }
            else if (is_senao)
            {
                /*Checando se é senao - LÉXICO*/
                int i = 5;
                if (strncmp(line, senao, 5) != 0) {
                    message_error("Módulo senao incorretamente", line_number);
                    return 1; /*O código PARA quando encontra erro*/
                }
                while (isspace((unsigned char)line[i])){i++;}

                /* Se houver conteúdo após "senao" - SINTÁTICO*/
                if (line[i] != '\0') {
                    /*criar função para tratar todos os casos*/
                    int dentroSenao = varredura_mesma_linha(line,line_number, i);

                    if (dentroSenao != 0)
                    {
                        message_error("Senao construído incorretamente", line_number);
                        return 1;
                    }
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
        else if(line[0]=='!')/*mudando valores de variáveis, atribuições*/
        { /*Vou fazer uma função só pra isso, e aí adicionar aqui e no inteiro e decimal*/
            if(verificarOperacaoMatematicaMain(line, 0, line_number)==1){
                return 1;
            } else{
                printf("Operação com variável ok\n");
            }
        }
        else if (line[0] == '_') /*chamada de função*/
        {
            /*aqui tem que pegar se a função foi declarada previamente*/
        }
        else if ((is_function || is_se) && (line[0] == '}' || line[0] == 'r'))
        {
            if (line[0] == 'r')
            {
                /* Verifica se retorno !variavel; - LÉXICO*/
                int i = 0;
                for (i=0; i < 7; i++)
                {
                    if (line[i] != retorno[i])
                    {
                        message_error("Retorno escrito incorretamente", line_number);
                        return 1; /*O código PARA quando encontra erro*/
                    }
                }
                /*SINTÁTICO*/
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
                {/* Verificar primeiro caractere (obrigatoriamente a-z) */
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

                retorno_control = 1;
                printf("retorno ok\n");
                /*Fim da verificação de retorno !variavel;*/
            }

            if (is_function)
            {
                if (retorno_control && line[0] == '}')
                {
                    curly_control = 1;
                    printf("fechou a função ok\n");
                }

                if (retorno_control && curly_control) {
                    return 0; /*tudo certo na função*/
                }
            }

            if (is_se && line[0] == '}') {
                return 0; /*tudo certo no se*/
            }

        }
        else if (line[0] == '}' || line[0] == '{' || line[0] == '\0')
        { /*Aqui tudo que pode estar sozinho na linha - condição final*/
            printf("Conteúdo ok\n");
        } else
        { /*aqui tudo que não poderia estar solto no conteúdo*/
            printf("Conteúdo não reconhecido na linha %i: %c\n",(*line_number), line[0]);
            return 1;
        }

         (*line_number)++;
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

void message_error(const char *erro, int *line_number)
{
    printf("Erro na linha %d: %s\n", *line_number, erro);
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
int verificarVariavelInteira(char line[], int posicao, int *line_number)
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
            int j = i;
            if (line[i] >= 'a' && line[i] <= 'z')
            {
                while (isalnum((unsigned char)line[i]))
                {
                    i++;
                }; /*verifica se o restante é alfanumerico*/
                if (line[i] == ',' && (isspace(line[i + 1])))
                { /*tem mais parâmetros que precisam ser verificados*/
                    int len = (i - j);              /* tamanho da substring*/
                    char *extraida = malloc(len + 1); /* +1 para o terminador '\0'*/
                    if (extraida == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(extraida, &line[j], len);
                    extraida[len] = '\0'; /* garante fim da string*/
                    raiz = inserir_no(raiz, extraida, "inteiro", 99, "0");

                    free(extraida);
                    return verificarVariavelInteira(line, i + 1, line_number);
                }
                else if (line[i] == ';' && line[i + 1] == '\0')
                {
                    int len = (i - j);              /* tamanho da substring*/
                    char *extraida = malloc(len + 1); /* +1 para o terminador '\0'*/
                    if (extraida == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(extraida, &line[j], len);
                    extraida[len] = '\0'; /* garante fim da string*/
                    raiz = inserir_no(raiz, extraida, "inteiro", 99, "0");
                    free(extraida);
                    return 0;
                }
                else if (line[i] == ';' && line[i + 1] == '\n')
                {
                    int len = (i - j);              // tamanho da substring
                    char *extraida = malloc(len + 1); // +1 para o terminador '\0'
                    if (extraida == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(extraida, &line[j], len);
                    extraida[len] = '\0'; // garante fim da string
                    raiz = inserir_no(raiz, extraida, "inteiro", 99, "0");
                    free(extraida);
                    return 0;
                }
                else if (line[i] == ';' && isspace(line[i + 1]))
                { /*tô ignorando espaços que aparecem depois*/
                    int len = (i - j);              // tamanho da substring
                    char *extraida = malloc(len + 1); // +1 para o terminador '\0'
                    if (extraida == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(extraida, &line[j], len);
                    extraida[len] = '\0'; // garante fim da string
                    raiz = inserir_no(raiz, extraida, "inteiro", 99, "0");
                    free(extraida);
                    return 0;
                }
                else if (isspace(line[i]))
                {
                    int len = (i - j);              // tamanho da substring
                    char *extraida = malloc(len + 1); // +1 para o terminador '\0'
                    if (extraida == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(extraida, &line[j], len);
                    extraida[len] = '\0'; // garante fim da string
                    raiz = inserir_no(raiz, extraida, "inteiro", 99, "0");
                    free(extraida);
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
                        int result = verificarOperacaoMatematica(line, i, line_number, 0); /*Aqui dentro vai realizar a atualização dos valores*/
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
int verificarVariavelTexto(char line[], int posicao, int *line_number)
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
            int j = i;
            if (line[i] >= 'a' && line[i] <= 'z')
            {
                while (isalnum((unsigned char)line[i]))
                {
                    i++;
                }; /*verifica se o restante é alfanumerico*/
                int k = i-1;
                if (line[i] == '[')
                {
                    i++;
                    int l = i;
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
                    int len = (k - j+1);              // tamanho da substring
                    char *extraida = malloc(len + 1); // +1 para o terminador '\0'
                    if (extraida == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(extraida, &line[j], len);
                    extraida[len] = '\0'; // garante fim da string

                    len = (i - l);              // tamanho da substring
                    char *tamanho = malloc(len + 1); // +1 para o terminador '\0'
                    if (tamanho == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(tamanho, &line[l], len);
                    tamanho[len] = '\0'; // garante fim da string
                    raiz = inserir_no(raiz, extraida, "texto", strtod(tamanho, NULL), "0");
                    free(tamanho);
                    free(extraida);
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
                                int j = i;
                                while(line[i]!='\0'||line[i]!='\n'){
                                    i++;
                                    if(line[i] == '"' || (is_smart_quote(line, i, strlen(line)) > 0)){
                                        int len = (i - j);              /* tamanho da substring*/
                                        char *extraida = malloc(len + 1); /* +1 para o terminador '\0'*/
                                        if (extraida == NULL) {
                                            message_error("Erro ao alocar memória", line_number);
                                            return 1;
                                        }

                                        strncpy(extraida, &line[j], len);
                                        extraida[len] = '\0'; /* garante fim da string*/
                                        raiz = inserir_no(raiz, "12", "texto", 99, extraida);
                                        Node *one = buscar_no(raiz, "12");
                                        if(extrair_e_atualizar_palavras(line,j-2,one,line_number)==1){
                                            return 1;
                                        }
                                        raiz = remover_no(raiz, "12");
                                        free(extraida);
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
                                if(line[i]=='!'){
                                    return verificarOperacaoMatematica(line, i, line_number, 0);
                                } else {
                                    message_error("Não foi encontrada aspas no texto\n", line_number);
                                    printf("aqui o texto: %c", line[i]);
                                    return 1;
                                }
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
int verificarVariavelDecimal(char line[], int posicao, int *line_number)
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
            int j = i;
            if (line[i] >= 'a' && line[i] <= 'z')
            {
                while (isalnum((unsigned char)line[i]))
                {
                    i++;
                }; /*verifica se o restante é alfanumerico*/
                int k = i;
                if (line[i] == '[')
                {
                    i++;
                    int l = i;
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
                    int len = (k - j);
                    char *extraida = malloc(len + 1);
                    if (extraida == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(extraida, &line[j], len);
                    extraida[len] = '\0';

                    len = (i - l);
                    char *tamanho = malloc(len + 1);
                    if (tamanho == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(tamanho, &line[l], len);
                    tamanho[len] = '\0';
                    float tamanho_float = strtof(tamanho, NULL);
                    raiz = inserir_no(raiz, extraida, "decimal", tamanho_float, "0");
                    free(tamanho);
                    free(extraida);
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
int verificarLeia(char line[], int posicao, int *line_number)
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

int extrair_e_printar_palavras(char *line, int posicao_atual, Node *encontrado, int *line_number) {
    int pos_igual = -1;
    for (int k = posicao_atual - 1; k >= 0; k--) {
        if (line[k] == '=') {
            pos_igual = k;
            break;
        }
    }

    if (pos_igual == -1) {
        message_error("Erro: '=' não encontrado antes da posição atual\n", line_number);
        return 1;
    }

    /* Agora processa as palavras antes do '=' */
    int inicio_palavra = 0;

    for (int k = 0; k < pos_igual; k++) {
        /* Encontrou uma palavra que começa com '!' */
        if (line[k] == '!') {
            inicio_palavra = k + 1; // Pula o '!'
            int fim_palavra = inicio_palavra;
            int tem_colchete = 0; // Flag para detectar colchetes

            // Primeiro, verifica se a palavra contém colchetes
            int temp_pos = inicio_palavra;
            while (temp_pos < pos_igual &&
                   line[temp_pos] != ',' &&
                   line[temp_pos] != ' ' &&
                   line[temp_pos] != '=') {

                if (line[temp_pos] == '[') {
                    tem_colchete = 1;
                    break;
                }
                temp_pos++;
            }

            // Se tem colchetes, pula toda a expressão (incluindo os colchetes)
            if (tem_colchete) {
                fim_palavra = inicio_palavra;
                while (fim_palavra < pos_igual &&
                       line[fim_palavra] != ',' &&
                       line[fim_palavra] != ' ' &&
                       line[fim_palavra] != '=') {
                    if (line[fim_palavra] == '[') {
                        // Pula todo o conteúdo até o ']' correspondente
                        int nivel_colchetes = 1;
                        fim_palavra++;
                        while (fim_palavra < pos_igual && nivel_colchetes > 0) {
                            if (line[fim_palavra] == '[') {
                                nivel_colchetes++;
                            } else if (line[fim_palavra] == ']') {
                                nivel_colchetes--;
                            }
                            fim_palavra++;
                        }
                    } else {
                        fim_palavra++;
                    }
                }
            } else {
                // Se não tem colchetes, encontra o fim normalmente
                while (fim_palavra < pos_igual &&
                       line[fim_palavra] != ',' &&
                       line[fim_palavra] != ' ' &&
                       line[fim_palavra] != '=') {
                    fim_palavra++;
                }
            }

            // Se encontrou uma palavra válida E não tem colchetes
            if (fim_palavra > inicio_palavra && !tem_colchete) {
                int len_palavra = fim_palavra - inicio_palavra;
                char *palavra = malloc(len_palavra + 1);

                if (palavra == NULL) {
                    message_error("Erro ao alocar memória", line_number);
                    return 1;
                }

                strncpy(palavra, &line[inicio_palavra], len_palavra);
                palavra[len_palavra] = '\0';

                /*printf("Palavra encontrada: %s\n", palavra);*/
                Node *encontrado1 = buscar_no(raiz, palavra);
                if(!encontrado1){
                    message_error("Você tentou usar uma variável não declarada anteriormente", line_number);
                    return 1;
                }
                if (strcmp(encontrado1->tipo, encontrado->tipo)!=0){
                    message_error("Os tipos das variáveis do lado direito e esquerdo do '=' devem ser os mesmos", line_number);
                    inorder(raiz);
                    return 1;
                }
                free(palavra);
            }
            // Se tem colchetes, simplesmente ignora a palavra inteira

            k = fim_palavra;
            while (k < pos_igual && (line[k] == ',' || line[k] == ' ')) {
                k++;
            }
            k--;
        }
    }
    return 0;
}

int verificarOperacaoMatematica(char line[], int posicao, int *line_number, int flagTemPonto) /*verifica depois de =*/
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
                int j = i;
                if (line[i] >= 'a' && line[i] <= 'z')
                {
                    while (isalnum((unsigned char)line[i]))
                    {
                        i++;
                    }; /*verifica se o restante é alfanumerico*/
                    int len = (i - j);              /* tamanho da substring*/
                    char *q = malloc(len + 1); /* +1 para o terminador '\0'*/
                    if (q == NULL) {
                        message_error("Erro ao alocar memória", line_number);
                        return 1;
                    }

                    strncpy(q, &line[j], len);
                    q[len] = '\0'; /* garante fim da string*/
                    Node *encontrado = buscar_no(raiz, q);
                    if(!encontrado){
                        message_error("Você tentou usar uma variável não declarada anteriormente", line_number);
                        return 1;
                    }

                    if(extrair_e_printar_palavras(line,i,encontrado,line_number)==1){
                       return 1;
                    }
                    if(strcmp(encontrado->tipo,"texto")==0){
                        while(isspace(line[i])){
                            i++;
                        }
                        if(line[i]!=';'){
                            if(line[i]!='\0'){
                                message_error("Operação com texto não permitida", line_number);
                            }
                        } else {
                            if(extrair_e_atualizar_palavras(line, i, encontrado, line_number)==1){
                                return 1;
                               }
                        }
                    }
                    free(q);
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
    /* Verifica usando comparação de strings segura */
    if (strncmp(str + pos, "\xE2\x80\x9C", 3) == 0) {  /* “ */
        return 1;
    }
    if (strncmp(str + pos, "\xE2\x80\x9D", 3) == 0) {  /* ” */
        return 1;
    }

    return 0;
}

int verificarOperacaoMatematicaMain(char line[], int posicao, int *line_number)
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

/*Função corrigida para verificar duplo balanceamento*/
int verificarBalanceamento(FILE* file) {
    char linha[1024];
    int numLinha = 1;
    No* pilha = NULL;  /* Inicializa pilha vazia */

    while (fgets(linha, sizeof(linha), file)) {
        /* Remove BOM se presente na primeira linha */
        if (numLinha == 1 && strlen(linha) >= 3 &&
            (unsigned char)linha[0] == 0xEF &&
            (unsigned char)linha[1] == 0xBB &&
            (unsigned char)linha[2] == 0xBF) {
            memmove(linha, linha + 3, strlen(linha) - 2);
        }

        int dentroDeAspas = 0;  /* Flag para controlar se estamos dentro de aspas */

        for (int i = 0; linha[i]; i++) {
            char c = linha[i];

            /* Verifica se é uma aspas (normal ou inteligente) */
            int eAspas = (c == '"') || is_smart_quote(linha, i, strlen(linha));

            if (eAspas) {
                /* Se encontrou aspas */
                if (pilha != NULL && pilha->delimitador == '"') {
                    /* Já temos aspas abertas - esta deve fechar */
                    No* temp = pilha;
                    pilha = pilha->next;
                    free(temp);
                    dentroDeAspas = 0;  /* Saímos das aspas */
                } else {
                    /* Não temos aspas abertas - esta deve abrir */
                    No* novo = malloc(sizeof(No));
                    if (novo == NULL) {
                        printf("Erro de memória\n");
                        return -1;
                    }
                    novo->delimitador = '"';
                    novo->linha = numLinha;
                    novo->pos = i + 1;  /* Posição legível (começando em 1)*/
                    novo->next = pilha;
                    pilha = novo;
                    dentroDeAspas = 1;  /* Entramos nas aspas*/
                }

                /* Pula bytes extras se for aspas inteligentes*/
                if (c != '"') {
                    i += 2;  // Avança os bytes extras do UTF-8
                }
                continue;
            }

            /* Se estamos dentro de aspas, ignora outros delimitadores*/
            if (dentroDeAspas) {
                continue;
            }

            /* Tratamento de outros delimitadores (apenas quando fora de aspas)*/
            if (c == '{' || c == '(' || c == '[') {
                No* novo = malloc(sizeof(No));
                if (novo == NULL) {
                    printf("Erro de memória\n");
                    return -1;
                }
                novo->delimitador = c;
                novo->linha = numLinha;
                novo->pos = i + 1;  /* Posição legível*/
                novo->next = pilha;
                pilha = novo;

            } else if (c == '}' || c == ')' || c == ']') {
                if (pilha == NULL) {
                    printf("ERRO linha %d, posição %d: '%c' fechamento sem abertura correspondente\n",
                           numLinha, i + 1, c);
                    return 1;
                }

                /* Determina qual deveria ser o delimitador de abertura*/
                char esperado;
                if (c == '}') esperado = '{';
                else if (c == ')') esperado = '(';
                else esperado = '[';

                if (pilha->delimitador != esperado) {
                    printf("ERRO '%c' aberto na linha %d, posição %d. Não foi fechado adequadamente\n",
                           pilha->delimitador, pilha->linha, pilha->pos);
                    return 1;
                }

                /* Remove da pilha */
                No* temp = pilha;
                pilha = pilha->next;
                free(temp);
            }
        }

        /* Verifica se há aspas não fechadas ao final da linha */
        if (pilha != NULL && pilha->delimitador == '"') {
            /* Verifica se a linha termina adequadamente (com ponto e vírgula)*/
            int ultimoCharSignificativo = strlen(linha) - 1;
            while (ultimoCharSignificativo >= 0 &&
                   (linha[ultimoCharSignificativo] == '\n' ||
                    linha[ultimoCharSignificativo] == '\r' ||
                    isspace(linha[ultimoCharSignificativo]))) {
                ultimoCharSignificativo--;
            }

            /* Se não termina com ponto e vírgula ou outros caracteres válidos após aspas */
            if (ultimoCharSignificativo >= 0 && linha[ultimoCharSignificativo] != ';') {
                printf("ERRO linha %d: Aspas abertas na posição %d não foram fechadas\n",
                       pilha->linha, pilha->pos);
                // Limpa a pilha antes de retornar
                while (pilha != NULL) {
                    No* temp = pilha;
                    pilha = pilha->next;
                    free(temp);
                }
                return 1;
            }
        }

        numLinha++;
    }

    /* Verifica se sobrou algo não foi fechado ao final do arquivo*/
    if (pilha != NULL) {
        if (pilha->delimitador == '"') {
            printf("ERRO: Aspas abertas na linha %d, posição %d não foram fechadas\n",
                   pilha->linha, pilha->pos);
        } else {
            printf("ERRO: '%c' aberto na linha %d, posição %d não foi fechado\n",
                   pilha->delimitador, pilha->linha, pilha->pos);
        }

        /* Limpa toda a pilha */
        while (pilha != NULL) {
            No* temp = pilha;
            pilha = pilha->next;
            free(temp);
        }
        return 1;
    }

    return 0;
}


/*função que irá varrer o restante da linha, em alguns casos*/
int varredura_mesma_linha(char *line, int *line_number, int i) {
    /*palavras reservadas - LÉXICO*/
    const char *leia = "leia";
    const char *escreva = "escreva";
    int tem_conteudo = 0; /*Flag para verificar se há conteúdo significativo na linha*/

    /*Se i está fora dos limites, não há nada para processar*/
    if (i >= strlen(line)) {
        return 0;
    }

    /*Remove espaços a partir da posição i*/
    while (isspace((unsigned char)line[i])) {
        i++;
    }

    /*Se a linha está vazia ou só tem espaços, não precisa de ponto e vírgula*/
    if (line[i] == '\0' || line[i] == '\n') {
        return 0;
    }

    while (line[i] != '\0' && line[i] != '\n') {
        char current_char = line[i];

        if (current_char == 'l') {
            /*VERIFICA SE É "leia" - LÉXICO*/
            int match = 1;
            for (int j = 0; j < 4; j++) {
                if (line[i + j] != leia[j]) {
                    match = 0;
                    break;
                }
            }

            if (match && (i + 4 >= strlen(line) || !isalnum(line[i + 4]))) {
                /*Encontrou "leia" completo*/
                i += 4; /*Pula "leia"*/
                tem_conteudo = 1;

                /*SINTÁTICO - verifica parênteses*/
                while (isspace((unsigned char)line[i])) {
                    i++;
                }

                if (line[i] != '(') {
                    message_error("Falta '(' depois de leia\n", line_number);
                    return 1;
                }

                if (verificarLeia(line, i + 1, line_number) == 1) {
                    return 1;
                }

                /*Verifica se há atribuição (não permitida no leia)*/
                int temp_i = i;
                while (line[temp_i] != '\0' && line[temp_i] != '\n') {
                    if (line[temp_i] == '=') {
                        message_error("Não é permitido atribuições no leia\n", line_number);
                        return 1;
                    }
                    temp_i++;
                }

                /*Avança até o final do comando leia*/
                while (line[i] != '\0' && line[i] != '\n' && line[i] != ';') {
                    i++;
                }

                printf("leia ok\n");
                continue;
            }
        }
        else if (current_char == 'e') {
            /*VERIFICA SE É "escreva" - LÉXICO*/
            int match = 1;
            for (int j = 0; j < 7; j++) {
                if (i + j >= strlen(line) || line[i + j] != escreva[j]) {
                    match = 0;
                    break;
                }
            }

            if (match && (i + 7 >= strlen(line) || !isalnum(line[i + 7]))) {
                /*Encontrou "escreva" completo*/
                i += 7; /*Pula "escreva"*/
                tem_conteudo = 1;

                /*SINTÁTICO - processa escreva*/
                int parenteses_count = 0;
                int aspas_abertas = 0;
                bool dentro_aspas = false;
                bool saiu_aspas = false;
                int len = strlen(line);

                /*Pula espaços após "escreva"*/
                while (isspace((unsigned char)line[i])) {
                    i++;
                }

                if (line[i] != '(') {
                    message_error("Falta '(' depois de escreva\n", line_number);
                    return 1;
                }

                parenteses_count++;
                i++; /*Pula '('*/

                /*Processa o conteúdo dentro dos parênteses*/
                while (line[i] != '\0' && line[i] != '\n')
                {
                    int quote_bytes = is_smart_quote(line, i, len);

                    if ((quote_bytes > 0 || line[i] == '"' ) && !saiu_aspas)
                    {
                        /*Tratamento de aspas*/
                        if (!dentro_aspas) {

                            dentro_aspas = true;
                            aspas_abertas++;
                        } else {

                            dentro_aspas = false;
                            aspas_abertas++;
                            saiu_aspas = true;

                            if (aspas_abertas > 2) {
                                message_error("Use apenas duas aspas por string\n", line_number);
                                return 1;
                            }
                        }

                        if (quote_bytes > 1) {
                            i += quote_bytes - 1; /*Ajusta para aspas UTF-8*/
                            continue;
                        }
                    } else if (!dentro_aspas && saiu_aspas) {
                        if (line[i] == ')') {
                            parenteses_count--;
                        }
                        else if (line[i] == ',' || line[i] == '!' || isspace(line[i]))
                        {
                            /*Caracteres válidos fora de aspas*/
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
                    }
                    else if (isspace((unsigned char)line[i]) || line[i] == ';' && dentro_aspas)
                    {
                        if (line[i] == ';')
                        {
                            break;
                        }
                    }

                    i++;
                }

                if (aspas_abertas < 2) {
                    message_error("Duas aspas necessárias\n", line_number);
                    return 1;
                }

                if (parenteses_count > 0) {
                    message_error("Parênteses não fechados em escreva\n", line_number);
                    return 1;
                }

                printf("escreva ok\n");
                continue;
            }
        }
        else if (current_char == '!') {
            /*Operações com variáveis - atribuições*/
            tem_conteudo = 1;

            if (verificarOperacaoMatematicaMain(line, i, line_number) == 1) {
                return 1;
            }

            /*Avança até o final da operação*/
            while (line[i] != '\0' && line[i] != '\n' && line[i] != ';') {
                i++;
            }

            printf("Operação com variável ok\n");
            continue;
        }
        else if (current_char == '_') {
            /*Chamada de função*/
            tem_conteudo = 1;

            /*Verifica se é uma função válida (__nome)*/
            if (i + 1 < strlen(line) && line[i + 1] == '_') {
                /*Lógica para verificar chamada de função*/
                /*Por enquanto, apenas avança até encontrar o final*/
                while (line[i] != '\0' && line[i] != '\n' && line[i] != ';') {
                    i++;
                }
                printf("Chamada de função ok\n");
                continue;
            } else {
                message_error("Nome de função deve começar com '__'\n", line_number);
                return 1;
            }
        }
        else if (current_char == 's' && i + 1 < strlen(line) && line[i + 1] == 'e') {
            /*Comando SE - casos especiais que não precisam de ; */
            /*Verifica se é realmente "se" completo*/
            if (i + 2 >= strlen(line) || !isalnum(line[i + 2])) {
                return 0; /*Comando "se" não precisa de ponto e vírgula*/
            }
            i++;
        }
        else if (current_char == 'p') {
            /*Comando PARA - casos especiais que não precisam de ; */
            const char *para = "para";
            int match = 1;
            for (int j = 0; j < 4; j++) {
                if (i + j >= strlen(line) || line[i + j] != para[j]) {
                    match = 0;
                    break;
                }
            }

            if (match && (i + 4 >= strlen(line) || !isalnum(line[i + 4]))) {
                return 0; /*Comando "para" não precisa de ponto e vírgula*/
            }
            i++;
        }
        else if (current_char == ';') {
            /*Encontrou ponto e vírgula*/
            /*Verifica se há apenas espaços após o ;*/
            int j = i + 1;
            while (j < strlen(line) && (isspace(line[j]) || line[j] == '\n')) {
                j++;
            }

            if (j >= strlen(line) || line[j] == '\0') {
                printf("; ok\n");
                return 0; /*Linha termina corretamente com ;*/
            } else {
                message_error("Conteúdo após ponto e vírgula\n", line_number);
                return 1;
            }
        }
        else if (isspace((unsigned char)current_char)) {
            /*Ignora espaços*/
            i++;
            continue;
        }
        else {
            /*Caractere não reconhecido*/
            if (isprint(current_char)) {
                printf("Conteúdo não reconhecido na linha %d: '%c'\n", line_number, current_char);
            } else {
                printf("Caractere não imprimível na linha %d (código: %d)\n", line_number, (int)current_char);
            }
            return 1;
        }

        i++;
    }

    /*VERIFICAÇÃO FINAL DO PONTO E VÍRGULA*/
    if (tem_conteudo) {
        /*Se há conteúdo significativo, deve terminar com ;*/

        /*Volta para verificar se o último caractere significativo é ;*/
        int last_char = strlen(line) - 1;
        while (last_char >= 0 && (line[last_char] == '\n' || line[last_char] == '\r' || isspace(line[last_char]))) {
            last_char--;
        }

        if (last_char >= 0 && line[last_char] != ';') {
            message_error("Linha deve terminar com ponto e vírgula", line_number);
            return 1;
        }
    }

    return 0;
}

/*----------------------------------------------------------------------------------------------------------*/
/*funções que retornam Resultado*/
/*função para tratar parametro das funcoes*/
Resultado verificarParametroFuncao(char line[], int posicao, int *line_number)
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
Resultado verificarParametrosPara(char line[], int posicao, int *line_number)
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

/*função para tratar parametros de se*/
Resultado verificarParametrosSe(char line[], int posicao, int *line_number, int len)
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


/*----------------------------------------------------------------------------------------------------------*/
/*funções que retornam Função*/
/* Função auxiliar para validar nomes de variáveis nos parâmetros */
int validar_nome_variavel(char *s) {
    if (s[0] != '!') {
        return 0;
    }
    if (!(s[1] >= 'a' && s[1] <= 'z')) {
        return 0;
    }
    int i = 2;
    while (s[i] != '\0') {
        if (!isalnum((unsigned char)s[i])) {
            return 0;
        }
        i++;
    }
    return 1;
}
/* Função para encontrar funções no arquivo */
Funcao* encontrar_funcoes(FILE *file) {
    Funcao *lista = NULL;
    char line[256];
    int line_number = 1;

    while (fgets(line, sizeof(line), file)) {
        /* Remover BOM se existir */
        if (strlen(line) >= 3 &&
            (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB &&
            (unsigned char)line[2] == 0xBF) {
            memmove(line, line + 3, strlen(line) - 2);
        }

        /* Remover espaços iniciais */
        int i = 0;
        while (isspace((unsigned char)line[i])) {
            i++;
        }

        /* Verificar se linha começa com "funcao" */
        if (strncmp(&line[i], "funcao", 6) == 0) {
            i += 6; /* Avançar após "funcao" */

            /* Pular espaços após "funcao" */
            while (isspace(line[i])) {
                i++;
            }

            /* Verificar "__" */
            if (line[i] != '_' || line[i+1] != '_') {
                line_number++;
                continue;
            }
            i += 2; /* Avançar após "__" */

            /* Extrair nome da função */
            int start_nome = i;
            if (!isalnum((unsigned char)line[i])) {
                line_number++;
                continue;
            }
            i++;
            while (isalnum((unsigned char)line[i])) {
                i++;
            }
            int nome_len = i - start_nome;
            char nome[64];
            if (nome_len >= 64) nome_len = 63;
            strncpy(nome, &line[start_nome], nome_len);
            nome[nome_len] = '\0';

            /* Pular espaços após o nome */
            while (isspace(line[i])) {
                i++;
            }

            /* Verificar '(' */
            if (line[i] != '(') {
                line_number++;
                continue;
            }
            i++; /* Avançar após '(' */

            /* Encontrar ')' correspondente */
            int start_params = i;
            int parent_count = 1;
            while (line[i] != '\0' && parent_count > 0) {
                if (line[i] == '(') parent_count++;
                else if (line[i] == ')') parent_count--;
                i++;
            }
            if (parent_count != 0) {
                line_number++;
                continue;
            }

            /* Extrair substring de parâmetros (excluindo o último ')') */
            int params_len = (i - 1) - start_params;
            char param_str[256] = {0};
            if (params_len > 0) {
                strncpy(param_str, &line[start_params], params_len);
            }

            /* Processar parâmetros */
            char parametros[10][64] = {{0}};
            int num_params = 0;

            /* Se não há parâmetros (string vazia ou só espaços) */
            char *temp = param_str;
            while (isspace(*temp)) temp++;
            if (*temp == '\0') {
                /* Função sem parâmetros - criar struct */
                Funcao *nova = (Funcao*)malloc(sizeof(Funcao));
                if (nova) {
                    strncpy(nova->nome, nome, 63);
                    nova->nome[63] = '\0';
                    nova->linha_declaracao = line_number;
                    nova->num_parametros = 0;
                    nova->proxima = lista;
                    lista = nova;
                }
            } else {
                /* Processar parâmetros separados por vírgula */
                char *token = strtok(param_str, ",");
                int valid_params = 1;

                while (token != NULL && num_params < 10) {
                    /* Remover espaços do token */
                    char *start = token;
                    while (isspace(*start)) start++;
                    char *end = token + strlen(token) - 1;
                    while (end > start && isspace(*end)) end--;
                    *(end + 1) = '\0';

                    /* Ignorar tokens vazios */
                    if (strlen(start) == 0) {
                        token = strtok(NULL, ",");
                        continue;
                    }

                    /* Para sua sintaxe, aceitar diretamente nomes de variáveis */
                    /* Verificar se é um tipo conhecido seguido de variável */
                    char *var_start = NULL;
                    if (strncmp(start, "inteiro", 7) == 0) {
                        var_start = start + 7;
                        while (isspace(*var_start)) var_start++;
                    } else if (strncmp(start, "texto", 5) == 0) {
                        var_start = start + 5;
                        while (isspace(*var_start)) var_start++;
                    } else if (strncmp(start, "decimal", 7) == 0) {
                        var_start = start + 7;
                        while (isspace(*var_start)) var_start++;
                    } else {
                        /* Assumir que é apenas nome de variável (sem tipo explícito) */
                        var_start = start;
                    }

                    /* Validar nome da variável */
                    if (!validar_nome_variavel(var_start)) {
                        valid_params = 0;
                        break;
                    }

                    /* Salvar nome do parâmetro */
                    strncpy(parametros[num_params], var_start, 63);
                    parametros[num_params][63] = '\0';
                    num_params++;

                    token = strtok(NULL, ",");
                }

                /* Se todos os parâmetros válidos, criar struct */
                if (valid_params) {
                    Funcao *nova = (Funcao*)malloc(sizeof(Funcao));
                    if (nova) {
                        strncpy(nova->nome, nome, 63);
                        nova->nome[63] = '\0';
                        nova->linha_declaracao = line_number;
                        nova->num_parametros = num_params;
                        for (int j = 0; j < num_params; j++) {
                            strncpy(nova->parametros[j], parametros[j], 63);
                            nova->parametros[j][63] = '\0';
                        }
                        nova->proxima = lista;
                        lista = nova;
                    }
                }
            }
        }

        line_number++;
    }
    return lista;
}

/*----------------------------------------------------------------------------------------------------------*/
/*Funções para tabela de símbolos*/
char* duplicar_string(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *d = malloc(len);
    if (!d) return NULL;
    memcpy(d, s, len);
    return d;
}

char *substring(const char *str, int inicio, int tamanho) {
  if (str == NULL || inicio < 0 || tamanho <= 0 || inicio + tamanho > strlen(str)) {
    return NULL; // Ou trate o erro de outra forma
  }

  char *sub = (char*) malloc(tamanho + 1); // Aloca espaço para a substring + \0
  if (sub == NULL) {
    return NULL; // Falha na alocação
  }

  strncpy(sub, str + inicio, tamanho);
  sub[tamanho] = '\0'; // Garante o terminador nulo
  return sub;
}

Node* criar_no(const char *nome, const char *tipo, float tamanho, const char *valor) {
    Node *n = malloc(sizeof(Node));
    if (!n) return NULL;
    n->nome = duplicar_string(nome);
    n->tipo = duplicar_string(tipo);
    n->tamanho = tamanho;
    n->valor = duplicar_string(valor);
    n->esq = n->dir = NULL;
    n->altura = 1; /* nó folha */
    return n;
}

int altura(Node *n) {
    return n ? n->altura : 0;
}

int max_int(int a, int b) { return (a > b) ? a : b; }

Node* rotacao_direita(Node *y) {
    Node *x = y->esq;
    Node *T2 = x->dir;

    /* rotação */
    x->dir = y;
    y->esq = T2;

    /* atualizar alturas */
    y->altura = max_int(altura(y->esq), altura(y->dir)) + 1;
    x->altura = max_int(altura(x->esq), altura(x->dir)) + 1;

    return x; /* nova raiz */
}

Node* rotacao_esquerda(Node *x) {
    Node *y = x->dir;
    Node *T2 = y->esq;

    y->esq = x;
    x->dir = T2;

    x->altura = max_int(altura(x->esq), altura(x->dir)) + 1;
    y->altura = max_int(altura(y->esq), altura(y->dir)) + 1;

    return y; /* nova raiz */
}

int fator_balanceamento(Node *n) {
    if (!n) return 0;
    return altura(n->esq) - altura(n->dir);
}

/* inserir: ordena por 'nome' (lexicográfico com strcmp). Duplicatas de 'nome' são ignoradas. */
Node* inserir_no(Node *raiz, const char *nome, const char *tipo, float tamanho, const char *valor) {
    if (raiz == NULL)
        return criar_no(nome, tipo, tamanho, valor);

    int cmp = strcmp(nome, raiz->nome);
    if (cmp < 0)
        raiz->esq = inserir_no(raiz->esq, nome, tipo, tamanho, valor);
    else if (cmp > 0)
        raiz->dir = inserir_no(raiz->dir, nome, tipo, tamanho, valor);
    else {
        /* duplicata: aqui atualizamos tipo/tamanho/valor opcionalmente; vamos atualizar */
        free(raiz->tipo);
        free(raiz->valor);
        raiz->tipo = duplicar_string(tipo);
        raiz->tamanho = tamanho;
        raiz->valor = duplicar_string(valor);
        return raiz;
    }

    /* atualizar altura */
    raiz->altura = 1 + max_int(altura(raiz->esq), altura(raiz->dir));

    int fb = fator_balanceamento(raiz);

    /* casos AVL */
    if (fb > 1 && strcmp(nome, raiz->esq->nome) < 0)
        return rotacao_direita(raiz); /* Left Left */

    if (fb < -1 && strcmp(nome, raiz->dir->nome) > 0)
        return rotacao_esquerda(raiz); /* Right Right */

    if (fb > 1 && strcmp(nome, raiz->esq->nome) > 0) {
        /* Left Right */
        raiz->esq = rotacao_esquerda(raiz->esq);
        return rotacao_direita(raiz);
    }

    if (fb < -1 && strcmp(nome, raiz->dir->nome) < 0) {
        /* Right Left */
        raiz->dir = rotacao_direita(raiz->dir);
        return rotacao_esquerda(raiz);
    };
    return raiz;
}

/* buscar por 'nome' */
Node* buscar_no(Node *raiz, const char *nome) {
    Node *cur = raiz;
    while (cur) {
        int cmp = strcmp(nome, cur->nome);
        if (cmp == 0) return cur;
        if (cmp < 0) cur = cur->esq;
        else cur = cur->dir;
    }
    return NULL;
}

Node* min_valor_no(Node *n) {
    Node *atual = n;
    while (atual && atual->esq)
        atual = atual->esq;
    return atual;
}

/* remover nó por nome (balanceando após remoção) */
Node* remover_no(Node *raiz, const char *nome) {
    if (raiz == NULL) return raiz;

    int cmp = strcmp(nome, raiz->nome);
    if (cmp < 0)
        raiz->esq = remover_no(raiz->esq, nome);
    else if (cmp > 0)
        raiz->dir = remover_no(raiz->dir, nome);
    else {
        /* nó encontrado */
        if (raiz->esq == NULL || raiz->dir == NULL) {
            Node *temp = raiz->esq ? raiz->esq : raiz->dir;

            if (temp == NULL) {
                /* sem filhos */
                free(raiz->nome);
                free(raiz->tipo);
                free(raiz->valor);
                free(raiz);
                return NULL;
            } else {
                /* um filho: substitui */
                free(raiz->nome);
                free(raiz->tipo);
                free(raiz->valor);
                Node *ret = temp;
                free(raiz);
                return ret;
            }
        } else {
            /* dois filhos: pegar sucessor inorder (menor da direita) */
            Node *temp = min_valor_no(raiz->dir);
            /* copiar dados do sucessor para o nó atual */
            free(raiz->nome);
            free(raiz->tipo);
            free(raiz->valor);
            raiz->nome = duplicar_string(temp->nome);
            raiz->tipo = duplicar_string(temp->tipo);
            raiz->tamanho = temp->tamanho;
            raiz->valor = duplicar_string(temp->valor);
            /* remover o sucessor */
            raiz->dir = remover_no(raiz->dir, temp->nome);
        }
    }

    if (raiz == NULL) return raiz;

    /* atualizar altura */
    raiz->altura = 1 + max_int(altura(raiz->esq), altura(raiz->dir));

    int fb = fator_balanceamento(raiz);

    /* balancear */
    if (fb > 1 && fator_balanceamento(raiz->esq) >= 0)
        return rotacao_direita(raiz);

    if (fb > 1 && fator_balanceamento(raiz->esq) < 0) {
        raiz->esq = rotacao_esquerda(raiz->esq);
        return rotacao_direita(raiz);
    }

    if (fb < -1 && fator_balanceamento(raiz->dir) <= 0)
        return rotacao_esquerda(raiz);

    if (fb < -1 && fator_balanceamento(raiz->dir) > 0) {
        raiz->dir = rotacao_direita(raiz->dir);
        return rotacao_esquerda(raiz);
    }

    return raiz;
}

/* alterar: remove registro com nome_antigo e insere novo registro
   (se nome_antigo existir). Se as chaves forem iguais, apenas atualiza campos. */
Node* alterar_no(Node *raiz, const char *nome_antigo, const char *novo_nome, const char *novo_tipo, int novo_tamanho, const char *novo_valor) {
    if (strcmp(nome_antigo, novo_nome) == 0) {
        /* mesma chave: podemos apenas atualizar tipo/tamanho/valor se existir */
        Node *n = buscar_no(raiz, nome_antigo);
        if (n) {
            free(n->tipo);
            free(n->valor);
            n->tipo = duplicar_string(novo_tipo);
            n->tamanho = novo_tamanho;
            n->valor = duplicar_string(novo_valor);
        } else {
            printf("Chave '%s' não encontrada.", nome_antigo);
        }
        return raiz;
    }

    if (!buscar_no(raiz, nome_antigo)) {
        printf("Chave '%s' não encontrada.", nome_antigo);
        return raiz;
    }

    raiz = remover_no(raiz, nome_antigo);
    raiz = inserir_no(raiz, novo_nome, novo_tipo, novo_tamanho, novo_valor);
    return raiz;
}

/* percurso inorder para mostrar (nome, tipo, tamanho, valor) */
void inorder(Node *raiz) {
    if (!raiz) return;
    inorder(raiz->esq);
    int casas = 3; /* implemento isso melhor depois */
    printf("(%s, %s, %.*f, %s) ", raiz->nome, raiz->tipo, casas, raiz->tamanho, raiz->valor);
    inorder(raiz->dir);
}

void liberar_arvore(Node *raiz) {
    if (!raiz) return;
    liberar_arvore(raiz->esq);
    liberar_arvore(raiz->dir);
    free(raiz->nome);
    free(raiz->tipo);
    free(raiz->valor);
    free(raiz);
}

int extrair_e_atualizar_palavras(char *line, int posicao_atual, Node *encontrado, int *line_number) {
    int pos_igual = -1;
    for (int k = posicao_atual - 1; k >= 0; k--) {
        if (line[k] == '=') {
            pos_igual = k;
            break;
        }
    }
    if (pos_igual == -1) {
        message_error("Erro: '=' não encontrado antes da posição atual\n", line_number);
        return 1;
    }
    /* Agora processa as palavras antes do '=' */
    int inicio_palavra = 0;
    for (int k = 0; k < pos_igual; k++) {
        /* Encontrou uma palavra que começa com '!' */
        if (line[k] == '!') {
            inicio_palavra = k + 1; // Pula o '!'
            int fim_palavra = inicio_palavra;
            int tem_colchete = 0;

            int temp_pos = inicio_palavra;
            while (temp_pos < pos_igual &&
                   line[temp_pos] != ',' &&
                   line[temp_pos] != ' ' &&
                   line[temp_pos] != '=') {

                if (line[temp_pos] == '[') {
                    tem_colchete = 1;
                    break;
                }
                temp_pos++;
            }

            if (tem_colchete) {
                fim_palavra = inicio_palavra;
                while (fim_palavra < pos_igual &&
                       line[fim_palavra] != ',' &&
                       line[fim_palavra] != ' ' &&
                       line[fim_palavra] != '=') {
                    if (line[fim_palavra] == '[') {
                        int nivel_colchetes = 1;
                        fim_palavra++;
                        while (fim_palavra < pos_igual && nivel_colchetes > 0) {
                            if (line[fim_palavra] == '[') {
                                nivel_colchetes++;
                            } else if (line[fim_palavra] == ']') {
                                nivel_colchetes--;
                            }
                            fim_palavra++;
                        }
                    } else {
                        fim_palavra++;
                    }
                }
            } else {
                while (fim_palavra < pos_igual &&
                       line[fim_palavra] != ',' &&
                       line[fim_palavra] != ' ' &&
                       line[fim_palavra] != '=') {
                    fim_palavra++;
                }
            }

            if (fim_palavra > inicio_palavra && !tem_colchete) {
                int len_palavra = fim_palavra - inicio_palavra;
                char *palavra = malloc(len_palavra + 1);
                if (palavra == NULL) {
                    message_error("Erro ao alocar memória", line_number);
                    return 1;
                }
                strncpy(palavra, &line[inicio_palavra], len_palavra);
                palavra[len_palavra] = '\0';
                printf("Palavra encontrada: %s\n", palavra);
                Node *encontrado1 = buscar_no(raiz, palavra);
                if(!encontrado1){
                    message_error("Você tentou usar uma variável não declarada anteriormente...", line_number);
                    return 1;
                }
                if (encontrado1->tamanho==encontrado->tamanho){
                    raiz = alterar_no(raiz, encontrado1->nome, encontrado1->nome, encontrado1->tipo, encontrado1->tamanho, encontrado->valor);
                } else {
                    message_error("Para realizar alterações em sua variável, elas precisam ter o mesmo tamanho", line_number);
                    return 1;
                }
                free(palavra);
            }
            k = fim_palavra;
            while (k < pos_igual && (line[k] == ',' || line[k] == ' ')) {
                k++;
            }
            k--;
        }
    }
    return 0;
}
