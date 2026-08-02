#include "compiler.h"
#include "helpers/vector.h"
#include "helpers/buffer.h"
#include <string.h>
#include <stdbool.h>
#include <string.h>
#define LEX_GETC_IF(buffer, c, exp)     \
    for (c = peekc(); exp; c = peekc()) \
    {                                   \
        buffer_write(buffer, c);        \
        nextc();                        \
    }
#define S_COMP(s1, s2) \
    (s1 && s2 && strcmp(s1, s2) == 0)

static struct lex_process *lex_process;
static struct token tmp_token;
struct token *read_next_token();

static struct pos lex_file_position()
{
    return lex_process->pos;
}

static char nextc()
{
    char c = lex_process->funtion->next_char(lex_process);
    lex_process->pos.col += 1;
    if (c == '\n')
    {
        lex_process->pos.line += 1;
        lex_process->pos.col = 1;
    }
    return c;
}

static char peekc()
{
    return lex_process->funtion->peek_char(lex_process);
}

static void pushc(char c)
{
    return lex_process->funtion->push_char(lex_process, c);
}

static bool operator_treated_as_one(char op)
{
    return op == '(' ||
           op == '[' ||
           op == '?' ||
           op == '*' ||
           op == ',';
}

static bool is_single_operator(char op)
{
    return op == '+' ||
           op == '-' ||
           op == '/' ||
           op == '*' ||
           op == '<' ||
           op == '>' ||
           op == '^' ||
           op == '%' ||
           op == '!' ||
           op == '=' ||
           op == '~' ||
           op == '|' ||
           op == '&' ||
           op == '(' ||
           op == '[' ||
           op == '.' ||
           op == ',' ||
           op == '?';
}

static bool is_valid_op(char *op)
{
    return S_COMP(op, "++") ||
           S_COMP(op, "--") ||
           S_COMP(op, "+=") ||
           S_COMP(op, "-=") ||
           S_COMP(op, "*=") ||
           S_COMP(op, "/=") ||
           S_COMP(op, ">=") ||
           S_COMP(op, "==") ||
           S_COMP(op, "!=") ||
           S_COMP(op, "<=") ||
           S_COMP(op, ">>") ||
           S_COMP(op, "<<") ||
           S_COMP(op, "++") ||
           S_COMP(op, "--") ||
           S_COMP(op, "||") ||
           S_COMP(op, "&&") ||
           S_COMP(op, "->") ||
           S_COMP(op, "...") ||
           S_COMP(op, "+") ||
           S_COMP(op, "-") ||
           S_COMP(op, "/") ||
           S_COMP(op, "*") ||
           S_COMP(op, "<") ||
           S_COMP(op, ">") ||
           S_COMP(op, "^") ||
           S_COMP(op, "%") ||
           S_COMP(op, "!") ||
           S_COMP(op, "=") ||
           S_COMP(op, "~") ||
           S_COMP(op, "|") ||
           S_COMP(op, "&") ||
           S_COMP(op, "(") ||
           S_COMP(op, "[") ||
           S_COMP(op, ".") ||
           S_COMP(op, ",") ||
           S_COMP(op, "?");
}

struct token *token_create(struct token *_token)
{
    memcpy(&tmp_token, _token, sizeof(struct token));
    tmp_token.pos = lex_file_position();
    return &tmp_token;
}

static struct token *lexer_last_token()
{
    return vector_back_or_null(lex_process->token_vec);
}

static struct token *handle_white_space()
{
    struct token *last_token = lexer_last_token();
    if (last_token)
    {
        last_token->whitespace = true;
    }
    nextc();
    return read_next_token();
}

const char *read_number_str()
{
    const char *num = NULL;
    struct buffer *buffer = buffer_create();
    char c = peekc();
    LEX_GETC_IF(buffer, c, (c >= '0' && c <= '9') || c == '.');
    buffer_write(buffer, 0x00);
    printf("token_buffer:%s\n", buffer_ptr(buffer));
    return buffer_ptr(buffer);
}

const char *read_str()
{
    const char *num = NULL;
    struct buffer *buffer = buffer_create();
    char c = nextc();
    LEX_GETC_IF(buffer, c, c != EOF && c != '\"');
    if (peekc() == EOF)
    {
        compiler_error(lex_process->compiler, "expected a \" but reached end of file");
    }
    buffer_write(buffer, 0x00);
    nextc();
    printf("token_buffer:%s\n", buffer_ptr(buffer));
    return buffer_ptr(buffer);
}

const char read_char()
{
    nextc();
    char c = nextc();
    if (peekc() == EOF)
    {
        compiler_error(lex_process->compiler, "expected a \' but reached end of file");
    }
    else if (peekc() != '\'')
    {
        compiler_error(lex_process->compiler, "expected a \' but instead received %c", peekc());
    }
    nextc();
    return c;
}

const char *read_operator()
{
    bool single_operator = true;
    const char *num = NULL;
    struct buffer *buffer = buffer_create();
    char op = nextc();
    buffer_write(buffer, op);
    char op1 = 0x00, op2 = 0x00;
    if (!operator_treated_as_one(op))
    {
        op1 = peekc();
        if (is_single_operator(op1))
        {
            buffer_write(buffer, op1);
            nextc();
            single_operator = false;
            op2 = peekc();
            if (op == '.' && op1 == '.' && op2 == '.')
            {
                buffer_write(buffer, '.');
                nextc();
            }
        }
    }
    buffer_write(buffer, 0x00);
    char *operator = buffer_ptr(buffer);
    if (single_operator && !is_single_operator(*operator))
    {
        compiler_error(lex_process->compiler, "invalid operation %s", operator);
    }
    if (!single_operator && !is_valid_op(operator))
    {
        if (op2)
        {
            pushc(op2);
            operator[2] = 0x00;
        }
        if (!is_valid_op(operator))
            if (op1)
            {
                pushc(op1);
                operator[1] = 0x00;
            }
    }
    printf("token_buffer:%s\n", buffer_ptr(buffer));
    return buffer_ptr(buffer);
}

double read_number()
{
    const char *s = read_number_str();
    return atof(s);
}

struct token *make_token_number_for_value(double number)
{
    return token_create(&(struct token){.type = TOKEN_TYPE_NUMBER, .ffnum = number});
}

struct token *make_token_number()
{
    return make_token_number_for_value(read_number());
}

struct token *make_token_string()
{
    return token_create(&(struct token){.type = TOKEN_TYPE_STRING, .sval = read_str()});
}

struct token *make_token_char()
{
    return token_create(&(struct token){.type = TOKEN_TYPE_CHAR, .cval = read_char()});
}

struct token *make_token_operator()
{
    return token_create(&(struct token){.type = TOKEN_TYPE_OPERATOR, .sval = read_operator()});
}

struct token *read_next_token()
{
    struct token *token = NULL;
    int col = lex_process->pos.col;
    int line = lex_process->pos.line;
    int ccol = lex_process->compiler->pos.col;
    int cline = lex_process->compiler->pos.line;
    char c = nextc();
    char cnext = nextc();
    pushc(cnext);
    pushc(c);
    lex_process->pos.col = col;
    lex_process->pos.line = line;
    lex_process->compiler->pos.col = ccol;
    lex_process->compiler->pos.line = cline;
    printf("%c,%c\n", c, cnext);
    switch (c)
    {
    ISNUMERIC:
        token = make_token_number();
        break;
    ISOPERATOR:
        if (c == '.' && ((cnext >= '0' && cnext <= '9') ||
                         cnext == ';'))
        {
            token = make_token_number();
            break;
        }
        token = make_token_operator();
        break;
    case '\"':
        token = make_token_string();
        break;
    case '\'':
        token = make_token_char();
        break;
    case ' ':
    case '\n':
    case '\t':
        token = handle_white_space();
    case EOF:
        break;
    default:
        compiler_error(lex_process->compiler, "unidentifiable token %c", c);
        break;
    }
    return token;
}

int lex(struct lex_process *lprocess)
{
    lprocess->current_expression_count = 0;
    lprocess->parenthesis_buffer = NULL;
    lex_process = lprocess;
    lprocess->pos.filename = lprocess->compiler->cfile.abs_path;
    lprocess->compiler->pos.filename = lprocess->compiler->cfile.abs_path;
    lprocess->compiler->pos.col = 1;
    lprocess->compiler->pos.line = 1;
    struct token *token = read_next_token();
    while (token)
    {
        vector_push(lprocess->token_vec, token);
        token = read_next_token();
    }
    return 0;
}