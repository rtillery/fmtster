// g++ -std=c++17 -o fmtmod fmtmod.cpp -lstdc++ -lfmt

#include <fmt/format.h>
#include <iostream>
using std::cout;
using std::endl;
#include <numeric>
#include <string>
using namespace std::string_literals;
#include <typeinfo>
#include <typeindex>

struct S {};
struct A { int val; };
const std::type_index A_TI = std::type_index(typeid(A));

template <>
struct fmt::formatter<S>
{
    int arg_id;

    auto parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it == end)
            return it;
        if (*it == '{')
        {
            // Parse a nested field.
            ++it;
            if (it == end || *it != '}')
                throw format_error("missing '}'");
            ++it;
            arg_id = ctx.next_arg_id();
        }
        return it;
    }

    auto format(S s, format_context& ctx)
    {
        auto arg = ctx.arg(arg_id);
        auto value = visit_format_arg([](auto value) -> int
        {
            if constexpr (std::is_integral_v<decltype(value)>)
            {
                return value;
            }
            else if constexpr (std::is_same_v<decltype(value), typename fmt::basic_format_arg<format_context>::handle>)
            {
                if (value.custom_.ti == A_TI)
                    return ((A*)(value.custom_.value))->val;
                else
                    throw format_error("(2) unsupported nested argument type: "s + typeid(decltype(value)).name());
            }
            else
            {
                throw format_error("(1) unsupported nested argument type: "s + typeid(decltype(value)).name());
            }
        }, arg);
        return format_to(ctx.out(), "[{}]", value);
    }
};

template<>
struct fmt::formatter<A>
{
    template<typename PC>
    constexpr auto parse(PC& ctx)
    {
        return ctx.begin();
    }

    template<typename FC>
    auto format(const A& a, FC& ctx)
    {
        return ctx.out();
    }
};

int main()
{
    cout << fmt::format("{:{}}", S(), 42) << endl;

    cout << fmt::format("{:{}}", S(), A{7}) << endl;
}
