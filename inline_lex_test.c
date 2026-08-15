#include <stdio.h>
#include "compiler.h"

int main()
{
    struct lex_process* lprocess = build_tokens_from_string(&(struct compile_process){
                                 .pos.line = 1,
                                 .pos.col = 1},
                             "test 123 123.123");
    if(!lprocess){
        return LEX_FAIL;
    }

    return COMPILER_OK;
}
