#pragma once

#include <string>
#include <unordered_set>
#include <map>
#include <vector>

#include <fmt/core.h>

#undef EOF

namespace graphql {
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

            SourceLocation(int line, int column) {
                this->line = line;
                this->column = column;
            }
    };

    std::vector<std::string> split_string(const std::string &str, const std::string &delimiter) {
        std::vector<std::string> strings;
        std::string::size_type pos = 0;
        std::string::size_type prev = 0;
        while ((pos = str.find(delimiter, prev)) != std::string::npos) {
            strings.push_back(str.substr(prev, pos - prev));
            prev = pos + 1;
        }
        strings.push_back(str.substr(prev));
        return strings;
    }

    class Source {
        public:
            std::string body;
            std::string name;
            SourceLocation *location_offset;

            Source(std::string body, std::string name, SourceLocation *location_offset) {
                this->body = body;
                this->name = name;
                if (location_offset->line <= 0) {
                    throw std::invalid_argument("line in location_offset is 1-indexed and must be positive.");
                }
                if (location_offset->column <= 0) {
                    throw std::invalid_argument("column in location_offset is 1-indexed and must be positive.");
                }
                this->location_offset = location_offset;
            }

            SourceLocation get_location(int position) {
                std::vector<std::string> lines = graphql::split_string(this->body.substr(0, position), "\n");
                int line, column;
                if (lines.size() > 0) {
                    line = lines.size();
                    column = lines.back().length() + 1;
                } else {
                    line = 1;
                    column = 1;
                }
                return SourceLocation(line, column);
            }
    };

    class GraphQLError : public std::exception {
        public:
            std::string message;
            std::vector<SourceLocation> *locations;
            Source *source;
            std::vector<int> *positions;
            std::exception *original_error;

            GraphQLError(std::string message, Source *source = nullptr, std::vector<int> *positions = nullptr) {
                this->message = message;
                this->source = source;
                this->positions = positions;
                if (positions != nullptr && source != nullptr) {
                    std::vector<SourceLocation> *locations = new std::vector<SourceLocation>();
                    for (auto &position : *positions) {
                        SourceLocation location = source->get_location(position);
                        locations->push_back(location);
                    }
                    this->locations = locations;
                }
            }
    };

    std::vector<int> *position_to_position_list(int position) {
        std::vector<int> *result = new std::vector<int>();
        result->push_back(position);
        return result;
    }

    class GraphQLSyntaxError : public GraphQLError {
        public:
            GraphQLSyntaxError(Source *source, int position, std::string description) : GraphQLError(fmt::format("Syntax Error: {}", description), source, position_to_position_list(position)) { }
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

    bool is_name_start(char &character) {
        return character == '_' || ('A' <= character && character <= 'Z') || ('a' <= character && character <= 'z');
    }

    class Lexer {
        private:
            Source *source;
            Token *token;
            Token *last_token;
            int line;
            int line_start;
        public:
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

            Token *read_comment(int start, int line, int col, Token *prev) {
                std::string body = this->source->body;
                int body_length = body.length();
                int position = start;
                while (true) {
                    position += 1;
                    if (position >= body_length)
                        break;
                    char &character = body[position];
                    if (character < ' ' && character != '\t')
                        break;
                }
                return new Token(TokenKind::COMMENT, start, position, line, col, prev, new std::string(&body[start + 1], &body[position]));
            }

            Token *read_number(int start, char &character, int line, int col, Token *prev) {
                Source source = *this->source;
                std::string body = source.body;
                int position = start;
                bool is_float = false;
                char current_character = character;
                if (current_character == '-') {
                    position += 1;
                    current_character = body[position];
                }
                if (current_character == '0') {
                    position += 1;
                    current_character = body[position];
                    if ('0' <= current_character && current_character <= '9') {
                        throw GraphQLSyntaxError(this->source, position, fmt::format("Invalid number, unexpected digit after 0: {}", current_character));
                    }
                } else {
                    position = read_digits(position, current_character);
                    current_character = body[position];
                }
                if (current_character == '.') {
                    is_float = true;
                    position += 1;
                    current_character = body[position];
                    position = read_digits(position, current_character);
                    current_character = body[position];
                }
                if (current_character == 'e' || current_character == 'E') {
                    is_float = true;
                    position += 1;
                    current_character = body[position];
                    if (current_character == '+' || current_character == '-') {
                        position += 1;
                        current_character = body[position];
                    }
                    position = read_digits(position, current_character);
                    current_character = body[position];
                }

                if (current_character == '.' || is_name_start(current_character)) {
                    throw GraphQLSyntaxError(this->source, position, fmt::format("Invalid number, expected digit but got: {}", current_character));
                }

                TokenKind kind;
                if (is_float)
                    kind = TokenKind::FLOAT;
                else
                    kind = TokenKind::INT;
                return new Token(kind, start, position, line, col, prev, new std::string(&body[start], &body[position]));
            }

            int position_after_whitespace(std::string body, int start_position) {
                int body_length = body.length();
                int position = start_position;
                while (position < body_length) {
                    char &character = body[position];
                    std::string character_set = " \t,\ufeff";
                    if (character_set.find(character) != std::string::npos) {
                        position += 1;
                    } else if (character == '\n') {
                        position += 1;
                        this->line += 1;
                        this->line_start = position;
                    } else if (character == '\r') {
                        if (body[position + 1] == '\n')
                            position += 2;
                        else
                            position += 1;
                        this->line += 1;
                        this->line_start = position;
                    } else {
                        break;
                    }
                }
                return position;
            }
    };
}

