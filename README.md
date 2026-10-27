## Trabalho compiladores

Este repositório contém o trabalho prático desenvolvido para a disciplina de Gramáticas e Compiladores. O projeto consiste na implementação de um compilador em linguagem C para uma linguagem de programação customizada, abordando as fases de análise léxica, sintática e semântica.

**Especificações do Compilador Desenvolvido**

O software foi construído seguindo restrições estritas de desenvolvimento, incluindo alocação dinâmica de memória com limite de 2048 KB e execução nativa sem dependência de plugins externos. As principais características e regras da linguagem implementada incluem:
    - Estrutura do Programa: Suporte a funções customizadas (funcao()) e exigência de uma função principal obrigatória (principal()).
    - Tipagem e Variáveis: Declaração de variáveis globais e locais dos tipos inteiro, texto e decimal, que devem ser obrigatoriamente iniciadas com o marcador ! (ex: !variavel).
    - Controle de Fluxo e I/O: Suporte a laços de repetição (para()), estruturas condicionais (se / senao) e comandos nativos de entrada e saída (leia() / escreva()).
    - Tabela de Símbolos: Estrutura de dados para o gerenciamento de escopo, contendo tipo de dado, nome da variável, valor e função/módulo a que pertence.
    - Análise e Tratamento de Erros: Identificação de falhas léxicas e sintáticas (com interrupção da compilação e indicação da linha), além de emissão de alertas para inconsistências semânticas ou uso elevado de memória (90% a 99%).

---
Requisitos
----------
Certifique-se de ter um compilador C (como o GCC/MinGW) instalado no seu sistema ou utilize uma IDE de sua preferência.

---
Como executar
-----------------

**Compilação via Code::Blocks**

1. Abra o Code::Blocks e vá em File > Open para carregar o código-fonte.

2. No menu superior, clique em Build e selecione Build and run (ou pressione a tecla F9).

3. Uma janela de console surgirá exibindo a execução do compilador.

**Compilação via Terminal (GCC)**

1. Abra o terminal e navegue até a pasta raiz do repositório clonado.

2. Compile o código executando o comando: gcc -o compilador main.c (substitua main.c pelos arquivos fonte reais do projeto).

3. Execute o programa gerado: no Linux/Mac digite ./compilador e no Windows digite compilador.exe.

**Compilação via Dev-C++**

1. Inicie o Dev-C++ e abra o arquivo fonte principal do projeto.

2. Acesse o menu superior Executar e clique em Compilar e Executar, ou pressione diretamente a tecla F11.

3. O terminal integrado será aberto automaticamente com o programa em execução.


---
Sobre a Disciplina
---------------------

A disciplina explora a teoria das linguagens formais, com foco em Linguagens Livres do Contexto. Os principais tópicos teóricos estudados incluem árvores de derivação, tratamento de ambiguidades, simplificação de gramáticas (remoção de produções vazias, unitárias e símbolos inúteis) e conversão para formas normais, como a Forma Normal de Chomsky e a Forma Normal de Greibach.
