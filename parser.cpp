#include "token.h"
#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>

using u32 = uint32_t;

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
        result.reserve(size + 1);

        // NOTE: includes no EOF
        if (::read(handle, result.data(), size) != size) {
            return "";
        }

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
            auto t = parse_next(&data);
            parse_whitespace(&data);
            tokens.push_back(t);
            positions.push_back(current_pos);
        }

        tokens.push_back(token::END);
        positions.push_back(pos);
    }

    void print_tokens() const {
        for (auto i = 0u; i < tokens.size(); i++) {
            printf("token: %d, %u\n", tokens[i], positions[i]);
        }
    }

private:
    bool is_not_at_eof() {
        return pos < file_size;
    }

    void advance_pos(const char** data, u8 n) {
        *data += n;
        pos += n;
    }

    Token parse_next(const char **data) {
        printf("next %s\n", *data); // TODO rm
        auto c = **data;
        switch (c) {
        case '@':
            return parse_keyword(data);
        default:
            return parse_error(data);
        }
    }

    Token parse_keyword(const char **data) {
        auto str = *data;
        if (memcmp(str, "@def", 4) == 0) {
            advance_pos(data, 4);
            return token::DEF;
        }

        if (memcmp(str, "@extern", 7) == 0) {
            advance_pos(data, 7);
            return token::EXTERN;
        }

        return parse_error(data);
    }

    void parse_whitespace(const char **data) {
        auto str = *data;
        bool found_non_whitespace = false;
        while (is_not_at_eof() && !found_non_whitespace) {
            switch (*str) {
            case ' ':
            case '\n':
            case '\r':
                printf("skipping!\n");
                str++;
                pos++;
                break;
            default:
                found_non_whitespace = true;
                break;
            }
        }

        assert((*data != str || !is_not_at_eof()) && "lexer did not fully lex a token");
        *data = str;
    }

    Token parse_error(const char** data) {
        auto str = *data;
        // TODO stop at . OR whitespace?
        while (is_not_at_eof() && *str != '.') {
            str++;
            pos++;
        }
        // consume the found char also
        *data = str + 1;
        pos++;

        return token::ERROR;
    }

    std::vector<Token> tokens;
    std::vector<u32> positions;  // only keep track of start..
    u32 pos;
    u32 file_size;
};

/*
 * TODO
 * whitespace
 * facts
 * rules
 * error recovery
 * api to access parser data
 */

int main(int argc, char **argv) {
    Parser p;
    p.parse("./path.eclair");
    p.print_tokens();
    return 0;
}
