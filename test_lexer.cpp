#include "graphql-cpp.hpp"

#include <cassert>
#include <iostream>

using namespace graphql;

Token *lex_one(std::string s) {
    Lexer *lexer = new Lexer(new Source(s));
    return lexer->advance();
}

int main(int argc, char *argv[]) {
    Token *token = lex_one("\ufeff foo");
    //assert(*token == Token(TokenKind::NAME, 2, 5, 1, 3, nullptr, new std::string("foo")));
    // TODO: Broken

    token = lex_one("foo");
    Token expected_token = Token(TokenKind::NAME, 0, 3, 1, 1, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\nfoo");
    expected_token = Token(TokenKind::NAME, 1, 4, 2, 1, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\rfoo");
    expected_token = Token(TokenKind::NAME, 1, 4, 2, 1, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\r\nfoo");
    expected_token = Token(TokenKind::NAME, 2, 5, 2, 1, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\n\rfoo");
    expected_token = Token(TokenKind::NAME, 2, 5, 3, 1, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\r\r\n\nfoo");
    expected_token = Token(TokenKind::NAME, 4, 7, 4, 1, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\n\n\r\rfoo");
    expected_token = Token(TokenKind::NAME, 4, 7, 5, 1, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\n \r\n \r  foo\n");
    expected_token = Token(TokenKind::NAME, 8, 11, 4, 3, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\n\n    foo\n\n\n");
    expected_token = Token(TokenKind::NAME, 6, 9, 3, 5, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\r\n\r\n  foo\r\n\r\n");
    expected_token = Token(TokenKind::NAME, 6, 9, 3, 3, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\r\r  foo\r\r");
    expected_token = Token(TokenKind::NAME, 4, 7, 3, 3, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\t\t  foo\t\t");
    expected_token = Token(TokenKind::NAME, 4, 7, 1, 5, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one("\n    #comment\n    foo#comment\n");
    expected_token = Token(TokenKind::NAME, 18, 21, 3, 5, nullptr, new std::string("foo"));
    assert(*token == expected_token);

    token = lex_one(",,,foo,,,");
    expected_token = Token(TokenKind::NAME, 3, 6, 1, 4, nullptr, new std::string("foo"));
    assert(*token == expected_token);
}

