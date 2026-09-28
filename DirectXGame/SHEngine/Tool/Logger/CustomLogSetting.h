#pragma once
#include <spdlog/fmt/fmt.h>
#include <Utility/DataStructures.h>

template <>
struct fmt::formatter<Vector2> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    // 実際の出力処理
    template <typename FormatContext>
    auto format(const Vector2& v, FormatContext& ctx) const {
        return fmt::format_to(
            ctx.out(),
            "({}, {})",
            v.x, v.y
        );
    }
};

template <>
struct fmt::formatter<Vector3> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    // 実際の出力処理
    template <typename FormatContext>
    auto format(const Vector3& v, FormatContext& ctx) const {
        return fmt::format_to(
            ctx.out(),
            "({}, {}, {})",
            v.x, v.y, v.z
        );
    }
};

template <>
struct fmt::formatter<Vector4> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    // 実際の出力処理
    template <typename FormatContext>
    auto format(const Vector4& v, FormatContext& ctx) const {
        return fmt::format_to(
            ctx.out(),
            "({}, {}, {}, {})",
            v.x, v.y, v.z, v.w
        );
    }
};

template <>
struct fmt::formatter<Matrix3x3> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    // 実際の出力処理
    template <typename FormatContext>
    auto format(const Matrix3x3& mat, FormatContext& ctx) const {
        return fmt::format_to(
            ctx.out(),
            "[{}, {}, {}]\n[{}, {}, {}]\n[{}, {}, {}]\n",
            mat.m[0][0], mat.m[0][1], mat.m[0][2],
			mat.m[1][0], mat.m[1][1], mat.m[1][2],
			mat.m[2][0], mat.m[2][1], mat.m[2][2]
        );
    }
};

template <>
struct fmt::formatter<Matrix4x4> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    // 実際の出力処理
    template <typename FormatContext>
    auto format(const Matrix4x4& mat, FormatContext& ctx) const {
        return fmt::format_to(
            ctx.out(),
            "[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n[{}, {}, {}, {}]\n",
            mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
			mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
			mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
			mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]
        );
    }
};
