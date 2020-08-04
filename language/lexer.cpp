#include "lexer.hpp"

Lexer::Lexer(Source source) {
    Token *startOfFileToken =
        new Token(TokenKind::SOF, 0, 0, 0, 0, nullptr, nullptr);

    this->source = source;
    this->lastToken = startOfFileToken;
    this->token = startOfFileToken;
    this->line = 1;
    this->lineStart = 0;
}

Token *Lexer::advance() {
    this->lastToken = this->token;
    this->token = this->lookahead();
    return this->token;
}

Token *Lexer::lookahead() {
    Token *token = this->token;
    if (token->kind != TokenKind::EOFILE) {
        do {
            if (token->next != nullptr)
                token = token->next;
            else
                token->next = readToken(this, token);
        } while (token->kind != TokenKind::COMMENT);
    }
}

Token *readToken(Lexer *lexer, Token *prev) {
    Source source = lexer->source;
    std::string body = source.body;
    int bodyLength = body.length();

    int pos = prev->end;
    while (pos < bodyLength) {
        int code = int(body[pos]);

        int line = lexer->line;
        int col = 1 + pos - lexer->lineStart;

        switch (code) {
            case 9:   //   \t
            case 32:  //  <space>
            case 44:  //  ,
                ++pos;
                continue;
            case 10:  //  \n
                ++pos;
                ++lexer->line;
                lexer->lineStart = pos;
                continue;
            case 13:  //  \r
                if (bodyLength > pos + 1 && int(body[pos + 1] == 10)) {
                    pos += 2;
                } else {
                    ++pos;
                }
                ++lexer->line;
                lexer->lineStart = pos;
                continue;
            case 33:  //  !
                return new Token(TokenKind::BANG, pos, pos + 1, line, col, prev,
                                 nullptr);
            case 35:  //  #
                return readComment(source, pos, line, col, prev);
            case 36:  //  $
                return new Token(TokenKind::DOLLAR, pos, pos + 1, line, col,
                                 prev, nullptr);
            case 38:  //  &
                return new Token(TokenKind::AMP, pos, pos + 1, line, col, prev,
                                 nullptr);
            case 40:  //  (
                return new Token(TokenKind::PAREN_L, pos, pos + 1, line, col,
                                 prev, nullptr);
            case 41:  //  )
                return new Token(TokenKind::PAREN_R, pos, pos + 1, line, col,
                                 prev, nullptr);
            case 46:  //  .
                if (int(body[pos + 1]) == 46 && int(body[pos + 2]) == 46) {
                    return new Token(TokenKind::SPREAD, pos, pos + 3, line, col,
                                     prev, nullptr);
                }
                break;
            case 58:  //  :
                return new Token(TokenKind::COLON, pos, pos + 1, line, col,
                                 prev, nullptr);
            case 61:  //  =
                return new Token(TokenKind::EQUALS, pos, pos + 1, line, col,
                                 prev, nullptr);
            case 64:  //  @
                return new Token(TokenKind::AT, pos, pos + 1, line, col, prev,
                                 nullptr);
            case 91:  //  [
                return new Token(TokenKind::BRACKET_L, pos, pos + 1, line, col,
                                 prev, nullptr);
            case 93:  //  ]
                return new Token(TokenKind::BRACKET_R, pos, pos + 1, line, col,
                                 prev, nullptr);
            case 123:  // {
                return new Token(TokenKind::BRACE_L, pos, pos + 1, line, col,
                                 prev, nullptr);
            case 124:  // |
                return new Token(TokenKind::PIPE, pos, pos + 1, line, col, prev,
                                 nullptr);
            case 125:  // }
                return new Token(TokenKind::BRACE_R, pos, pos + 1, line, col,
                                 prev, nullptr);
            case 34:  //  "
                if (bodyLength > pos + 2 && int(body[pos + 1]) == 34 &&
                    int(body[pos + 2] == 34)) {
                    return readBlockString(source, pos, line, col, prev, lexer);
                }
                return readString(source, pos, line, col, prev);
            case 45:  //  -
            case 48:  //  0
            case 49:  //  1
            case 50:  //  2
            case 51:  //  3
            case 52:  //  4
            case 53:  //  5
            case 54:  //  6
            case 55:  //  7
            case 56:  //  8
            case 57:  //  9
                return readNumber(source, pos, code, line, col, prev);
            case 65:   //  A
            case 66:   //  B
            case 67:   //  C
            case 68:   //  D
            case 69:   //  E
            case 70:   //  F
            case 71:   //  G
            case 72:   //  H
            case 73:   //  I
            case 74:   //  J
            case 75:   //  K
            case 76:   //  L
            case 77:   //  M
            case 78:   //  N
            case 79:   //  O
            case 80:   //  P
            case 81:   //  Q
            case 82:   //  R
            case 83:   //  S
            case 84:   //  T
            case 85:   //  U
            case 86:   //  V
            case 87:   //  W
            case 88:   //  X
            case 89:   //  Y
            case 90:   //  Z
            case 95:   //  _
            case 97:   //  a
            case 98:   //  b
            case 99:   //  c
            case 100:  // d
            case 101:  // e
            case 102:  // f
            case 103:  // g
            case 104:  // h
            case 105:  // i
            case 106:  // j
            case 107:  // k
            case 108:  // l
            case 109:  // m
            case 110:  // n
            case 111:  // o
            case 112:  // p
            case 113:  // q
            case 114:  // r
            case 115:  // s
            case 116:  // t
            case 117:  // u
            case 118:  // v
            case 119:  // w
            case 120:  // x
            case 121:  // y
            case 122:  // z
                return readName(source, pos, line, col, prev);
        }
    }
}

Token *readComment(Source source, int start, int line, int col, Token *prev) {}

Token *readBlockString(Source source, int start, int line, int col, Token *prev,
                       Lexer *lexer) {}

Token *readString(Source source, int start, int line, int col, Token *prev) {}

Token *readNumber(Source source, int start, int firstCode, int line, int col,
                  Token *prev) {}

Token *readName(Source source, int start, int line, int col, Token *prev) {}