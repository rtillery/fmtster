#pragma once

/* Copyright (c) 2021 Harman International Industries, Incorporated.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define FMTSTER_VERSION 000400 // 0.4.0

#include <algorithm>
#include <fmt/core.h>
#include <fmt/format.h>
#include <regex>
#include <string>
#include <type_traits>
#include <utility>

namespace fmtster
{
using std::find;
using std::string;
using std::stoi;
using std::declval;
using std::void_t;
using std::enable_if_t;
using std::true_type;
using std::false_type;
using std::conjunction_v;
using std::disjunction_v;
using std::negation;

// short helper alias for fmt::format() used by adding "using fmtster::F;" to
//  client code
template<typename ...Args>
string F(std::string_view fmt, const Args&... args)
{
    return fmt::format(fmt, args...);
}

struct FmtsterBase; // forward declaration

namespace internal
{

template<typename>
struct fmtster_true
    : true_type
{};

//
// macro to create has_FN<> templates (also creates has_FN_v<> helper)
//
#define fmtster_MAKEHASFN(FN)                                                  \
template<typename T, typename... Args>                                         \
static auto test_ ## FN(int)                                                   \
    -> fmtster_true<decltype(declval<T>().FN(declval<Args>()...))>;            \
                                                                               \
template<typename, typename...>                                                \
static auto test_ ## FN(long) -> false_type;                                   \
                                                                               \
template<typename T, typename... Args>                                         \
struct has_ ## FN : decltype(test_ ## FN<T, Args...>(0))                       \
{};                                                                            \
                                                                               \
template<typename ...Ts>                                                       \
inline constexpr bool has_ ## FN ## _v = has_ ## FN<Ts...>::value

//
// macro to create has_TYPE<> templates (also creates has_TYPE_v<> helper)
//
#define fmtster_MAKEHASTYPE(TYPE)                                              \
template<typename T, typename = void>                                          \
struct has_ ## TYPE                                                            \
    : false_type                                                               \
{};                                                                            \
                                                                               \
template<typename T>                                                           \
struct has_ ## TYPE<T, void_t<typename T::TYPE> >                              \
    : true_type                                                                \
{};                                                                            \
                                                                               \
template<typename ...Ts>                                                       \
inline constexpr bool has_ ## TYPE ## _v = has_ ## TYPE<Ts...>::value

//
// macro to create is_ID<> templates (also creates is_ID_v<> helper)
//   COND provides the conditional value used to check traits
//
// NOTE 1: Any spaces in the CONDition argument are misinterpreted by the
//         macro parser, so use of parentheses around this argument is
//         recommended.
// NOTE 2: CONDition _can_ be a logical grouping of xxx_v<> values, but it
//         appears that all of these are evaluated, making some combinations
//         fail before the SFINAE can kick in. std::conjunction_v<> &
//         std::disjunction_v<> seem to use lazy evaluation, so use of them
//         is preferred.
#define fmtster_MAKEIS(ID, COND)                                               \
template<typename T, typename = void>                                          \
struct is_ ## ID                                                               \
    : false_type                                                               \
{};                                                                            \
                                                                               \
template<typename T>                                                           \
struct is_ ## ID<T, enable_if_t<COND> >                                        \
    : true_type                                                                \
{};                                                                            \
                                                                               \
template<typename ...Ts>                                                       \
inline constexpr bool is_ ## ID ## _v = is_ ## ID<Ts...>::value

//
// has_TYPE<> declarations
//
fmtster_MAKEHASTYPE(const_iterator);
fmtster_MAKEHASTYPE(key_type);
fmtster_MAKEHASTYPE(mapped_type);
fmtster_MAKEHASTYPE(container_type);

//
// has_FN<> declarations
//
fmtster_MAKEHASFN(begin);
fmtster_MAKEHASFN(end);
fmtster_MAKEHASFN(at);

// functional equivalent for fmtster_MAKEHASFN(operator[])
template<typename T, typename U = void>
struct has_operator_index
    : false_type
{};
template<typename T>
struct has_operator_index<
    T,
    void_t<decltype(declval<T&>()[declval<const typename T::key_type&>()])> >
    : true_type
{};

//
// is_ID<> declarations
//
template<typename T, typename = void>
struct is_string : false_type
{};
template<class T, class Traits, class Alloc>
struct is_string<std::basic_string<T, Traits, Alloc>, void> : true_type
{};
template<class T, template<typename, typename, typename> class STRING>
struct is_string<T, STRING<T, std::char_traits<T>, std::allocator<T> > > : true_type
{};
template<typename T>
inline constexpr bool is_string_v = is_string<T>::value;

fmtster_MAKEIS(container,
               (conjunction_v<has_const_iterator<T>, has_begin<T>, has_end<T> >));
// overriding specialization for std::string, which is not considered a container by fmtster
template<>
struct is_container<string>
    : false_type
{};

fmtster_MAKEIS(mappish, (conjunction_v<has_key_type<T>,
                                       has_mapped_type<T>,
                                       has_operator_index<T> >));
fmtster_MAKEIS(multimappish,
               (conjunction_v<has_key_type<T>,
                              has_mapped_type<T>,
                              negation<has_at<T, typename T::key_type> > >));
fmtster_MAKEIS(adapter, has_container_type_v<T>);

// specific detection for std::pair<> only
template<typename T>
struct is_pair : false_type
{};
template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2> > : true_type
{};
template<typename ...Ts>
inline constexpr bool is_pair_v = is_pair<Ts...>::value;

// specific detection for std::tuple<>
template<typename T>
struct is_tuple : false_type
{};
template<typename... Ts>
struct is_tuple<std::tuple<Ts...> > : true_type
{};
template<typename... Ts>
inline constexpr bool is_tuple_v = is_tuple<Ts...>::value;

// tools for use below
fmtster_MAKEIS(fmtsterable,
               (std::is_base_of_v<fmtster::FmtsterBase, fmt::formatter<T> >));
fmtster_MAKEIS(braceable, (disjunction_v<is_mappish<T>,
                                         is_multimappish<T>,
                                         is_pair<T>,
                                         is_tuple<T> >));

//
// misc helpers
//
// template argument iteration for std::tuple<>
template<typename F, typename... Ts, std::size_t... Is>
void ForEachElement(const std::tuple<Ts...>& tup,
                    F fn,
                    std::index_sequence<Is...>)
{
    (void)(int[]) { 0, ((void)fn(std::get<Is>(tup)), 0)... };
}
template<typename F, typename...Ts>
void ForEachElement(const std::tuple<Ts...>& tup, F fn)
{
    ForEachElement(tup, fn, std::make_index_sequence<sizeof...(Ts)>());
}

} // namespace internal

// JSON style structure for re-use
struct JSONStyle
{
    string tab = "  ";         // expanded tab
};

// base class that handles formatting
struct FmtsterBase
{
    static uint sDefaultFormat;
    static fmtster::JSONStyle sDefaultStyle;

    fmtster::JSONStyle style;

int styleIndex = -1;

bool setDefaultStyle = false;

    int formatSetting = 0;     // format (0 = JSON)

    int braIndentSetting = 0;  // beginning number of brace/bracket indents
    int dataIndentSetting = 1; // beginning number of data indents

    bool disableBraces = false;
    bool dumpStyle = false;

    string braIndent = "";     // expanded brace/bracket indent
    string dataIndent = "  ";  // expanded data indent

    template<typename T>
    T escapeValue(const T& val)
    {
        return val;
    }
    // escape string the JSON way
    string escapeValue(const string& str)
    {
        string out;
        out.reserve(str.length() * 6);
        for (const char c : str)
        {
            if ((c <= '\x1F') || (c >= '\x7F'))
            {
                switch (c)
                {
                case '\b': out += R"(\b)"; break;
                case '\f': out += R"(\f)"; break;
                case '\n': out += R"(\n)"; break;
                case '\r': out += R"(\r)"; break;
                case '\t': out += R"(\t)"; break;
                default:
                    out += F(R"(\u{:04X})", (unsigned int)((unsigned char)c));
                }
            }
            else
            {
                switch (c)
                {
                case '\\':   out += R"(\\)"; break;
                case '\"':   out += R"(\")"; break;
                case '/':    out += R"(\/)"; break;
                case '\x7F': out += R"(\u007F)"; break;
                default:     out += c;
                }
            }
        }

        return out;
    } // escapeValue()

    // Parses the format in the format {<format>,<style>,<tab>,<indent>}.
    // > format (default is 0: JSON):
    //     0 = JSON
    // >     style (default is 0):
    //         0 = open brace/bracket on same line as key, each property on new line
    //         1 = same as 0 with no open/close braces/brackets
    // > tab (default is 2 spaces):
    //     positive integers: spaces
    //     0: no tab
    //     negative integers: hard tabs
    // > indent (default is 0):
    //     positive integers: number of <tab>s to start the indent
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        using fmtster::F;

        auto itCtxEnd = find(ctx.begin(), ctx.end(), '}');
        const string strFmt(ctx.begin(), itCtxEnd);

        std::regex rx("([^,]*),?([^,]*),?([^,]*),?([^,]*)");
        std::smatch sm;
        std::regex_search(strFmt, sm, rx);

        //
        // Format
        //   0, j*, J* = JSON
        //
        const auto strFormat = sm[1].str();
        if (strFormat.empty())
        {
            formatSetting = sDefaultFormat;
        }
        else
        {
            const auto c0 = strFormat[0];
            if ((c0 == '0') || (c0 == 'j') || (c0 == 'J'))
            {
                formatSetting = 0;
            }
            else
            {
                throw fmt::format_error(fmt::format("unsupported format parameter: \"{}\"",
                                                    escapeValue(strFormat)));
            }
        }

        // Style
        //   Format == JSON
        //     JSONStyle
        const auto strStyle = sm[2].str();
        if (!strStyle.empty())
        {
            if (strStyle != "{}")
                throw fmt::format_error(fmt::format("unsupported style parameter: \"{}\"",
                                                    escapeValue(strStyle)));
            else
styleIndex = 1;
        }

        // Per Call Parms
        //   '-' - indicates next parm is negated
        //   'b` - enable surrounding braces/brackets (default is enabled)
        //   'f' - use format provided as default style (default is disabled)
        //   's' - use Style provided as default style (default is disabled)
        //   `!` - print provided or default format and provided or default
        //         style (instead of data) (default is disabled)
        const auto strParms = sm[3].str();
        if (!strParms.empty)
        {
            bool negate = false;
            for (const auto c : strParms)
            {
                switch (c)
                {
                case 'b': disableBraces = negate; break;
                case 'f': sDefaultFormat = formatSetting; break;
                case 's': setDefaultStyle = true; break;
                case '!': dumpStyle = true; break;
                default: ;
                }

                negate = (c == '-');
            }
        }

        // Indent
        //   Baseline number of tab units to indent entire object (default 0)
        auto strIndent = sm[4].str();
        if (!strIndent.empty())
        {
            braIndentSetting = stoi(strIndent);
            if (braIndentSetting < 0)
                throw fmt::format_error(fmt::format("invalid indent: \"{}\"",
                                        braIndentSetting));
        }


// !!!! This can't be done here, because the style hasn't been read yet. But
//      placing it in the format() function means it's not automatically
//      available to all FmtsterBase children without an explicit call.
        braIndent.clear();
        for (int i = 0; i < braIndentSetting; i++)
            braIndent += style.tab;

        dataIndentSetting = braIndentSetting;
        dataIndent = braIndent;

        if (!disableBraces)
        {
            dataIndentSetting++;
            dataIndent += style.tab;
        }

        return itCtxEnd;
    } // parse()

    // templated function to provide non-container {fmt} string
    template<typename T, typename = void>
    std::enable_if_t<std::negation_v<internal::is_fmtsterable<T> >, std::string>
        createFormatString(const T& val,
                           const string& indent,
                           bool /* not used */,
                           bool addComma)
    {
        return indent + (addComma ? "{}," : "{}");
    }

    // templated function to provided {fmt} string and add quotes to strings
    string createFormatString(const string& val,
                              const string& indent,
                              bool /* not used */,
                              bool addComma)
    {
        return indent + (addComma ? "\"{}\"," : "\"{}\"");
    }

    // templated function to provide container (supported by fmster) {fmt} string
    template<typename T>
    std::enable_if_t<internal::is_fmtsterable_v<T>, std::string>
        createFormatString(const T&,
                           const string&,
                           bool addBraces,
                           bool addComma)
    {
        return fmt::format("{{:{},{},{},{}}}{}",
                           formatSetting,
!addBraces ? 1 : 0,
sStyle.tab.size(),
                           dataIndentSetting,
                           addComma ? "," : "");
    }
}; // struct FmtterBase

extern uint FmtsterBase::sFormat;
extern JSONStyle FmtsterBase::sStyle;

} // namespace fmtster

// fmt::formatter<> used for all containers
template<typename T, typename Char>
struct fmt::formatter<T,
                      Char,
                      std::enable_if_t<fmtster::internal::is_container_v<T> > >
    : fmtster::FmtsterBase
{
    template<typename FormatContext>
    using FCIt_t = decltype(std::declval<FormatContext>().out());

    // templated function inner loop function for all containers except multimaps
    template<typename FormatContext, typename C = T>
    std::enable_if_t<std::negation_v<fmtster::internal::is_multimappish<C> > >
        format_loop(const C& c, FCIt_t<FormatContext>& itOut)
    {
        using namespace fmtster::internal;

        auto itC = c.begin();
        while (itC != c.end())
        {
            std::string fmtStr;
            if (!disableBraces || (itC != c.begin()))
                fmtStr = "\n";

            const auto& val = *itC;

            if (!is_braceable_v<C>)
                fmtStr += dataIndent;

            itC++;

            fmtStr += createFormatString(val, "", !is_braceable_v<C>, itC != c.end());

            itOut = fmt::format_to(itOut, fmtStr, escapeValue(val));
        }
    }

    // templated function inner loop function for multimaps
    template<typename FormatContext, typename C = T>
    std::enable_if_t<fmtster::internal::is_multimappish_v<C> >
        format_loop(const C& c, FCIt_t<FormatContext>& itOut)
    {
        auto remainingElements = c.size();
        if (c.empty())
        {
            itOut = fmt::format_to(itOut, " ");
        }
        else
        {
            auto it = c.begin();
            while (it != c.end())
            {
                std::string fmtStr;
                if (!disableBraces || (it != c.begin()))
                    fmtStr = "\n";

                const auto& key = it->first;
                fmtStr += createFormatString(key, dataIndent, false, false);
                fmtStr += " : ";
                itOut = fmt::format_to(itOut, fmtStr, escapeValue(key));

                std::vector<typename C::mapped_type> vals;
                do
                {
                    vals.insert(vals.begin(), it->second);
                    it++;
                } while ((it != c.end()) && (it->first == key));

                itOut = fmt::format_to(itOut,
                                       createFormatString(vals, "", true, it != c.end()),
                                       vals);
            }
        }
    }

    template<typename FormatContext>
    auto format(const T& sc, FormatContext& ctx)
    {
        using namespace fmtster::internal;

        // output opening bracket/brace (if enabled)
        auto itOut = disableBraces ?
                     ctx.out() :
                     fmt::format_to(ctx.out(), is_braceable_v<T> ? "{{" : "[");

        const bool empty = (sc.end() == sc.begin());

        if (empty && !disableBraces)
            itOut = fmt::format_to(itOut, " ");
        else
            format_loop<FormatContext, T>(sc, itOut);

        // output closing brace
        if (!disableBraces)
        {
            if (empty)
                itOut = fmt::format_to(itOut, is_braceable_v<T> ? "}}" : "]");
            else
                itOut = fmt::format_to(itOut,
                                       is_braceable_v<T> ? "\n{}}}" : "\n{}]",
                                       braIndent);
        }

        return itOut;
    }
}; // struct fmt::formatter< containers >

// fmt::formatter<> for adapters (wraps containers & removes some functions)
template<typename A, typename Char>
struct fmt::formatter<A,
                      Char,
                      std::enable_if_t<fmtster::internal::is_adapter<A>::value> >
{
    std::string mStrFmt;

    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        // get adapter format and forward to internal type
        auto itCtxEnd = std::find(ctx.begin(), ctx.end(), '}');
        mStrFmt = "{" + std::string(ctx.begin(), itCtxEnd) + "}";
        return itCtxEnd;
    }

    template <class ADAPTER>
    static typename ADAPTER::container_type const& GetAdapterContainer(ADAPTER &a)
    {
        struct hack : public ADAPTER
        {
            static typename ADAPTER::container_type const& Get(ADAPTER &a)
            {
                return a.*&hack::c;
            }
        };
        return hack::Get(a);
    }

    template<typename FormatContext>
    auto format(const A& ac, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), mStrFmt, GetAdapterContainer(ac));
    }
}; // struct fmt::formatter< adapters >

// fmt::formatter<> for std::pair<>
template<typename T1, typename T2>
struct fmt::formatter<std::pair<T1, T2> > : fmtster::FmtsterBase
{
    // create format string
    std::string createPairFormatString(const T1& v1, const T2& v2)
    {
        std::string fmtStr;

        if (!disableBraces)
            fmtStr += "{{\n";

        fmtStr += createFormatString(v1, dataIndent, false, false);
        fmtStr += " : ";
        fmtStr += createFormatString(v2, "", true, false);

        if (!disableBraces)
            fmtStr += fmt::format("\n{}}}}}", braIndent);

        return fmtStr;
    } // createPairFormatString()

    template<typename FormatContext>
    auto format(const std::pair<T1, T2>& p, FormatContext& ctx)
    {
        return format_to(ctx.out(),
                         createPairFormatString(p.first, p.second),
                         escapeValue(p.first), escapeValue(p.second));
    }
}; // struct fmt::formatter<std::pair<> >

// fmt::formatter<> for std::tuple<> (wraps group of heterogeneous objects known at compile time)
template<typename... Ts>
struct fmt::formatter<std::tuple<Ts...> > : fmtster::FmtsterBase
{
    template<typename FormatContext>
    auto format(const std::tuple<Ts...>& tup, FormatContext& ctx)
    {
        using namespace fmtster::internal;

        // output opening bracket (if enabled)
        auto itOut = disableBraces ?
                     ctx.out() :
                     fmt::format_to(ctx.out(), "{{");
        auto count = sizeof...(Ts);

        const bool empty = !count;
        if (empty && !disableBraces)
        {
            itOut = fmt::format_to(itOut, " ");
        }
        else
        {
            ForEachElement(tup,
                           [&](const auto& elem)
                           {
                               std::string fmtStr;
                               if (!disableBraces || (count != sizeof...(Ts)))
                                   fmtStr = "\n";

                               fmtStr += createFormatString(elem,
                                                            dataIndent,
                                                            false,
                                                            --count);
                               itOut = fmt::format_to(itOut, fmtStr, elem);
                           });

            // output closing brace
            if (!disableBraces)
            {
                if (empty)
                    itOut = fmt::format_to(itOut, "}");
                else
                    itOut = fmt::format_to(itOut, "\n{}}}", braIndent);
            }
        }

        return itOut;
    }
}; // struct fmt::formatter<std::tuple<> >

// fmt::formatter<> for fmtster::JSONStyle
template<>
struct fmt::formatter<fmtster::JSONStyle>
    : fmtster::FmtsterBase
{
    template<typename FormatContext>
    auto format(const fmtster::JSONStyle& style, FormatContext& ctx)
    {
        fmtster::FmtsterBase::sStyle = style;

        auto it = ctx.out();

        if (true) // dumpStyle)
        {
            using namespace std::string_literals;
            using std::make_pair;

            if (!disableBraces)
                it = format_to(it, "{{\n");

            const auto pr = make_pair("tab"s, style.tab);
            it = format_to(it, createFormatString(pr, dataIndent, false, false), pr);

            if (!disableBraces)
                it = format_to(it, "\n{}}}", braIndent);

        }

        return it;
    }
};
