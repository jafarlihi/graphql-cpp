#include "lexer.hpp"

Lexer::Lexer(Source source)
{
    Token *startOfFileToken = new Token(TokenKind::SOF, 0, 0, 0, 0, nullptr, nullptr);

    this->source = source;
    this->lastToken = startOfFileToken;
    this->token = startOfFileToken;
    this->line = 1;
    this->lineStart = 0;
}

Token *Lexer::advance()
{
    this->lastToken = this->token;
    this->token = this->lookahead();
    return this->token;
}

Token *Lexer::lookahead()
{
}