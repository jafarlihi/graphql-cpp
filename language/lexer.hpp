#include "source.hpp"
#include "ast.hpp"

class Lexer
{
private:
    Source source;
    Token *lastToken;
    Token *token;
    int line;
    int lineStart;

public:
    Lexer(Source source);
    Token *advance();
    Token *lookahead();
};