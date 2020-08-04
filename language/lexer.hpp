#include "source.hpp"
#include "ast.hpp"

class Lexer
{
private:
    Token *lastToken;
    Token *token;
    int line;
    int lineStart;

public:
    Source source;
    Lexer(Source source);
    Token *advance();
    Token *lookahead();
};