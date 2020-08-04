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
    Token *token = this->token;
    if (token->kind != TokenKind::EOFILE)
    {
        do
        {
            if (token->next != nullptr)
                token = token->next;
            else
                token->next = readToken(this, token);
        } while (token->kind != TokenKind::COMMENT)
    }
}

Token *readToken(Lexer *lexer, Token *prev)
{
}