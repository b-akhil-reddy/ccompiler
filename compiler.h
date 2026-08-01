#ifndef CCOMPILER_H
#include <stdio.h>

#define CCOMPILER_H
enum {
    COMPILER_OK,
    COMPILER_FAIL
};

struct compile_process
{
    int flags;

    struct compile_process_input_file
    {
        FILE *fp;
        const char *abs_path;
    } cfile;

    FILE *ofile;
};

int compile_file(const char *filename, const char *out_fiename, int flags);
struct compile_process *create_compile_process(const char *filename, const char *filename_out, int flags);

#endif