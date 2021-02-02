#pragma once

#include <string>
#include <unordered_set>
#include <map>
#include <vector>

#include <fmt/core.h>

#undef EOF

enum TokenKind {
    SOF,
    EOF,
    BANG,
    DOLLAR,
    AMP,
    PAREN_L,
    PAREN_R,
    SPREAD,
    COLON,
    EQUALS,
    AT,
    BRACKET_L,
    BRACKET_R,
    BRACE_L,
    PIPE,
    BRACE_R,
    NAME,
    INT,
    FLOAT,
    STRING,
    BLOCK_STRING,
    COMMENT
};

class SourceLocation {
    public:
        int line;
        int column;
};

class Source {
    public:
        std::string body;
        std::string name;
        SourceLocation location_offset;
        Source(std::string body, std::string name, SourceLocation location_offset) {
            this->body = body;
            this->name = name;
            if (location_offset.line <= 0) {
                throw std::invalid_argument("line in location_offset is 1-indexed and must be positive.");
            }
            if (location_offset.column <= 0) {
                throw std::invalid_argument("column in location_offset is 1-indexed and must be positive.");
            }
            this->location_offset = location_offset;
        }
};

class GraphQLError : public std::exception {
    public:
        std::string message;
        std::vector<SourceLocation> locations;
        Source *source;
        std::vector<int> positions;
        std::exception *original_error;
        GraphQLError(std::string message, Source *source = nullptr, std::vector<int> positions = nullptr);
};

class Token {
    public:
        TokenKind kind;
        int start;
        int end;
        int line;
        int column;
        std::string *value;
        Token *prev;
        Token *next;

        Token(TokenKind kind, int start, int end, int line, int column, Token *prev = nullptr, std::string *value = nullptr) {
            this->kind = kind;
            this->start = start;
            this->end = end;
            this->line = line;
            this->column = column;
            this->value = value;
            this->prev = prev;
            this->next = nullptr;
        }
};

std::unordered_set<TokenKind> punctuator_token_kinds {
    TokenKind::BANG,
        TokenKind::DOLLAR,
        TokenKind::AMP,
        TokenKind::PAREN_L,
        TokenKind::PAREN_R,
        TokenKind::SPREAD,
        TokenKind::COLON,
        TokenKind::EQUALS,
        TokenKind::AT,
        TokenKind::BRACKET_L,
        TokenKind::BRACKET_R,
        TokenKind::BRACE_L,
        TokenKind::PIPE,
        TokenKind::BRACE_R,
};

std::map<char, TokenKind> single_character_token_kinds {
    {'!', TokenKind::BANG},
        {'$', TokenKind::DOLLAR},
        {'&', TokenKind::AMP},
        {'(', TokenKind::PAREN_L},
        {')', TokenKind::PAREN_R},
        {':', TokenKind::COLON},
        {'=', TokenKind::EQUALS},
        {'@', TokenKind::AT},
        {'[', TokenKind::BRACKET_L},
        {']', TokenKind::BRACKET_R},
        {'{', TokenKind::BRACE_L},
        {'}', TokenKind::BRACE_R},
        {'|', TokenKind::PIPE}
};

std::string unexpected_character_message(char &character) {
    if (character < ' ' && character != '\t' && character != '\n' && character != '\r')
        return fmt::format("Cannot contain the invalid character {}.", character);
    if (character == '\'')
        return "Unexpected single quote character ('), did you mean to use a double quote (\")?";
    return fmt::format("Cannot parse the unexpected character {}.", character);
}

class Lexer {
    private:
        Source *source;
        Token *token;
        Token *last_token;
        int line;
        int line_start;
    public:
        int position_after_whitespace(std::string body, int start_position);
        Token *read_comment(int start, int line, int col, Token *prev);
        Token *read_number(int start, char &character, int line, int col, Token *prev);
        int read_digits(int start, char &character);
        Token *read_string(int start, int line, int col, Token *prev);
        Token *read_block_string(int start, int line, int col, Token *prev);
        Token *read_name(int start, int line, int col, Token *prev);

        Lexer(Source *source) {
            this->source = source;
            this->token = this->last_token = new Token(TokenKind::SOF, 0, 0, 0, 0);
            this->line = 1;
            this->line_start = 0;
        }

        Token *advance() {
            this->last_token = this->token;
            this->token = this->lookahead();
            return this->token;
        }

        Token *lookahead() {
            Token *token = this->token;
            if (token->kind != TokenKind::EOF) {
                while (true) {
                    if (token->next == nullptr)
                        token->next = this->read_token(token);
                    token = token->next;
                    if (token->kind != TokenKind::COMMENT)
                        break;
                }
            }
            return token;
        }

        Token *read_token(Token *prev) {
            Source *source = this->source;
            std::string body = source->body;
            int body_length = body.length();

            int pos = this->position_after_whitespace(body, prev->end);
            int line = this->line;
            int col = 1 + pos - this->line_start;

            if (pos >= body_length)
                return new Token(TokenKind::EOF, body_length, body_length, line, col, prev);

            char &character = body[pos];
            bool kind_fetched = true;
            TokenKind kind;
            try {
                kind = single_character_token_kinds.at(character);
            } catch (std::out_of_range e) {
                kind_fetched = false;
            }
            if (kind_fetched)
                return new Token(kind, pos, pos + 1, line, col, prev);
            if (character == '#')
                return this->read_comment(pos, line, col, prev);
            else if (character == '.') {
                if (body[pos + 1] == '.' && body[pos + 2] == '.')
                    return new Token(TokenKind::SPREAD, pos, pos + 3, line, col, prev);
            }
            else if ((('A' <= character) && (character <= 'Z')) || (('a' <= character) && (character <= 'z')) || character == '_')
                return this->read_name(pos, line, col, prev);
            else if ((('0' <= character) && (character <= '9')) || character == '-')
                return this->read_number(pos, character, line, col, prev);
            else if (character == '"') {
                if (body[pos + 1] == '"' && body[pos + 2] == '"')
                    return this->read_block_string(pos, line, col, prev);
                return this->read_string(pos, line, col, prev);
            }

            throw GraphQLSyntaxError(source, pos, unexpected_character_message(character));
        }
};

