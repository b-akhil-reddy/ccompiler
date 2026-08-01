#include "compiler.h"
#include "helpers/vector.h"
#include <stdlib.h>
struct lex_process *create_lex_process(struct compile_process *cprocess, struct lex_process_functions *compiler_lex_functions, void *private)
{
    struct lex_process *lprocess = calloc(1, sizeof(struct lex_process));
    lprocess->compiler = cprocess;
    lprocess->funtion = compiler_lex_functions;
    lprocess->token_vec = vector_create(sizeof(struct token));
    lprocess->pos.col = 1;
    lprocess->pos.line = 1;
    lprocess->current_expression_count = 0;
    lprocess->private = private;
    lprocess->parenthesis_buffer = NULL;
    lprocess->private = NULL;
    return lprocess;
}

void free_lex_process(struct lex_process *lprocess)
{
    vector_free(lprocess->token_vec);
    free(lprocess);
}

void *lex_process_private(struct lex_process *lprocess)
{
    return lprocess->private;
}

struct vector *lex_process_tokens(struct lex_process *lprocess)
{
    return lprocess->token_vec;
}