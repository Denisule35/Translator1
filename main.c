#include <stdio.h>
#include "lexer.h"
#include "utils.h"
#include "parser.h"
int main(void) {
    char *buffer = loadFile("1.q");
    puts(buffer);
    tokenize(buffer);
    showTokens();
    parse();
    return 0;
}