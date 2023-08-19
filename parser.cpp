#include "token.h"
#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

using u32 = uint32_t;

// extra padding to avoid edgecases with keywords at end of file
static constexpr u32 PADDING_BYTES = 8;

class File {
public:
    File(std::string_view path)
        : handle(open(path.data(), O_RDONLY)) {
        if (handle == -1) {
            throw 1; // TODO proper exception handling (in whole file)
        }
    }

    ~File() {
        close(handle);
    }

    std::string read() const {
        auto size = lseek(handle, 0, SEEK_END);
        if (size == -1) {
            throw 2;
        }

        lseek(handle, 0, SEEK_SET);  // rewind

        std::string result;
        result.reserve(size + 1 + PADDING_BYTES);

        // NOTE: includes no EOF
        if (::read(handle, result.data(), size) != size) {
            return "";
        }

        memset (result.data() + size, 0, 1 + PADDING_BYTES);
        return result;
    }

private:
    int handle;
};

class Parser {
    using Token = token::Token;

public:
    Parser()
        : tokens()
        , positions()
        , pos(0)
        , file_size(0) {}

    void parse(std::string_view path) {
        File f(path);
        auto contents = f.read();
        file_size = contents.capacity();
        auto data = contents.c_str();

        while (is_not_at_eof()) {
            auto current_pos = pos;
            parse_whitespace(&data);
            auto t = parse_next(&data);
            tokens.push_back(t);
            positions.push_back(current_pos);
        }

        tokens.push_back(token::END);
        positions.push_back(pos);
    }

    void print_tokens() const {
        File f("./path.eclair");
        auto contents = f.read();
        auto str = contents.c_str();

        for (auto i = 0u; i < tokens.size(); i++) {
            auto t = tokens[i];
            if (t != token::END) {
                auto start_pos = positions[i];
                auto end_pos = positions[i + 1];
                auto diff = end_pos - start_pos;
                auto s = (char*)malloc(diff + 1);
                memcpy(s, str + start_pos, diff);
                s[diff] = '\0';
                printf("token %d, %u -> %u: %s\n", t, start_pos, end_pos, s);
                free(s);
            }
        }
    }

private:
    inline bool is_not_at_eof() {
        return pos < file_size;
    }

    void advance_pos(const char** data, u8 n) {
        *data += n;
        pos += n;
    }

    inline Token simple_token(const char **data, Token t) {
        advance_pos(data, 1);
        return t;
    }

    Token parse_next(const char** data) {
        printf("next %s\n", *data); // TODO rm
        auto str = *data;
        auto c = *str;
        /*
        TODO
        NEQ = 5,
        LT = 6,
        LTE = 7,
        GT = 8,
        GTE = 9,
        LINE_COMMENT = 15,
        BLOCK_COMMENT_START = 16,
        BLOCK_COMMENT_END = 17,
        NUMBER_LITERAL = 26,
        STRING_LITERAL = 27,
        NEGATION,

        more "reserved keywords":
        INPUT = 20,
        OUTPUT = 21,
        */
        switch (c) {
        case '@':
            return parse_keyword(data);
        case '_':
            return simple_token(data, token::WILDCARD);
        case '?':
            return simple_token(data, token::HOLE);
        case '(':
            return simple_token(data, token::LPAREN);
        case ')':
            return simple_token(data, token::RPAREN);
        case '+':
            return simple_token(data, token::PLUS);
        case '-':
            return simple_token(data, token::MINUS);
        case '*':
            return simple_token(data, token::MUL);
        case '/':
            return simple_token(data, token::DIVIDE);
        case '=':
            return simple_token(data, token::EQ);
        case '.':
            return simple_token(data, token::PERIOD);
        case ',':
            return simple_token(data, token::COMMA);
        case ':':
            if (*(str + 1) == '-') {
                advance_pos(data, 2);
                return token::ENTAILS;
            }

            advance_pos(data, 1);
            return token::COLON;
        default:
            // TODO refactor to another helper function
            if (isalpha(c)) {
                // Could be a type or identifier
                if (c == 'u' || c == 's') {  // u32 or string, possibly
                    return parse_type_or_identifier(data, c);
                }

                advance_pos(data, 1);
                return parse_rest_of_identifier(data);
            }

            return parse_error(data);
        }
    }

    Token parse_type_or_identifier(const char **data, char start) {
        // TODO refactor code
        auto str = *data;
        switch (start) {
            case 'u':
                if (*(str + 1) == '3' && *(str + 2) == '2') {
                    advance_pos(data, 3);

                    if (parse_rest_of_identifier(data) == token::IDENTIFIER) {
                        return token::IDENTIFIER;
                    }

                    // if next char is not part of an identifier, we found a u32 type
                    return token::U32_TYPE;
                }
                return parse_rest_of_identifier(data);
            case 's':
                if (memcmp (str + 1, "tring", 5) == 0) {
                    advance_pos(data, 6);

                    if (parse_rest_of_identifier(data) == token::IDENTIFIER) {
                        return token::IDENTIFIER;
                    }

                    // if next char is not part of an identifier, we found a u32 type
                    return token::STRING_TYPE;
                }
                return parse_rest_of_identifier(data);
            default:
                return parse_rest_of_identifier(data);
        }
    }

    Token parse_rest_of_identifier(const char** data) {
        auto str = *data;
        while (is_not_at_eof()) {
            auto c = *str;
            if (isalnum(c) || c == '_') {
                str++;
                pos++;
            } else if (c == '(' || c == ',' || c == ')') {
                // TODO improve check (for arithmetic, ...)
                // TODO fix off by 1 error
                break;
            } else {
                return parse_error(data);
            }
        }

        *data = str;
        return token::IDENTIFIER;
    }

    Token parse_keyword(const char **data) {
        auto str = *data;
        if (memcmp(str, "@def ", 5) == 0) {
            advance_pos(data, 5);
            return token::DEF;
        }

        if (memcmp(str, "@extern ", 8) == 0) {
            advance_pos(data, 8);
            return token::EXTERN;
        }

        return parse_error(data);
    }

    void parse_whitespace(const char **data) {
        auto str = *data;
        while (is_not_at_eof()) {
            if (isspace(*str)) {
                str++;
                pos++;
                continue;
            }

            break;
        }

        *data = str;
    }

    Token parse_error(const char** data) {
        // TODO use strchr? (following code is broken)
        // auto str = *data;
        // auto dot = strchr(str, '.');
        // if (dot != nullptr) {
        //     auto diff = (dot - str) + 1;
        //     pos += diff;
        //     *data = dot;
        // } else {
        //     auto diff = file_size - pos;
        //     *data += diff;
        //     pos = file_size;
        // }
        //
        // return token::ERROR;

        // TODO parse till whitespace, rm code above
        auto str = *data;
        while (is_not_at_eof() && !isspace(*str)) {
            str++;
            pos++;
        }
        // consume the found char also
        *data = str + 1;
        pos++;

        return token::ERROR;
    }

    std::vector<Token> tokens;
    std::vector<u32> positions;  // only keep track of start.. TODO change to spans, to avoid problems with whitespace
    u32 pos;
    u32 file_size;
};

// TODO create high-level api to access parser data

// TODO generate 50MB eclair file as benchmark

int main(int argc, char **argv) {
    Parser p;
    p.parse("./path.eclair");
    p.print_tokens();
    return 0;
}
