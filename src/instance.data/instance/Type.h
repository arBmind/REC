#pragma once
#include "meta/Flags.h"
#include "strings/View.h"

namespace instance {

using Name = strings::String;
using NameView = strings::View;
struct Module;
using ModuleView = const Module*;

enum class TypeFlag {
    Constructor = 1 << 0,
    Destructor = 1 << 1,
};
using TypeFlags = meta::Flags<TypeFlag>;

enum class Parser {
    Expression,
    SingleToken,
    IdTypeValue,
    IdTypeValueTuple,
    OptionalIdTypeValueTuple,
};

struct Type {
    uint64_t size{};
    TypeFlags flags{};
    Parser parser{};
    ModuleView module{};
};
using TypeView = const Type*;

inline auto nameOf(const Type& type) -> NameView { return NameView{"type"}; }

} // namespace instance
