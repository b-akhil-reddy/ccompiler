#ifndef CCOMPILER_H
#define CCOMPILER_H
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define S_COMP(s1, s2) \
    (s1 && s2 && strcmp(s1, s2) == 0)

#define ISNUMERIC \
    case '0':     \
    case '1':     \
    case '2':     \
    case '3':     \
    case '4':     \
    case '5':     \
    case '6':     \
    case '7':     \
    case '8':     \
    case '9'

#define ISOPERATOR \
    case '+':      \
    case '-':      \
    case '*':      \
    case '<':      \
    case '>':      \
    case '^':      \
    case '%':      \
    case '!':      \
    case '=':      \
    case '~':      \
    case '|':      \
    case '&':      \
    case '(':      \
    case '[':      \
    case '.':      \
    case ',':      \
    case '?'

#define ISSYMBOL \
    case '{':    \
    case '}':    \
    case ':':    \
    case ';':    \
    case '#':    \
    case '\\':   \
    case ')':    \
    case ']'

#define ISALPHA(c) (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')

#define ISHEXC(c) (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')

struct pos
{
    int line;
    int col;
    const char *filename;
};

enum
{
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_CHAR,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_NEWLINE
};

struct token
{
    int type;
    int flags;
    struct pos pos;
    union
    {
        char cval;
        const char *sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
        float fnum;
        double ffnum;
        void *any;
    };

    bool whitespace;
    const char *between_brackets;
};

struct lex_process;
typedef char (*LEX_PROCESS_NEXT_CHAR)(struct lex_process *process);
typedef char (*LEX_PROCESS_PEEK_CHAR)(struct lex_process *process);
typedef void (*LEX_PROCESS_PUSH_CHAR)(struct lex_process *process, char c);

struct lex_process_functions
{
    LEX_PROCESS_NEXT_CHAR next_char;
    LEX_PROCESS_PEEK_CHAR peek_char;
    LEX_PROCESS_PUSH_CHAR push_char;
};
struct compile_process;
struct lex_process
{
    struct pos pos;
    struct vector *token_vec;
    int current_expression_count;
    struct buffer *parenthesis_buffer;
    struct lex_process_functions *funtion;
    struct compile_process *compiler;
    void *private;
};

enum
{
    COMPILER_OK,
    COMPILER_FAIL
};

enum
{
    LEX_OK,
    LEX_INP_ERR,
    LEX_FAIL
};

struct compile_process
{
    int flags;
    struct pos pos;
    struct compile_process_input_file
    {
        FILE *fp;
        const char *abs_path;
    } cfile;

    FILE *ofile;
};

int compile_file(const char *filename, const char *out_fiename, int flags);
struct compile_process *create_compile_process(const char *filename, const char *filename_out, int flags);
char compile_process_next_char(struct lex_process *lex_process);
char compile_process_peek_char(struct lex_process *lex_process);
void compile_process_push_char(struct lex_process *lex_process, char c);
void compiler_error(struct compile_process *cprocess, const char *msg, ...);
void compiler_warn(struct compile_process *cprocess, const char *msg, ...);

struct lex_process *create_lex_process(struct compile_process *cprocess, struct lex_process_functions *compiler_lex_functions, void *private);
void free_lex_process(struct lex_process *lprocess);
void *lex_process_private(struct lex_process *lprocess);
struct vector *lex_process_tokens(struct lex_process *lprocess);
int lex(struct lex_process *lprocess);

bool token_is_keyword(struct token *token, char *keyword);

#endif