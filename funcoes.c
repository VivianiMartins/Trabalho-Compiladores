#include "funcoes.h"
#include <ctype.h>

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
