#pragma once
#include "strings/CodePoint.h"
#include "strings/String.h"

#include "meta/Optional.h"
#include "meta/algorithm.h"

#include <string>

namespace strings {

struct View;
using OptionalView = meta::Optional<meta::DefaultPacked<View>>;

/// non owning read only view to a utf8 encoded string
struct View {
    using This = View;
    using Byte = uint8_t;
    using It = const Byte *;

private:
    It start_m;
    It end_m;

public:
    constexpr View() noexcept // valid empty view
        : View(nullptr, nullptr) {}

    template<size_t N>
    constexpr explicit View(const char (&str)[N]) noexcept // view a constant string literal
        : View(reinterpret_cast<const Byte *>(str), reinterpret_cast<const Byte *>(str + N - 1)) {}

    constexpr View(const Byte *start, const Byte *end) noexcept // from iterator range
        : start_m(start)
        , end_m(end) {}

    View(const String &s) noexcept // view existing owning string
        : View(s.data(), s.data() + s.byteCount().v) {}

    explicit View(const std::string &s) noexcept // view owning std::string
        : View(reinterpret_cast<const Byte *>(s.data()), reinterpret_cast<const Byte *>(s.data() + s.length())) {}

    // full value semantics
    constexpr View(const This &) noexcept = default;
    constexpr This &operator=(const This &) noexcept = default;
    constexpr View(This &&) noexcept = default;
    constexpr This &operator=(This &&) noexcept = default;

    // enable fast optional
    // equal if view on the same range
    // use content_equals for content comparison
    constexpr bool operator==(const This &o) const { return start_m == o.start_m && end_m == o.end_m; }
    constexpr bool operator!=(const This &o) const { return !(*this == o); }
    bool operator<(const This &o) const {
        auto l = *this;
        auto r = o;
        while (true) {
            auto lcp = l.pullCodePoint().orValue(CodePoint{0});
            if (lcp == 0) return true;
            auto rcp = r.pullCodePoint().orValue(CodePoint{0});
            if (lcp < rcp) continue;
            return false;
        }
    }

    constexpr bool isContentEqual(const This &o) const {
        return byteCount().v == o.byteCount().v && meta::equals(*this, o.begin());
    }

    constexpr auto byteCount() const -> Count { return {static_cast<uint32_t>(end_m - start_m)}; }
    constexpr bool isEmpty() const { return start_m == end_m; }

    /// view of the first N bytes
    template<size_t N>
    constexpr auto front() const -> OptionalView {
        if (byteCount().v >= N) {
            return View(start_m, start_m + N);
        }
        return {};
    }

    bool pullBom() & {
        return front<3>().map([=](This v) {
            if (v.isContentEqual(This("\xEF\xBB\xBF"))) {
                pop<3>();
                return true;
            }
            return false;
        });
    }

    auto pullCodePoint() -> OptionalCodePoint {
        // see https://en.wikipedia.org/wiki/UTF-8
        return pop().map([=](Byte c0) -> OptionalCodePoint {
            if ((c0 & 0x80) != 0x80) return CodePoint{c0};
            if ((c0 & 0xE0) == 0xC0) {
                return pop().map([=](Byte c1) -> OptionalCodePoint {
                    if ((c1 & 0xC0) != 0x80) return {};
                    return CodePoint{((c0 & 0x1Fu) << 6) | ((c1 & 0x3Fu) << 0)};
                });
            }
            if ((c0 & 0xF0) == 0xE0) {
                return pop().map([=](Byte c1) -> OptionalCodePoint {
                    if ((c1 & 0xC0) != 0x80) return {};
                    return pop().map([=](Byte c2) -> OptionalCodePoint {
                        if ((c2 & 0xC0) != 0x80) return {};
                        return CodePoint{((c0 & 0x0Fu) << 12) | ((c1 & 0x3Fu) << 6) | ((c2 & 0x3Fu) << 0)};
                    });
                });
            }
            if ((c0 & 0xF8) == 0xF0) {
                return pop().map([=](Byte c1) -> OptionalCodePoint {
                    if ((c1 & 0xC0) != 0x80) return {};
                    return pop().map([=](Byte c2) -> OptionalCodePoint {
                        if ((c2 & 0xC0) != 0x80) return {};
                        return pop().map([=](Byte c3) -> OptionalCodePoint {
                            if ((c3 & 0xC0) != 0x80) return {};
                            return CodePoint{((c0 & 0x07u) << 18) | ((c1 & 0x3Fu) << 12) | ((c2 & 0x3Fu) << 6) |
                                             ((c3 & 0x3Fu) << 0)};
                        });
                    });
                });
            }
            return {};
        });
    }

    constexpr auto begin() const -> It { return start_m; }
    constexpr auto end() const -> It { return end_m; }

    constexpr auto front() const -> Byte { return *start_m; }

private:
    auto peek() -> meta::Optional<Byte> {
        if (0 == byteCount().v) return {};
        return *start_m;
    }
    auto pop() -> meta::Optional<Byte> {
        if (0 == byteCount().v) return {};
        auto v = *start_m;
        start_m++;
        return v;
    }
    template<size_t N>
    bool pop() {
        if (N > byteCount().v) {
            start_m = end_m;
            return false;
        }
        start_m += N;
        return true;
    }
};

inline auto to_string(const View &v) { return String(v.begin(), v.end()); }

} // namespace strings
