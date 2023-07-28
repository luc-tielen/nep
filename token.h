#pragma once
#include <cstdint>

using u8 = uint8_t;

namespace token {
    using Token = u8;

    static constexpr Token
        PLUS = 0,
        MINUS = 1,
        MUL = 2,
        DIVIDE = 3,
        EQ = 4,
        NEQ = 5,
        LT = 6,
        LTE = 7,
        GT = 8,
        GTE = 9,
        COMMA = 10,
        PERIOD = 11,
        WILDCARD = 12,
        HOLE = 13,
        ENTAILS = 14, // :-
        LINE_COMMENT = 15,
        BLOCK_COMMENT_START = 16,
        BLOCK_COMMENT_END = 17,
        DEF = 18,
        EXTERN = 19,
        INPUT = 20,
        OUTPUT = 21,
        LPAREN = 22,
        RPAREN = 23,
        U32_TYPE = 24,
        STRING_TYPE = 25,
        NUMBER_LITERAL = 26,
        STRING_LITERAL = 27,
        IDENTIFIER = 28,
        END = 0xfe, // EOF
        ERROR = 0xff;
};
