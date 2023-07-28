#include "token.h"
#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <cstdio>

using u32 = uint32_t;

class File {
public:

    File(std::string_view path)
        : handle(fopen(path.data(), "r")) {
        if (handle == nullptr) {
            throw 1; // TODO proper exception handling (in whole file)
        }
    }

    ~File() {
        fclose(handle);
    }

    std::string read() const {
        auto seek_result = fseek(handle, 0, SEEK_END);
        if (seek_result != 0) {
            throw 2;
        }

        auto size = ftell(handle);
        rewind(handle);

        std::string result;
        result.reserve(size);

        if (!fgets(result.data(), size, handle)) {
            return "";
        }
        return result;
    }

private:
    FILE* handle;
};

class Parser {
    using Token = token::Token;

public:
    Parser()
        : tokens()
        , positions()
        , pos(0) {}

    void parse(std::string_view path) {
        File f(path);
        auto contents = f.read();
        auto data = contents.c_str();

        while (true) {
            auto is_eof = parse_token(&data);
            if (is_eof) {
                break;
            }
        }
    }

    void print_tokens() const {
        for (auto i = 0u; i < tokens.size(); i++) {
            printf("token: %d, %u\n", tokens[i], positions[i]);
        }
    }

private:
    // TODO helper functions to parse tokens
    bool parse_token(const char **data) {
        auto current_pos = pos;
        auto t = parse_next(data);
        tokens.push_back(t);
        positions.push_back(pos);
        return t == token::END;
    }

    Token parse_next(const char **data) {
        (*data)++; // TODO improve this..
        return token::END;
    }

    std::vector<Token> tokens;
    std::vector<u32> positions;  // only keep track of start..
    u32 pos;
};

/*
 * TODO
 * whitespace
 * facts
 * rules
 * api to access parser data
 */

int main(int argc, char **argv) {
    Parser p;
    p.parse("./path.eclair");
    p.print_tokens();
    return 0;
}
