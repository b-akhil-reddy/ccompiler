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
    process->ofile = ofile;
    return process;
}