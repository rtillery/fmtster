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

#define LOGENABLE

#ifdef LOGENABLE
#include <iostream>

#define LOGPREAMBLE \
std::cout << "(" << __LINE__ << ") " << __func__ << "()"

#define LOGMID(...) \
std::cout << fmt::format(__VA_ARGS__)

#define LOGBEGIN(...) \
LOGPREAMBLE;          \
LOGMID(__VA_ARGS__)

#define LOGEND(...) \
LOGMID(__VA_ARGS__) << endl;

#define LOG(...)   \
LOGPREAMBLE;       \
LOGMID(": ");      \
LOGEND(__VA_ARGS__)

#define LOGENTRY  \
LOGPREAMBLE;      \
LOGEND(" entry")

#define LOGEXIT   \
LOGPREAMBLE;      \
LOGEND(" exit")

#else

#define LOGMID(...)
#define LOG(...)
#define LOGENTRY
#define LOGEND

#endif // !LOGENABLE

#define FMTSTER_VERSION 000400 // 0.4.0

#include <algorithm>
#include <cstdint>
#include <fmt/core.h>
#include <fmt/format.h>
#include <regex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

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
using std::vector;

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
struct has_ ## FN                                                              \
    : decltype(test_ ## FN<T, Args...>(0))                                     \
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
struct is_string
    : false_type
{};
template<class T, class Traits, class Alloc>
struct is_string<std::basic_string<T, Traits, Alloc>, void>
    : true_type
{};
template<class T, template<typename, typename, typename> class STRING>
struct is_string<T, STRING<T, std::char_traits<T>, std::allocator<T> > >
    : true_type
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
struct is_pair
    : false_type
{};
template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2> >
    : true_type
{};
template<typename ...Ts>
inline constexpr bool is_pair_v = is_pair<Ts...>::value;

// specific detection for std::tuple<>
template<typename T>
struct is_tuple
    : false_type
{};
template<typename... Ts>
struct is_tuple<std::tuple<Ts...> >
    : true_type
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

template<typename T>
T EscapeJSONString(const T& val)
{
    return val;
}
// escape string the JSON way
template<>
string EscapeJSONString(const string& str)
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
} // EscapeJSONString()

template<typename T>
T FormatToValue(const char* sz)
{
    T format = -1;
    if (sz)
    {
        const auto c0 = *sz;
        if ((c0 == '0') || (c0 == 'j') || (c0 == 'J'))
            format = 0;
        else
            throw fmt::format_error(F("unsupported format parameter: \"{}\"",
                                      EscapeJSONString(string(sz))));
    }
    return format;
}
template<typename T>
T FormatToValue(const string& str)
{
    return FormatToValue<T>(str.c_str());
}
template<typename T>
T FormatToValue(int i)
{
    if (i != 0)
        throw fmt::format_error(F("unsupported format parameter: \"{}\"",
                                  i));
    return i;
}

template<typename T>
T ToValue(const char* sz)
{
    T val = 0;

    if (sz)
    {
        do
        {
            const char c = *sz;
            if ((c > '9') || (c < '0'))
            {
                val = 0;
                break;
            }
            val = (val * 10) + (c - '0');
        } while (*(++sz));
    }

    return val;
}
template<typename T>
T ToValue(const string& str)
{
    return ToValue<T>(str.c_str());
}

} // namespace internal

// JSON style class
class JSONStyle
{
public:
    string tab = "  ";         // expanded tab

    enum STYLEDEFS
    {
        BLANK               = 0x0,  // gap
        STYLE = BLANK,              // empty arrays/objs,
                                    // single value arrays/objs
        reserved_1          = 0x1,  // gap
        NOSPACE = reserved_1,       // empty arrays/objs
        SAMELINE = reserved_1,      // single value arrays/objs
        SPACE               = 0x2,  // gap
        // SPACE = SPACE            // empty arrays/objs
        SPACEx2             = 0x3,  // gap
        reserved_4          = 0x4,  // gap
        reserved_5          = 0x5,  // gap
        TAB                 = 0x6,  // gap
        TABx2               = 0x7,  // gap
        NEWLINE             = 0x8,  // gap
        reserved_9          = 0x9,  // gap
        NEWLINE_SPACE       = 0xA,  // gap
        NEWLINE_SPACEx2     = 0xB,  // gap
        reserved_C          = 0xC,  // gap
        reserved_D          = 0xD,  // gap
        NEWLINE_TAB         = 0xE,  // gap
        NEWLINE_TABx2       = 0xF   // gap
    };

    struct JSONStyleConfig
    {
        bool cr : 1;
        bool lf : 1;

        bool hardTab : 1;
        unsigned int tabCount : 4;

        // [ <gap A> value, <gap B> value <gap C> ]
        unsigned int gapA : 4;
        unsigned int gapB : 4;
        unsigned int gapC : 4;

        // { <gap 1> "string" <gap 2> : <gap 3> value, <gap 4> "string" <gap 5> : <gap 6> value <gap 7> }
        unsigned int gap1 : 4;
        unsigned int gap2 : 4;
        unsigned int gap3 : 4;
        unsigned int gap4 : 4;
        unsigned int gap5 : 4;
        unsigned int gap6 : 4;
        unsigned int gap7 : 4;

        unsigned int emptyArray : 2;
        unsigned int emptyObject : 2;
        unsigned int sva : 2;
        unsigned int svo : 2;
    };

    union JSONStyleUnion
    {
        JSONStyleConfig vConfig;
        uint64_t vValue;
    };

    static constexpr JSONStyleUnion DEFAULTCONFIG =
    {
        .vConfig =
        {
#ifdef _WIN32
            .cr = true,
            .lf = true,
#elif defined macintosh
            .cr = true,
            .lf = false,
#else
            .cr = false,
            .lf = true,
#endif

            .hardTab = false,
            .tabCount = 2,

            .gapA = NEWLINE_TAB,
            .gapB = SPACE,
            .gapC = SPACE,

            .gap1 = NEWLINE_TAB,
            .gap2 = BLANK,
            .gap3 = SPACE,
            .gap4 = NEWLINE_TAB,
            .gap5 = BLANK,
            .gap6 = SPACE,
            .gap7 = NEWLINE,

            .emptyArray = SPACE,
            .emptyObject = SPACE,
            .sva = SAMELINE,
            .svo = SAMELINE,
        } // .vStyle
    };

    JSONStyleUnion mStyle;

    JSONStyle(uint64_t value = DEFAULTCONFIG.vValue)
    {
        if (value)
            mStyle.vValue = value;
        else
            mStyle.vValue = DEFAULTCONFIG.vValue;
    }

    uint64_t& value()
    {
        return mStyle.vValue;
    }
    const uint64_t& cvalue() const
    {
        return mStyle.vValue;
    }
    JSONStyleConfig& config()
    {
        return mStyle.vConfig;
    }
    const JSONStyleConfig& cconfig() const
    {
        return mStyle.vConfig;
    }

    static_assert(sizeof(uint64_t) == (64 / 8));
    static_assert(sizeof(JSONStyleConfig) <= sizeof(uint64_t));
    static_assert(sizeof(JSONStyleUnion) == sizeof(uint64_t));

//     char (*__kaboom)[sizeof(uint64_t)] = 1;
//     char (*__kaboom)[sizeof(JSONStyleConfig)] = 1;
//     char (*__kaboom)[sizeof(JSONStyleUnion)] = 1;
};

// base class that handles formatting
struct FmtsterBase
{
    static unsigned int sDefaultFormat;
    static fmtster::JSONStyle sDefaultStyle;

    // results of parsing for use in format()
    vector<string> mArgData = { "" };
    vector<int> mNestedArgIndex = { 0 };

    int mFormatSetting = sDefaultFormat;    // format (0 = JSON)
    fmtster::JSONStyle mStyleSetting = sDefaultStyle;

    int mBraIndentSetting = 0;  // beginning number of brace/bracket indents
    int mDataIndentSetting = 1; // beginning number of data indents

    bool mDisableBras = false;
    bool mDumpStyle = false;

    string mBraIndent = "";     // expanded brace/bracket indent
    string mDataIndent = "  ";  // expanded data indent

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
LOGENTRY;

        // generic handling of N comma-separated parms, including recursive braces
auto i = ctx.begin();
LOGBEGIN("");
while (i != ctx.end())
    LOGMID("{}", *(i++));
LOGEND("\"");

        int parmIndex = 0;
        int braces = 1;
        auto it = ctx.begin();
        while (it != ctx.end())
        {
            const auto c = *it;
            if (c == '{')
            {
                mNestedArgIndex[parmIndex] = ctx.next_arg_id();
                braces++;
            }
            else if (c == '}')
            {
                --braces;
                if (!braces)
                    break;
            }
            else if (c == ',')
            {
                parmIndex++;
                mArgData.resize(parmIndex + 1);
                mNestedArgIndex.resize(parmIndex + 1);
            }
            else
            {
                mArgData[parmIndex] += c;
            }
            it++;
        }

LOG("[0]: \"{}\", {}", mArgData[0], mNestedArgIndex[0]);
if (mArgData.size() > 1)
{
    LOG("[1]: \"{}\", {}", mArgData[1], mNestedArgIndex[1]);
    if (mArgData.size() > 2)
    {
        LOG("[2]: \"{}\", {}", mArgData[2], mNestedArgIndex[2]);
        if (mArgData.size() > 3)
            LOG("[3]: \"{}\", {}", mArgData[3], mNestedArgIndex[3]);
    }
}

LOGEXIT;
        return it;
    } // parse()



    // This function must be called by each FmtsterBase child immediately on
    //  entry to the format() function. It completes the updating of the style
    //  object based on arguments provided, if necessary.
    template<typename FormatContext>
    void decipherParms(FormatContext& ctx)
    {
        using namespace fmtster::internal;

        // ensure vectors are large enough for unchecked processing below
        mArgData.resize(4, "");
        mNestedArgIndex.resize(4, 0);



        //
        // format
        //
        const size_t FORMAT_PARM_INDEX = 0;
        if (mNestedArgIndex[FORMAT_PARM_INDEX])
        {
            auto formatArg = ctx.arg(mNestedArgIndex[FORMAT_PARM_INDEX]);
            auto formatSetting = visit_format_arg(
                [](auto value) -> int
                {
                    // This construct is required because at compile time all
                    //  paths are linked, even though that are not allowed at
                    //  run time, and without this, the return value doens't
                    //  match the function return value in some cases, so the
                    //  compile fails.
                    if constexpr (std::is_integral_v<decltype(value)>)
                    {
                        return value;
                    }
                    else
                    {
                        throw fmt::format_error("unsupported nested argument type");
                    }
                },
                formatArg
            );

            mFormatSetting = FormatToValue<decltype(mFormatSetting)>(formatSetting);
LOG("mFormatSetting (from nested arg): {}", mFormatSetting);
        }
        else if(!mArgData[FORMAT_PARM_INDEX].empty())
        {
LOG("mArgData[FORMAT_PARM_INDEX]: {}", mArgData[FORMAT_PARM_INDEX]);
            mFormatSetting = FormatToValue<decltype(mFormatSetting)>(mArgData[FORMAT_PARM_INDEX]);
LOG("mFormatSetting (from direct arg): {}", mFormatSetting);
        }
else
{
LOG("default mFormatSetting: {}", mFormatSetting);
}



        //
        // style
        //
        const size_t STYLE_PARM_INDEX = 1;
        if (mNestedArgIndex[STYLE_PARM_INDEX])
        {
            auto styleArg = ctx.arg(mNestedArgIndex[STYLE_PARM_INDEX]);
            mStyleSetting.value() = visit_format_arg(
                [](auto value) -> uint64_t
                {
                    // This construct is required because at compile time all
                    //  type paths are linked, even though that are not allowed
                    //  at run time, and without this, the return value doens't
                    //  match the function return value in some cases, so the
                    //  compile fails.
                    if constexpr (std::is_integral_v<decltype(value)>)
                    {
                        return value;
                    }
                    else
                    {
                        throw fmt::format_error("unsupported nested argument type");
                    }
                },
                styleArg
            );

LOG("mStyleSetting.config (from nested arg): {},\nmStyleSetting.value: {}", mStyleSetting, mStyleSetting.value());
        }
        else if(!mArgData[STYLE_PARM_INDEX].empty())
        {
LOG("mArgData[STYLE_PARM_INDEX]: {}", mArgData[STYLE_PARM_INDEX]);
            mStyleSetting.value() =
                ToValue<uint64_t>(mArgData[STYLE_PARM_INDEX]);
LOG("mStyleSetting.config (from direct arg): {},\nmStyleSetting.value: {}", mStyleSetting, mStyleSetting.value());
        }
else
{
LOG("mStyleSetting.config (default): {},\nmStyleSetting.value: {}", mStyleSetting, mStyleSetting.value());
}



        //
        // perm call parms
        //
        const size_t PER_CALL_PARM_INDEX = 2;
        string pcpSetting;
        if (mNestedArgIndex[PER_CALL_PARM_INDEX])
        {
            auto pcpArg = ctx.arg(mNestedArgIndex[PER_CALL_PARM_INDEX]);
            pcpSetting = visit_format_arg(
                [](auto value) -> int
                {
                    if constexpr (std::conjunction_v<std::is_same<const char*, decltype(value)>,
                                                     fmtster::internal::is_string<decltype(value)> >)
                    {
                        return value;
                    }
                    else
                    {
                        throw fmt::format_error("unsupported nested argument type");
                    }
                },
                pcpArg);

LOG("pcpSetting (from nested arg): {}", pcpSetting);
        }
        else if (!mArgData[PER_CALL_PARM_INDEX].empty())
        {
LOG("mArgData[PER_CALL_PARM_INDEX]: {}", mArgData[PER_CALL_PARM_INDEX]);
            pcpSetting = mArgData[PER_CALL_PARM_INDEX];
LOG("pcpSetting (from direct arg): {}", pcpSetting);
        }
else
{
LOG("pcpSetting (default): {}", pcpSetting);
}

        // parse those parms
        if (!pcpSetting.empty())
        {
            bool negate = false;
            for (const auto c : pcpSetting)
            {
                switch (c)
                {
                case 'b': mDisableBras = negate; break;
                case 'f': sDefaultFormat = mFormatSetting; break;
                case 's': sDefaultStyle = mStyleSetting; break;
                case '!': mDumpStyle = true; break;
// @@@ Temporary, for compatibility with existing tests before changing
case '1': mDisableBras = true; break;
                default: ;
                }

                negate = (c == '-');
            }
        }


        //
        // indent
        //
        const size_t INDENT_PARM_INDEX = 3;
        if (mNestedArgIndex[INDENT_PARM_INDEX])
        {
            auto indentArg = ctx.arg(mNestedArgIndex[INDENT_PARM_INDEX]);
            auto indentSetting = visit_format_arg(
                [](auto value) -> int
                {
                    if constexpr (std::is_integral_v<decltype(value)>)
                    {
                        return value;
                    }
                    else
                    {
                        throw fmt::format_error("unsupported nested argument type");
                    }
                },
                indentArg);

            mBraIndentSetting = FormatToValue<decltype(mBraIndentSetting)>(indentSetting);
        }
        else if (!mArgData[INDENT_PARM_INDEX].empty())
        {
            mBraIndentSetting = ToValue<decltype(mBraIndentSetting)>(mArgData[INDENT_PARM_INDEX]);
        }

        if (mBraIndentSetting < 0)
            throw fmt::format_error(fmt::format("invalid indent: \"{}\"",
                                                mBraIndentSetting));

        mDataIndentSetting = mDisableBras ? mBraIndentSetting : mBraIndentSetting + 1;
LOG("mBraIndentSetting: {}, mDataIndentSetting: {}", mBraIndentSetting ,mDataIndentSetting);

        mBraIndent.clear();
        auto i = mBraIndentSetting;
        while (i--)
            mBraIndent += mStyleSetting.tab;

        mDataIndent.clear();
        i = mDataIndentSetting;
        while (i--)
            mDataIndent += mStyleSetting.tab;

LOG("decipherParms() exit");
    } // decipherParms()

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
                           mFormatSetting,
                           (mStyleSetting.value() == sDefaultStyle.value()) ? 0 : mStyleSetting.value(),
                           "",
                           mDataIndentSetting,
                           addComma ? "," : "");
    }
}; // struct FmtterBase

extern unsigned int FmtsterBase::sDefaultFormat;
extern JSONStyle FmtsterBase::sDefaultStyle;

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

LOGENTRY;

        auto itC = c.begin();
        while (itC != c.end())
        {
            std::string fmtStr;

            if (!mDisableBras || (itC != c.begin()))
                fmtStr = "\n";

            const auto& val = *itC;

            if (!is_braceable_v<C>)
                fmtStr += mDataIndent;

            itC++;

if (!fmtster::internal::is_fmtsterable_v<decltype(val)>)
    fmtStr += createFormatString(val, "", !is_braceable_v<C>, itC != c.end());
else
{
    fmtStr += "{:{},{},{},{}}";
    if (itC != c.end())
        fmtStr += ",";
}
//             fmtStr += createFormatString(val, "", !is_braceable_v<C>, itC != c.end());

//             itOut = fmt::format_to(itOut, fmtStr, fmtster::internal::EscapeJSONString(val));
LOG("fmtStr: \"{}\"", fmtStr);
// LOG("val: \"{}\"", fmtster::internal::EscapeJSONString(val));
LOG("val type: {}", typeid(val).name());
LOG("mFormatSetting: {}", mFormatSetting);
LOG("mStyleSetting.value(): {}", mStyleSetting.value());
LOG("\"\"");
LOG("mDataIndentSetting: {}", mDataIndentSetting);
itOut = fmt::format_to(itOut,
                       fmtStr,
                       fmtster::internal::EscapeJSONString(val),
                       mFormatSetting,
                       (mStyleSetting.value() == sDefaultStyle.value()) ? 0 : mStyleSetting.value(),
                       "",
                       mDataIndentSetting);
        }

LOGEXIT;
    } // format_loop() (all containers except multimap)

    // templated function inner loop function for multimaps
    template<typename FormatContext, typename C = T>
    std::enable_if_t<fmtster::internal::is_multimappish_v<C> >
        format_loop(const C& c, FCIt_t<FormatContext>& itOut)
    {
        using namespace fmtster::internal;

LOGENTRY;

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
                if (!mDisableBras || (it != c.begin()))
                    fmtStr = "\n";

                const auto& key = it->first;
                fmtStr += createFormatString(key, mDataIndent, false, false);
                fmtStr += " : ";
                itOut = fmt::format_to(itOut, fmtStr, EscapeJSONString(key));

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

LOGEXIT;
    } // format_loop() (multimaps)

    template<typename FormatContext>
    auto format(const T& sc, FormatContext& ctx)
    {
LOGENTRY;

        using namespace fmtster::internal;

decipherParms(ctx);

        // output opening bracket/brace (if enabled)
        auto itOut = mDisableBras ?
                     ctx.out() :
                     fmt::format_to(ctx.out(), is_braceable_v<T> ? "{{" : "[");

        const bool empty = (sc.end() == sc.begin());

        if (empty && !mDisableBras)
            itOut = fmt::format_to(itOut, " ");
        else
            format_loop<FormatContext, T>(sc, itOut);

        // output closing brace
        if (!mDisableBras)
        {
            if (empty)
                itOut = fmt::format_to(itOut, is_braceable_v<T> ? "}}" : "]");
            else
                itOut = fmt::format_to(itOut,
                                       is_braceable_v<T> ? "\n{}}}" : "\n{}]",
                                       mBraIndent);
        }

LOGEXIT;
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
LOGENTRY;

        // get adapter format and forward to internal type
        auto itCtxEnd = std::find(ctx.begin(), ctx.end(), '}');
        mStrFmt = "{" + std::string(ctx.begin(), itCtxEnd) + "}";
        return itCtxEnd;
    } // parse()

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

LOGEXIT;
        return hack::Get(a);
    } // GetAdapterContainer()

    template<typename FormatContext>
    auto format(const A& ac, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), mStrFmt, GetAdapterContainer(ac));
    }
}; // struct fmt::formatter< adapters >

// fmt::formatter<> for std::pair<>
template<typename T1, typename T2>
struct fmt::formatter<std::pair<T1, T2> >
    : fmtster::FmtsterBase
{
    // create format string
    std::string createPairFormatString(const T1& v1, const T2& v2)
    {
        std::string fmtStr;

        if (!mDisableBras)
            fmtStr += "{{\n";

        fmtStr += createFormatString(v1, mDataIndent, false, false);
        fmtStr += " : ";
        fmtStr += createFormatString(v2, "", true, false);

        if (!mDisableBras)
            fmtStr += fmt::format("\n{}}}}}", mBraIndent);

        return fmtStr;
    } // createPairFormatString()

    template<typename FormatContext>
    auto format(const std::pair<T1, T2>& p, FormatContext& ctx)
    {
        using namespace fmtster::internal;
decipherParms(ctx);

        return format_to(ctx.out(),
                         createPairFormatString(p.first, p.second),
                         EscapeJSONString(p.first), EscapeJSONString(p.second));
    }
}; // struct fmt::formatter<std::pair<> >

// fmt::formatter<> for std::tuple<> (wraps group of heterogeneous objects known at compile time)
template<typename... Ts>
struct fmt::formatter<std::tuple<Ts...> >
    : fmtster::FmtsterBase
{
    template<typename FormatContext>
    auto format(const std::tuple<Ts...>& tup, FormatContext& ctx)
    {
LOGENTRY;

        using namespace fmtster::internal;

decipherParms(ctx);

        // output opening bracket (if enabled)
        auto itOut = mDisableBras ?
                     ctx.out() :
                     fmt::format_to(ctx.out(), "{{");
        auto count = sizeof...(Ts);

        const bool empty = !count;
        if (empty && !mDisableBras)
        {
            itOut = fmt::format_to(itOut, " ");
        }
        else
        {
            ForEachElement(tup,
                           [&](const auto& elem)
                           {
                               std::string fmtStr;
                               if (!mDisableBras || (count != sizeof...(Ts)))
                                   fmtStr = "\n";

                               fmtStr += createFormatString(elem,
                                                            mDataIndent,
                                                            false,
                                                            --count);
                               itOut = fmt::format_to(itOut, fmtStr, elem);
                           });

            // output closing brace
            if (!mDisableBras)
            {
                if (empty)
                    itOut = fmt::format_to(itOut, "}");
                else
                    itOut = fmt::format_to(itOut, "\n{}}}", mBraIndent);
            }
        }

LOGEXIT;
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
LOGENTRY;

        // @@@ Make this conditional (difficult because it differs from the
        //     behavior of all the other types).
        fmtster::FmtsterBase::sDefaultStyle = style;

        auto it = ctx.out();

        if (mDumpStyle)
        {
            using namespace std::string_literals;
            using std::make_pair;

           if (!mDisableBras)
                it = format_to(it, "{{\n");

            const auto config = style.cconfig();
//             const auto tup = std::make_tuple(
//                 make_pair("cr", config.cr),
//                 make_pair("lf", config.lf),
//                 make_pair("hardTab", config.hardTab),
//                 make_pair("tabCount", config.tabCount),
//                 make_pair("gapA", config.gapA),
//                 make_pair("gapB", config.gapB),
//                 make_pair("gapC", config.gapC),
//                 make_pair("gap1", config.gap1),
//                 make_pair("gap2", config.gap2),
//                 make_pair("gap3", config.gap3),
//                 make_pair("gap4", config.gap4),
//                 make_pair("gap5", config.gap5),
//                 make_pair("gap6", config.gap6),
//                 make_pair("gap7", config.gap7),
//                 make_pair("emptyArray", config.emptyArray),
//                 make_pair("emptyObject", config.emptyObject),
//                 make_pair("sva", config.sva),
//                 make_pair("svo", config.svo)
//             );
//             it = format_to(it, createFormatString(tup, mDataIndent, false, false), tup);
it = format_to(it, "  \"cr\" : {},", config.cr);
it = format_to(it, "  \"lf\" : {},", config.lf);
it = format_to(it, "  \"hardTab\" : {},", config.hardTab);
it = format_to(it, "  \"tabCount\" : {},", config.tabCount);
it = format_to(it, "  \"gapA\" : {:04b},", config.gapA);
it = format_to(it, "  \"gapB\" : {:04b},", config.gapB);
it = format_to(it, "  \"gapC\" : {:04b},", config.gapC);
it = format_to(it, "  \"gap1\" : {:04b},", config.gap1);
it = format_to(it, "  \"gap2\" : {:04b},", config.gap2);
it = format_to(it, "  \"gap3\" : {:04b},", config.gap3);
it = format_to(it, "  \"gap4\" : {:04b},", config.gap4);
it = format_to(it, "  \"gap5\" : {:04b},", config.gap5);
it = format_to(it, "  \"gap6\" : {:04b},", config.gap6);
it = format_to(it, "  \"gap7\" : {:04b},", config.gap7);
it = format_to(it, "  \"emptyArray\" : {:02b},", config.emptyArray);
it = format_to(it, "  \"emptyObject\" : {:02b},", config.emptyObject);
it = format_to(it, "  \"sva\" : {:02b},", config.sva);
it = format_to(it, "  \"svo\" : {:02b}", config.svo);

            if (!mDisableBras)
                it = format_to(it, "\n{}}}", mBraIndent);
        }

        return it;
    }
};
