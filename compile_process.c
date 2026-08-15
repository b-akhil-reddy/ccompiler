#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
struct compile_process *create_compile_process(const char *filename, const char *out_filename, int flags)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        return NULL;
    }
    FILE *ofile = NULL;
    if (out_filename)
    {
        ofile = fopen(out_filename, "w");
        if (!file)
        {
            return NULL;
        }
    }

    struct compile_process *process = calloc(1, sizeof(struct compile_process));
    process->flags = flags;
    process->cfile.fp = file;
    process->cfile.abs_path = filename;
    process->ofile = ofile;
    process->pos.line = 1;
    process->pos.col = 1;
    return process;
}

static int tester = 0;
char compile_process_next_char(struct lex_process *lex_process)
{
    struct compile_process *compiler = lex_process->compiler;
    char c = getc(compiler->cfile.fp);
    if (c == '\n')
    {
        compiler->pos.line += 1;
        compiler->pos.col = 1;
    }
    else
    {
        compiler->pos.col += 1;
    }
    printf("compiler_process:%c:%d::%d\n", c, compiler->pos.line, compiler->pos.col);
    tester += 1;
    // if(tester==6)
    // exit(1);
    return c;
}

char compile_process_peek_char(struct lex_process *lex_process)
{
    struct compile_process *compiler = lex_process->compiler;
    char c = getc(compiler->cfile.fp);
    ungetc(c, compiler->cfile.fp);
    return c;
}

void compile_process_push_char(struct lex_process *lex_process, char c)
{
    struct compile_process *compiler = lex_process->compiler;
    ungetc(c, compiler->cfile.fp);
}