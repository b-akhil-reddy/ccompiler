#include "compiler.h"

bool token_is_keyword(struct token *token, char *keyword)
{
    return token->type == TOKEN_TYPE_KEYWORD && S_COMP(token->sval, "include");
}
