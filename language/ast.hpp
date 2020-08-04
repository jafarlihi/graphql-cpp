#include <string>

#include "tokenKind.hpp"

class Token
{
private:
    int start;
    int end;
    int line;
    int column;
    std::string *value;
    Token *prev;
    Token *next;

public:
    TokenKind kind;
    Token(TokenKind kind, int start, int end, int line, int column, Token *prev, std::string *value);
};