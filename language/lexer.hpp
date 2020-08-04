#include "source.hpp"
#include "ast.hpp"

class Lexer
{
public:
    Source source;
    Token *lastToken;
    Token *token;
    int line;
    int lineStart;
    Lexer(Source source);
    Token *advance();
    Token *lookahead();
};