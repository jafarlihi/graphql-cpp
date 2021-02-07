#pragma once

#include <string>
#include <unordered_set>
#include <map>
#include <vector>
#include <sstream>
#include <ostream>
#include <iterator>

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

            friend bool operator==(const SourceLocation &lhs, const SourceLocation &rhs);
    };

    bool operator==(const SourceLocation &lhs, const SourceLocation &rhs) {
        return lhs.line == rhs.line && rhs.column == lhs.column;
    }

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

            Source(std::string body, std::string name = "GraphQL request", SourceLocation *location_offset = new SourceLocation(1, 1)) {
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
                std::vector<std::string> lines = split_string(this->body.substr(0, position), "\n");
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
            std::string description;
            GraphQLSyntaxError(Source *source, int position, std::string description) : GraphQLError(fmt::format("Syntax Error: {}", description), source, position_to_position_list(position)), description{description} { }
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

            friend bool operator==(Token &lhs, Token &rhs);
    };

    bool operator==(Token &lhs, Token &rhs) {
        if (lhs.kind != rhs.kind) return false;
        if (lhs.start != rhs.start) return false;
        if (lhs.end != rhs.end) return false;
        if (lhs.line != rhs.line) return false;
        if (lhs.column != rhs.column) return false;
        if ((lhs.value != nullptr && rhs.value == nullptr) || (lhs.value == nullptr && rhs.value != nullptr)) return false;
        if (lhs.value != rhs.value && *lhs.value != *rhs.value) return false;
        /*
           if ((lhs.prev != nullptr && rhs.prev == nullptr) || (lhs.prev == nullptr && rhs.prev != nullptr)) return false;
           if (lhs.prev != rhs.prev && *lhs.prev != *rhs.prev) return false;
           if ((lhs.next != nullptr && rhs.next == nullptr) || (lhs.next == nullptr && rhs.next != nullptr)) return false;
           if (lhs.next != rhs.next && *lhs.next != *rhs.next) return false;
           */
        return true;
    }

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

    int char2hex(char &character) {
        return (character < '0')  ? -1 :
            (character <= '9') ? (character - '0') :
            (character < 'A')  ? -1 :
            (character <= 'F') ? (character - 'A' + 10) :
            (character < 'a')  ? -1 :
            (character <= 'f') ? (character - 'a' + 10) : -1;
    }

    int uni_char_code(std::string str) {
        return char2hex(str[0]) << 12 | char2hex(str[1]) << 8 | char2hex(str[2]) << 4 | char2hex(str[3]);
    }

    int leading_whitespace(std::string s) {
        int i = 0;
        int n = s.length();
        while (i < n && (s[i] == ' ' || s[i] == '\t'))
            i += 1;
        return i;
    }

    int get_block_string_indentation(std::vector<std::string> lines, bool trailing_culled) {
        int common_indent = -1;
        int i = -1;
        if (trailing_culled)
            i = 0;
        else
            i = 1;
        for (; i < lines.size(); i++) {
            int indent = leading_whitespace(lines[i]);
            if (indent == lines[i].length())
                continue;
            if (common_indent == -1 || indent < common_indent) {
                common_indent = indent;
                if (common_indent == 0)
                    break;
            }
        }
        if (common_indent == -1) return 0;
        return common_indent;
    }

    bool is_blank_string(std::string s) {
        for (int i = 0; i < s.length(); i++)
            if (!std::isspace(s[i]))
                return false;
        return true;
    }

    template <class T, class charT=char, class traits=std::char_traits<charT>>
        class infix_ostream_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {
            std::basic_ostream<charT, traits> *os;
            charT const* delimiter;
            bool first_elem;
            public:
            typedef charT char_type;
            typedef traits traits_type;
            typedef std::basic_ostream<charT, traits> ostream_type;
            infix_ostream_iterator(ostream_type& s) : os(&s), delimiter(0), first_elem(true) {}
            infix_ostream_iterator(ostream_type& s, charT const *d) : os(&s), delimiter(d), first_elem(true) {}
            infix_ostream_iterator<T, charT, traits>& operator=(T const &item) {
                if (!first_elem && delimiter != 0)
                    *os << delimiter;
                *os << item;
                first_elem = false;
                return *this;
            }
            infix_ostream_iterator<T, charT, traits> &operator*() {
                return *this;
            }
            infix_ostream_iterator<T, charT, traits> &operator++() {
                return *this;
            }
            infix_ostream_iterator<T, charT, traits> &operator++(int) {
                return *this;
            }
        };

    std::string *dedent_block_string_value(std::string raw_string) {
        std::vector<std::string> lines = split_string(raw_string, "\n");

        int i = 0;
        bool trailing_culled = false;
        while (i < lines.size() && is_blank_string(lines[i]))
            i++;
        if (i > 0) {
            lines.erase(lines.begin(), lines.begin() + i);
            trailing_culled = true;
        }
        i = lines.size() - 1;
        while (i > 0 && is_blank_string(lines[i]))
            i--;
        if (i < lines.size() - 1 && i >= 0)
            lines.erase(lines.begin() + i + 1, lines.end());

        int common_indent = get_block_string_indentation(lines, trailing_culled);
        if (common_indent) {
            if (trailing_culled)
                i = 0;
            else
                i = 1;
            for (; i < lines.size(); i++)
                lines[i] = lines[i].substr(common_indent, lines[i].length());
        }

        if (lines.size() > 0) {
            const char* const delimiter = "\n";
            std::ostringstream imploded_value;
            std::copy(lines.begin(), lines.end(), infix_ostream_iterator<std::string>(imploded_value, delimiter));
            return new std::string(imploded_value.str());
        } else {
            return new std::string("");
        }
    }

    char get_escaped_character(char &character) {
        switch (character) {
            case 'b':
                return '\b';
            case 'f':
                return '\f';
            case 'n':
                return '\n';
            case 'r':
                return '\r';
            case 't':
                return '\t';
        }
        return '\n';
    }

    class Lexer {
        private:
            Source *source;
            Token *token;
            Token *last_token;
            int line;
            int line_start;
        public:
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

            int read_digits(int start, char &character) {
                Source source = *this->source;
                std::string body = source.body;
                int position = start;
                char current_character = character;
                while ('0' <= current_character && current_character <= '9') {
                    position += 1;
                    current_character = body[position];
                }
                if (position == start) {
                    throw GraphQLSyntaxError(this->source, position, fmt::format("Invalid number, expected digit but got: {}", current_character));
                }
                return position;
            }

            Token *read_string(int start, int line, int col, Token *prev) {
                Source source = *this->source;
                std::string body = source.body;
                int body_length = body.length();
                int position = start + 1;
                int chunk_start = position;
                std::vector<std::string> value;

                while (position < body_length) {
                    char &character = body[position];
                    if (character == '\n' || character == '\r')
                        break;
                    if (character == '"') {
                        value.push_back(std::string(&body[chunk_start], &body[position]));
                        const char* const delimiter = "";
                        std::ostringstream imploded_value;
                        std::copy(value.begin(), value.end(), std::ostream_iterator<std::string>(imploded_value, delimiter));
                        return new Token(TokenKind::STRING, start, position + 1, line, col, prev, new std::string(imploded_value.str()));
                    }
                    if (character < ' ' && character != '\t') {
                        throw GraphQLSyntaxError(this->source, position, fmt::format("Invalid character within String: {}", character));
                    }
                    position += 1;
                    if (character == '\\') {
                        value.push_back(std::string(&body[chunk_start], &body[position - 1]));
                        character = body[position];
                        if (character == '"' || character == '/' || character == '\\') {
                            value.push_back(std::string(&body[position], &body[position + 1]));
                        } else if (character == 'b' || character == 'f' || character == 'n' || character == 'r' || character == 't') {
                            value.push_back(std::string(1, get_escaped_character(character)));
                        } else if (character == 'u' && position + 4 < body_length) {
                            int code = uni_char_code(std::string(&body[position + 1], &body[position + 5]));
                            if (code < 0) {
                                std::string escape = std::string(&body[position], &body[position + 5]);
                                throw GraphQLSyntaxError(this->source, position, fmt::format("Invalid character escape sequence: {}", escape));
                            }
                            value.push_back(std::string(1, static_cast<char>(code)));
                            position += 4;
                        } else {
                            std::string escape = std::string(1, character);
                            throw GraphQLSyntaxError(this->source, position, fmt::format("Invalid character escape sequence: {}", escape));
                        }
                        position += 1;
                        chunk_start = position;
                    }
                }

                throw GraphQLSyntaxError(this->source, position, "Unterminated string.");
            }

            Token *read_block_string(int start, int line, int col, Token *prev) {
                Source source = *this->source;
                std::string body = source.body;
                int body_length = body.length();
                int position = start + 3;
                int chunk_start = position;
                std::string raw_value = "";

                while (position < body_length) {
                    char &character = body[position];
                    if (character == '"' && std::string(&body[position + 1], &body[position + 3]) == "\"\"") {
                        raw_value.append(std::string(&body[chunk_start], &body[position]));
                        return new Token(TokenKind::BLOCK_STRING, start, position + 3, line, col, prev, dedent_block_string_value(raw_value));
                    }
                    if (character < ' ' && character != '\t' && character != '\n' && character != '\r') {
                        throw GraphQLSyntaxError(this->source, position, fmt::format("Invalid character within String: {}", character));
                    }
                    if (character == '\n') {
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
                    } else if (character == '\\' && std::string(&body[position + 1], &body[position + 4]) == "\"\"\"") {
                        raw_value.append(std::string(&body[chunk_start], &body[position]));
                        position += 4;
                        chunk_start = position;
                    } else {
                        position += 1;
                    }
                }

                throw GraphQLSyntaxError(this->source, position, "Unterminated string.");
            }


            Token *read_name(int start, int line, int col, Token *prev) {
                std::string body = this->source->body;
                int body_length = body.length();
                int position = start + 1;
                while (position < body_length) {
                    char &character = body[position];
                    if (!(character == '_' || ('0' <= character && character <= '9') || ('A' <= character && character <= 'Z') || ('a' <= character && character <= 'z')))
                        break;
                    position += 1;
                }
                return new Token(TokenKind::NAME, start, position, line, col, prev, new std::string(&body[start], &body[position]));
            }


            int position_after_whitespace(std::string body, int start_position) {
                int body_length = body.length();
                int position = start_position;
                while (position < body_length) {
                    char &character = body[position];
                    std::string character_set = " \t,\ufeff"; // TODO: BOM checked against correctly?
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

