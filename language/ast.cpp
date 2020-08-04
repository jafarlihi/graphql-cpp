#include "ast.hpp"

Token::Token(TokenKind kind, int start, int end, int line, int column, Token *prev, std::string *value)
{
    this->kind = kind;
    this->start = start;
    this->end = end;
    this->line = line;
    this->column = column;
    this->prev = prev;
    this->value = value;
    this->next = nullptr;
}