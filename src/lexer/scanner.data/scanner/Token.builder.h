#pragma once
#include "Token.h"

namespace scanner {

using Tokens = std::vector<Token>;

namespace details {

template<class Tok>
struct TokenBuilder {
    static auto build(Tok&& t) -> Token { // token type => token
        return {std::move(t)};
    }
};

template<>
struct TokenBuilder<Token> {
    static auto build(Token&& t) -> Token { return std::move(t); } // full token is kept
};

template<>
struct TokenBuilder<View> {
    static auto build(const View& b) -> Token {
        auto tok = scanner::IdentifierLiteral{};
        tok.input = b;
        return tok;
    }
};

} // namespace details

// Helper to ease create a single token for unit tests
template<class Tok>
auto buildToken(Tok&& t) -> Token {
    return details::TokenBuilder<Tok>::build(std::forward<Tok>(t));
}

// Helper to build a vector of Tokens at once
template<class... Tok>
auto buildTokens(Tok&&... t) -> Tokens {
    return Tokens{scanner::buildToken(std::forward<Tok>(t))...};
}

} // namespace scanner
