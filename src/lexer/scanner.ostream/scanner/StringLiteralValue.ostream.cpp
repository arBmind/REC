#include "StringLiteralValue.ostream.h"

#include <strings/Rope.ostream.h>

namespace scanner {

auto operator<<(std::ostream& out, const StringErrors& errors) -> std::ostream& {
    for (const auto& error : errors) {
        out << "  error: " << error << '\n';
    }
    return out;
}

auto operator<<(std::ostream& out, const StringLiteralValue& lit) -> std::ostream& {
    return out << '"' << lit.text << '"';
}

} // namespace scanner
