#include "graphql-cpp.hpp"

#include <cassert>
#include <iostream>

#include <fmt/core.h>

using namespace graphql;

Token *lex_one(std::string s) {
    Lexer *lexer = new Lexer(new Source(s));
    return lexer->advance();
}

Token *lex_second(std::string s) {
    Lexer *lexer = new Lexer(new Source(s));
    lexer->advance();
    return lexer->advance();
}

void assert_syntax_error(std::string text, std::string message, SourceLocation location) {
    try {
        lex_second(text);
        assert(false);
    } catch (GraphQLSyntaxError e) {
        assert(e.message == fmt::format("Syntax Error: {}", message));
        assert(e.description == message);
        std::vector<SourceLocation> locations{location};
        assert(*e.locations == locations);
    }
}

int main(int argc, char *argv[]) {
    Token *token = nullptr;

    //token = lex_one("\ufeff foo");
    //assert(*token == Token(TokenKind::NAME, 2, 5, 1, 3, nullptr, new std::string("foo")));
    // TODO: Fix

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

    try {
        token = lex_one("\n\n    ?\n");
        assert(false);
    } catch (GraphQLSyntaxError e) {
        assert(e.message == "Syntax Error: Cannot parse the unexpected character ?.");
    }

    token = lex_one("\"\"");
    expected_token = Token(TokenKind::STRING, 0, 2, 1, 1, nullptr, new std::string(""));
    assert(*token == expected_token);

    token = lex_one("\"simple\"");
    expected_token = Token(TokenKind::STRING, 0, 8, 1, 1, nullptr, new std::string("simple"));
    assert(*token == expected_token);

    token = lex_one("\" white space \"");
    expected_token = Token(TokenKind::STRING, 0, 15, 1, 1, nullptr, new std::string(" white space "));
    assert(*token == expected_token);

    token = lex_one("\"quote \\\"\"");
    expected_token = Token(TokenKind::STRING, 0, 10, 1, 1, nullptr, new std::string("quote \""));
    assert(*token == expected_token);

    token = lex_one("\"escaped \\n\\r\\b\\t\\f\"");
    expected_token = Token(TokenKind::STRING, 0, 20, 1, 1, nullptr, new std::string("escaped \n\r\b\t\f"));
    assert(*token == expected_token);

    token = lex_one("\"slashes \\\\ \\/\"");
    expected_token = Token(TokenKind::STRING, 0, 15, 1, 1, nullptr, new std::string("slashes \\ /"));
    assert(*token == expected_token);

    //token = lex_one("\"unicode \\u1234\\u5678\\u90AB\\uCDEF\"");
    //expected_token = Token(TokenKind::STRING, 0, 34, 1, 1, nullptr, new std::string("unicode \u1234\u5678\u90AB\uCDEF"));
    //assert(*token == expected_token);
    // TODO: Fix

    assert_syntax_error("\"", "Unterminated string.", SourceLocation(1, 2));
    assert_syntax_error("\"\"\"", "Unterminated string.", SourceLocation(1, 4));
    assert_syntax_error("\"\"\"\"", "Unterminated string.", SourceLocation(1, 5));
    assert_syntax_error("\"no end quote", "Unterminated string.", SourceLocation(1, 14));
    assert_syntax_error("'single quotes'", "Unexpected single quote character ('), did you mean to use a double quote (\")?", SourceLocation(1, 1));
    //assert_syntax_error("\"contains unescaped \x07 control char\"", "Invalid character within String: '\\x07'.", SourceLocation(1, 21));
    //assert_syntax_error("\"null-byte is not \x00 end of file\"", "Invalid character within String: '\\x00'.", SourceLocation(1, 19));
    // TODO: Fix
    assert_syntax_error("\"multi\nline\"", "Unterminated string.", SourceLocation(1, 7));
    assert_syntax_error("\"multi\rline\"", "Unterminated string.", SourceLocation(1, 7));
    //assert_syntax_error("\"bad \\x esc\"", "Invalid character escape sequence: '\\x'.", SourceLocation(1, 7));
    //assert_syntax_error("\"bad \\u1 esc\"", "Invalid character escape sequence: '\\u1 es'.", SourceLocation(1, 7));
    //assert_syntax_error("\"bad \\u0XX1 esc\"", "Invalid character escape sequence: '\\u0XX1'.", SourceLocation(1, 7));
    //assert_syntax_error("\"bad \\uXXXX esc\"", "Invalid character escape sequence: '\\uXXXX'.", SourceLocation(1, 7));
    //assert_syntax_error("\"bad \\uFXXX esc\"", "Invalid character escape sequence: '\\uFXXX'.", SourceLocation(1, 7));
    //assert_syntax_error("\"bad \\uXXXF esc\"", "Invalid character escape sequence: '\\uXXXF'.", SourceLocation(1, 7));
    // TODO: Fix

    token = lex_one("\"\"\"\"\"\"");
    expected_token = Token(TokenKind::BLOCK_STRING, 0, 6, 1, 1, nullptr, new std::string(""));
    assert(*token == expected_token);

    token = lex_one("\"\"\"simple\"\"\"");
    expected_token = Token(TokenKind::BLOCK_STRING, 0, 12, 1, 1, nullptr, new std::string("simple"));
    assert(*token == expected_token);

    token = lex_one("\"\"\" white space \"\"\"");
    expected_token = Token(TokenKind::BLOCK_STRING, 0, 19, 1, 1, nullptr, new std::string(" white space "));
    assert(*token == expected_token);

    token = lex_one("\"\"\"contains \" quote\"\"\"");
    expected_token = Token(TokenKind::BLOCK_STRING, 0, 22, 1, 1, nullptr, new std::string("contains \" quote"));
    assert(*token == expected_token);

    //token = lex_one("\"\"\"contains \\\"\"\" triple-quote\"\"\"");
    //expected_token = Token(TokenKind::BLOCK_STRING, 0, 32, 1, 1, nullptr, new std::string("contains \"\"\" triple-quote"));
    //assert(*token == expected_token);
    // TODO: Fix

    token = lex_one("\"\"\"multi\nline\"\"\"");
    expected_token = Token(TokenKind::BLOCK_STRING, 0, 16, 1, 1, nullptr, new std::string("multi\nline"));
    assert(*token == expected_token);

    //token = lex_one("\"\"\"multi\rline\r\nnormalized\"\"\"");
    //expected_token = Token(TokenKind::BLOCK_STRING, 0, 28, 1, 1, nullptr, new std::string("multi\nline\nnormalized"));
    //assert(*token == expected_token);
    // TODO: Fix

    token = lex_one("\"\"\"unescaped \\n\\r\\b\\t\\f\\u1234\"\"\"");
    expected_token = Token(TokenKind::BLOCK_STRING, 0, 32, 1, 1, nullptr, new std::string("unescaped \\n\\r\\b\\t\\f\\u1234"));
    assert(*token == expected_token);

    token = lex_one("\"\"\"slashes \\\\ \\/\"\"\"");
    expected_token = Token(TokenKind::BLOCK_STRING, 0, 19, 1, 1, nullptr, new std::string("slashes \\\\ \\/"));
    assert(*token == expected_token);

    token = lex_one("\"\"\"\n\n        spans\n          multiple\n            lines\n\n        \"\"\"");
    expected_token = Token(TokenKind::BLOCK_STRING, 0, 68, 1, 1, nullptr, new std::string("spans\n  multiple\n    lines"));
    assert(*token == expected_token);

    /*token = lex_second("\"\"\""
            ""
            "        spans"
            "          multiple"
            "            lines"
            ""
            "        \n \"\"\" second_token"
            );
    expected_token = Token(TokenKind::NAME, 71, 83, 8, 6, nullptr, new std::string("second_token"));
    std::cout << token->start << " " << token->end << " " << token->column << " " << token->line << std::endl;
    assert(*token == expected_token);*/
    // TODO: Fix

    assert_syntax_error("\"\"\"", "Unterminated string.", SourceLocation(1, 4));
    assert_syntax_error("\"\"\"no end quite", "Unterminated string.", SourceLocation(1, 16));
    //assert_syntax_error("\"\"\"contains unescaped \x07 control char\"\"\"", "Invalid character within String: '\\x07'.", SourceLocation(1, 23));
    //assert_syntax_error("\"\"\"null-byte is not \x00 end of file\"\"\"", "Invalid character within String: '\\x00'.", SourceLocation(1, 21));
    // TODO: Fix

    token = lex_one("0");
    expected_token = Token(TokenKind::INT, 0, 1, 1, 1, nullptr, new std::string("0"));
    assert(*token == expected_token);

    token = lex_one("1");
    expected_token = Token(TokenKind::INT, 0, 1, 1, 1, nullptr, new std::string("1"));
    assert(*token == expected_token);

    token = lex_one("4");
    expected_token = Token(TokenKind::INT, 0, 1, 1, 1, nullptr, new std::string("4"));
    assert(*token == expected_token);

    token = lex_one("9");
    expected_token = Token(TokenKind::INT, 0, 1, 1, 1, nullptr, new std::string("9"));
    assert(*token == expected_token);

    token = lex_one("42");
    expected_token = Token(TokenKind::INT, 0, 2, 1, 1, nullptr, new std::string("42"));
    assert(*token == expected_token);

    token = lex_one("4.123");
    expected_token = Token(TokenKind::FLOAT, 0, 5, 1, 1, nullptr, new std::string("4.123"));
    assert(*token == expected_token);

    token = lex_one("-4");
    expected_token = Token(TokenKind::INT, 0, 2, 1, 1, nullptr, new std::string("-4"));
    assert(*token == expected_token);

    token = lex_one("-42");
    expected_token = Token(TokenKind::INT, 0, 3, 1, 1, nullptr, new std::string("-42"));
    assert(*token == expected_token);

    token = lex_one("-4.123");
    expected_token = Token(TokenKind::FLOAT, 0, 6, 1, 1, nullptr, new std::string("-4.123"));
    assert(*token == expected_token);

    token = lex_one("0.123");
    expected_token = Token(TokenKind::FLOAT, 0, 5, 1, 1, nullptr, new std::string("0.123"));
    assert(*token == expected_token);

    token = lex_one("123e4");
    expected_token = Token(TokenKind::FLOAT, 0, 5, 1, 1, nullptr, new std::string("123e4"));
    assert(*token == expected_token);

    token = lex_one("123E4");
    expected_token = Token(TokenKind::FLOAT, 0, 5, 1, 1, nullptr, new std::string("123E4"));
    assert(*token == expected_token);

    token = lex_one("123e-4");
    expected_token = Token(TokenKind::FLOAT, 0, 6, 1, 1, nullptr, new std::string("123e-4"));
    assert(*token == expected_token);

    token = lex_one("123e+4");
    expected_token = Token(TokenKind::FLOAT, 0, 6, 1, 1, nullptr, new std::string("123e+4"));
    assert(*token == expected_token);

    token = lex_one("-1.123e4");
    expected_token = Token(TokenKind::FLOAT, 0, 8, 1, 1, nullptr, new std::string("-1.123e4"));
    assert(*token == expected_token);

    token = lex_one("-1.123E4");
    expected_token = Token(TokenKind::FLOAT, 0, 8, 1, 1, nullptr, new std::string("-1.123E4"));
    assert(*token == expected_token);

    token = lex_one("-1.123e-4");
    expected_token = Token(TokenKind::FLOAT, 0, 9, 1, 1, nullptr, new std::string("-1.123e-4"));
    assert(*token == expected_token);

    token = lex_one("-1.123e+4");
    expected_token = Token(TokenKind::FLOAT, 0, 9, 1, 1, nullptr, new std::string("-1.123e+4"));
    assert(*token == expected_token);

    token = lex_one("-1.123e4567");
    expected_token = Token(TokenKind::FLOAT, 0, 11, 1, 1, nullptr, new std::string("-1.123e4567"));
    assert(*token == expected_token);
}

