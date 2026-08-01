#include "compiler.h"

int compile_file(const char* filename, const char* out_filename, int flags) {
    struct compile_process* process = create_compile_process(filename,out_filename,flags);
    if (!process) {
        return COMPILER_FAIL;
    }
    // TODO: lexical analysis, parsing, code generation
    return COMPILER_OK;
}