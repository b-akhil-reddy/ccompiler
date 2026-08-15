#include <stdio.h>
#include "compiler.h"

int main()
{
    int res = compile_file("./test/test.c", "./test/test", 0);
    if (res == COMPILER_OK)
    {
        printf("compiled successfully!\n");
    }
    else if (res == COMPILER_FAIL)
    {
        printf("compilation failed\n");
    }
    else
    {
        printf("unknown error\n");
    }
    return res;
}