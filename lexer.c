#include "compiler.h"
#include "helpers/vector.h"
#include "helpers/buffer.h"
#include <string.h>
#include <stdbool.h>
#include <string.h>

#define LEX_GETC_IF_WITH_CHECK(buffer, c, exp, check, value) \
    for (c = peekc(); exp; c = peekc())                      \
    {                                                        \
        buffer_write(buffer, c);                             \
        check = check || c == value;                         \
        nextc();                                             \
    }

#define LEX_GETC_IF(buffer, c, exp)     \
    for (c = peekc(); exp; c = peekc()) \
    {                                   \
        buffer_write(buffer, c);        \
        nextc();                        \
    }

static struct lex_process *lex_process;
static struct token tmp_token;
struct token *read_next_token();
static void lex_finish_expression();
bool lex_is_in_expression();

static struct pos lex_file_position()
{
    return lex_process->pos;
}

static char nextc()
{
    char c = lex_process->funtion->next_char(lex_process);
    if (c == '\n')
    {
        lex_process->pos.line += 1;
        lex_process->pos.col = 1;
    }
    else
    {
        lex_process->pos.col += 1;
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

const char *read_number_str(bool *isfloat)
{
    const char *num = NULL;
    struct buffer *buffer = buffer_create();
    char c = peekc();
    LEX_GETC_IF_WITH_CHECK(buffer, c, (c >= '0' && c <= '9') || c == '.', (*isfloat), '.');
    buffer_write(buffer, 0x00);
    printf("token_buffer:%s\n", buffer_ptr(buffer));
    return buffer_ptr(buffer);
}

const char *read_str(char endlim)
{
    const char *num = NULL;
    struct buffer *buffer = buffer_create();
    char c = nextc();
    LEX_GETC_IF(buffer, c, c != EOF && c != endlim);
    if (peekc() == EOF)
    {
        compiler_error(lex_process->compiler, "expected a %c but reached end of file", endlim);
    }
    buffer_write(buffer, 0x00);
    nextc();
    printf("token_buffer:%s\n", buffer_ptr(buffer));
    return buffer_ptr(buffer);
}

const char read_escaped_char(char c)
{
    char out = 0;
    switch (c)
    {
    case 'n':
        out = '\n';
        break;
    case 't':
        out = '\t';
        break;
    case '\'':
        out = '\'';
        break;
    case '\"':
        out = '\"';
        break;
    case '\\':
        out = '\\';
        break;
    case '0':
        out = '\0';
        break;
    default:
        compiler_error(lex_process->compiler, "invalid character received after escape character \\");
        break;
    }
    return out;
}

const char read_char()
{
    nextc();
    char c = nextc();
    if (c == '\\')
    {
        c = nextc();
        c = read_escaped_char(c);
        if (peekc() == EOF)
        {
            compiler_error(lex_process->compiler, "expected a \' but reached end of file");
        }
        else if (peekc() != '\'')
        {
            compiler_error(lex_process->compiler, "expected a \' but instead received %c", peekc());
        }
    }
    else if (peekc() == EOF)
    {
        compiler_error(lex_process->compiler, "expected a \' but reached end of file");
    }
    else if (peekc() != '\'')
    {
        compiler_error(lex_process->compiler, "expected a \' but instead received %c", peekc());
    }
    nextc();
    printf("token_buffer:%c\n", c);
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
    int col1 = lex_process->pos.col;
    int line1 = lex_process->pos.line;
    int ccol1 = lex_process->compiler->pos.col;
    int cline1 = lex_process->compiler->pos.line;
    int col2 = lex_process->pos.col;
    int line2 = lex_process->pos.line;
    int ccol2 = lex_process->compiler->pos.col;
    int cline2 = lex_process->compiler->pos.line;
    if (!operator_treated_as_one(op))
    {
        op1 = peekc();
        col1 = lex_process->pos.col;
        line1 = lex_process->pos.line;
        ccol1 = lex_process->compiler->pos.col;
        cline1 = lex_process->compiler->pos.line;
        if (is_single_operator(op1))
        {
            buffer_write(buffer, op1);
            nextc();
            single_operator = false;
            op2 = peekc();
            col2 = lex_process->pos.col;
            line2 = lex_process->pos.line;
            ccol2 = lex_process->compiler->pos.col;
            cline2 = lex_process->compiler->pos.line;
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
        if (op2 != '\0' && is_single_operator(op2))
        {
            pushc(op2);
            lex_process->pos.col = col2;
            lex_process->pos.line = line2;
            lex_process->compiler->pos.col = ccol2;
            lex_process->compiler->pos.line = cline2;
            operator[2] = 0x00;
        }
        if (!is_valid_op(operator) && is_single_operator(op1))
            if (op1)
            {
                pushc(op1);
                lex_process->pos.col = col1;
                lex_process->pos.line = line1;
                lex_process->compiler->pos.col = ccol1;
                lex_process->compiler->pos.line = cline1;
                operator[1] = 0x00;
            }
    }
    printf("token_buffer:%s\n", buffer_ptr(buffer));
    return buffer_ptr(buffer);
}

struct token *make_token_number_for_int(unsigned long number)
{
    return token_create(&(struct token){.type = TOKEN_TYPE_NUMBER, .llnum = number});
}

struct token *make_token_number_for_float(double number)
{
    return token_create(&(struct token){.type = TOKEN_TYPE_NUMBER, .ffnum = number});
}

struct token *make_token_hex()
{
    struct buffer *buffer = buffer_create();
    char c = 0;
    bool isfloat = false;
    LEX_GETC_IF(buffer, c, ISHEXC(c));
    buffer_write(buffer, 0x00);
    unsigned long number = strtol(buffer_ptr(buffer), 0, 16);
    return make_token_number_for_int(number);
}

struct token *make_token_binary()
{
    struct buffer *buffer = buffer_create();
    char c = 0;
    LEX_GETC_IF(buffer, c, c == '0' || c == '1');
    buffer_write(buffer, 0x00);
    unsigned long number = strtol(buffer_ptr(buffer), 0, 2);
    return make_token_number_for_int(number);
}

struct token *make_token_number()
{
    bool isfloat = false;
    const char *s = read_number_str(&isfloat);
    if (isfloat)
    {
        double num = atof(s);
        return make_token_number_for_float(num);
    }
    else
    {
        int num = atoi(s);
        return make_token_number_for_int(num);
    }
}

struct token *make_token_string(char endlim)
{
    return token_create(&(struct token){.type = TOKEN_TYPE_STRING, .sval = read_str(endlim)});
}

struct token *make_token_char()
{
    return token_create(&(struct token){.type = TOKEN_TYPE_CHAR, .cval = read_char()});
}

struct token *make_token_newline()
{
    return token_create(&(struct token){.type = TOKEN_TYPE_NEWLINE, .cval = nextc()});
}

struct token *make_token_symbol()
{
    char c = nextc();
    if (c == ')')
    {
        lex_finish_expression();
    }
    printf("token_buffer:%c\n", c);
    return token_create(&(struct token){.type = TOKEN_TYPE_SYMBOL, .cval = c});
}

static void lex_new_expression()
{
    lex_process->current_expression_count++;
    if (lex_process->current_expression_count == 1)
    {
        lex_process->parenthesis_buffer = buffer_create();
    }
}

static void lex_finish_expression()
{
    lex_process->current_expression_count--;
    if (lex_process->current_expression_count < 0)
    {
        compiler_error(lex_process->compiler, "expression closed before opening");
    }
}

bool lex_is_in_expression()
{
    return lex_process->current_expression_count > 0;
}

bool is_keyword(char *identifier)
{
    return S_COMP(identifier, "char") ||
           S_COMP(identifier, "short") ||
           S_COMP(identifier, "int") ||
           S_COMP(identifier, "long") ||
           S_COMP(identifier, "signed") ||
           S_COMP(identifier, "unsigned") ||
           S_COMP(identifier, "float") ||
           S_COMP(identifier, "double") ||
           S_COMP(identifier, "void") ||
           S_COMP(identifier, "struct") ||
           S_COMP(identifier, "union") ||
           S_COMP(identifier, "static") ||
           S_COMP(identifier, "__ignore_typecheck") ||
           S_COMP(identifier, "return") ||
           S_COMP(identifier, "include") ||
           S_COMP(identifier, "sizeof") ||
           S_COMP(identifier, "if") ||
           S_COMP(identifier, "else") ||
           S_COMP(identifier, "while") ||
           S_COMP(identifier, "for") ||
           S_COMP(identifier, "do") ||
           S_COMP(identifier, "break") ||
           S_COMP(identifier, "continue") ||
           S_COMP(identifier, "switch") ||
           S_COMP(identifier, "case") ||
           S_COMP(identifier, "default") ||
           S_COMP(identifier, "goto") ||
           S_COMP(identifier, "typedef") ||
           S_COMP(identifier, "const") ||
           S_COMP(identifier, "extern") ||
           S_COMP(identifier, "restrict") ||
           S_COMP(identifier, "enum") ||
           S_COMP(identifier, "register") ||
           S_COMP(identifier, "atomic");
}

struct token *make_token_operator()
{
    char op = peekc();
    if (op == '<')
    {
        struct token *last_token = lexer_last_token();
        if (token_is_keyword(last_token, "include"))
        {
            return make_token_string('>');
        }
    }
    else if (op == '(')
    {
        lex_new_expression();
    }
    // Closing of expression will be handled in symbols as ')' is part of symbols

    return token_create(&(struct token){.type = TOKEN_TYPE_OPERATOR, .sval = read_operator()});
}

struct token *make_token_identifier_or_keyword()
{
    const char *num = NULL;
    struct buffer *buffer = buffer_create();
    char c = peekc();
    LEX_GETC_IF(buffer, c, (ISALPHA(c) || (c >= '0' && c <= '9') || (c == '_')) && c != EOF);
    buffer_write(buffer, 0x00);
    printf("token_buffer:%s\n", buffer_ptr(buffer));
    if (is_keyword(buffer_ptr(buffer)))
    {
        return token_create(&(struct token){.type = TOKEN_TYPE_KEYWORD, .sval = buffer_ptr(buffer)});
    }
    return token_create(&(struct token){.type = TOKEN_TYPE_IDENTIFIER, .sval = buffer_ptr(buffer)});
}

struct token *make_token_singleline_comment()
{
    struct buffer *buffer = buffer_create();
    char c = peekc();
    LEX_GETC_IF(buffer, c, c != '\n' && c != EOF);
    printf("token_buffer:%s\n", buffer_ptr(buffer));
    return token_create(&(struct token){.type = TOKEN_TYPE_COMMENT, .sval = buffer_ptr(buffer)});
}

struct token *make_token_multiline_comment()
{
    struct buffer *buffer = buffer_create();
    char c = peekc();
    while (1)
    {
        LEX_GETC_IF(buffer, c, c != '*' && c != EOF);
        if (c == EOF)
        {
            compiler_error(lex_process->compiler, "multiline comment not ended");
        }
        else if (c == '*')
        {
            nextc();
            if (peekc() == '/')
            {
                nextc();
                break;
            }
        }
    }
    printf("token_buffer:%s\n", buffer_ptr(buffer));
    return token_create(&(struct token){.type = TOKEN_TYPE_COMMENT, .sval = buffer_ptr(buffer)});
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
    if (c == '0' && cnext == 'x')
    {
        return make_token_hex();
    }
    else if (c == '0' && cnext == 'b')
    {
        return make_token_binary();
    }
    else if (c == '/' && cnext == '/')
    {
        return make_token_singleline_comment();
    }
    else if (c == '/' && cnext == '*')
    {
        return make_token_multiline_comment();
    }
    else if (c == '/')
    {
        pushc(cnext);
        pushc(c);
        lex_process->pos.col = col;
        lex_process->pos.line = line;
        lex_process->compiler->pos.col = ccol;
        lex_process->compiler->pos.line = cline;
        return make_token_operator();
    }
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
    ISSYMBOL:
        token = make_token_symbol();
        break;
    case '\"':
        token = make_token_string('\"');
        break;
    case '\'':
        token = make_token_char();
        break;
    case ' ':
    case '\t':
        token = handle_white_space();
        break;
    case '\n':
        token = make_token_newline();
        break;
    case EOF:
        break;
    default:
        if ((ISALPHA(c) || (c == '_')))
        {
            token = make_token_identifier_or_keyword();
        }
        if (!token)
        {
            compiler_error(lex_process->compiler, "unidentifiable token at %c", c);
        }
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