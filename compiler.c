#include "compiler.h"
#include <stdarg.h>
#include <stdlib.h>

#include <err.h>

struct lex_process_functions compiler_lex_functions =
    {
        .next_char = compile_process_next_char,
        .peek_char = compile_process_peek_char,
        .push_char = compile_process_push_char};

int compile_file(const char *filename, const char *out_filename, int flags)
{
    struct compile_process *cprocess = create_compile_process(filename, out_filename, flags);
    if (!cprocess)
    {
        return COMPILER_FAIL;
    }
    // TODO: lexical analysis, parsing, code generation
    // lexical analysis
    struct lex_process *lex_process = create_lex_process(cprocess, &compiler_lex_functions, NULL);
    if (!lex_process)
    {
        printf("creation of lexer failed!\n");
        return COMPILER_FAIL;
    }

    if (lex(lex_process) != LEX_OK)
    {
        printf("failed during lexing!\n");
        return COMPILER_FAIL;
    }
    return COMPILER_OK;
}

void compiler_error(struct compile_process *cprocess, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr," on line %d, col %d, in file %s\n", cprocess->pos.line, cprocess->pos.col, cprocess->pos.filename);
    exit(-1);
}

void compiler_warn(struct compile_process *cprocess, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr," on line %d, col %d, in file %s\n", cprocess->pos.line, cprocess->pos.col, cprocess->pos.filename);
}

