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

#define FMTSTER_VERSION 000500 // 0.5.0

#include <algorithm>
#include <cstdint>
#include <fmt/core.h>
#include <fmt/format.h>
#include <regex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <iostream>
using std::cout;
using std::endl;
using std::boolalpha;

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

struct Base; // forward declaration

namespace internal
{

template<typename>
struct fmtster_true
  : true_type
{};

template<typename T>
using simplify_type = std::remove_cv_t<std::remove_reference_t<T> >;

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
  : decltype(test_ ## FN<T, Args...>(0))                                       \
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
  : false_type                                                                 \
{};                                                                            \
                                                                               \
template<typename T>                                                           \
struct has_ ## TYPE<T, void_t<typename T::TYPE> >                              \
  : true_type                                                                  \
{};                                                                            \
                                                                               \
template<typename ...Ts>                                                       \
inline constexpr bool has_ ## TYPE ## _v = has_ ## TYPE<Ts...>::value

//
// macro to create is_ID<> templates (also creates is_ID_v<> helper)
//   COND provides the conditional value used to check traits
//
// NOTE 1: Any spaces or angle brackets (greater-than or less-than) in the
//         CONDition argument are misinterpreted by the parser, so use of
//         parentheses around this argument is recommended.
// NOTE 2: CONDition _can_ be a logical grouping of xxx_v<> values, but it
//         appears that all of these are evaluated, making some combinations
//         fail before the SFINAE can kick in. std::conjunction_v<> &
//         std::disjunction_v<> seem to use lazy evaluation, so use of them
//         is preferred.
#define fmtster_MAKEIS(ID, COND)                                               \
template<typename T, typename = void>                                          \
struct is_ ## ID                                                               \
  : false_type                                                                 \
{};                                                                            \
                                                                               \
template<typename T>                                                           \
struct is_ ## ID<T, enable_if_t<COND> >                                        \
  : true_type                                                                  \
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
template <typename T>
using is_string = std::is_constructible<std::string, T>;
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
               (std::is_base_of_v<fmtster::Base, fmt::formatter<simplify_type<T> > >));
fmtster_MAKEIS(braceable, (disjunction_v<is_mappish<T>,
                                         is_multimappish<T>,
                                         is_pair<T>,
                                         is_tuple<T> >));

//
// misc helpers
//
// Dummy escape function for all but strings
template<typename T>
T EscapeIfJSONString(const T& val)
{
    return val;
}
// escape string the JSON way
template<>
string EscapeIfJSONString(const string& strIn)
{
    string strOut;
    strOut.reserve(strIn.length() * 6);
    for (const char c : strIn)
    {
        if ((c <= '\x1F') || (c >= '\x7F'))
        {
            switch (c)
            {
            case '\b': strOut += R"(\b)"; break;
            case '\f': strOut += R"(\f)"; break;
            case '\n': strOut += R"(\n)"; break;
            case '\r': strOut += R"(\r)"; break;
            case '\t': strOut += R"(\t)"; break;
            default:
                strOut += F(R"(\u{:04X})", (unsigned int)((unsigned char)c));
            }
        }
        else
        {
            switch (c)
            {
            case '\\':   strOut += R"(\\)"; break;
            case '\"':   strOut += R"(\")"; break;
            case '/':    strOut += R"(\/)"; break;
            case '\x7F': strOut += R"(\u007F)"; break;
            default:     strOut += c;
            }
        }
    }

    return strOut;
} // EscapeIfJSONString()

template<typename T>
T ToValue(const char* sz, const string& throwArg = "")
{
    T val = 0;

    if (sz)
    {
        do
        {
            const char c = *sz;
            if ((c > '9') || (c < '0'))
            {
                if (throwArg.empty())
                {
                    val = 0;
                    break;
                }
                else
                {
                    throw fmt::format_error(F("fmtster: unsupported {} argument: \"{}\"",
                                              throwArg,
                                              sz));
                }
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

int FormatToValue(__int128_t i)
{
    if (i != 0)
        throw fmt::format_error(F("fmtster: unsupported format argument value: {}", i));
    return i;
}
int FormatToValue(const char* const sz)
{
    int format = -1;
    if (sz)
    {
        const auto c0 = *sz;
        if ((c0 == 'j') || (c0 == 'J'))
        {
            format = 0;
        }
        else
        {
            format = ToValue<int>(sz, "format");
        }
    }
    return FormatToValue(format);
}
int FormatToValue(const string& str)
{
    return FormatToValue(str.c_str());
}
int FormatToValue(const fmt::basic_string_view<char>& str)
{
    return FormatToValue(str.data());
}
template<typename T>
int FormatToValue(T i)
{
    return FormatToValue((__int128_t)i);
}

} // namespace internal

//
// enumeration for use in JSONStyle below
//
enum JSS
{
    BLANK               = 0x0,
    reserved_1          = 0x1,
    SPACE               = 0x2,
    SPACEx2             = 0x3,
    reserved_4          = 0x4,
    reserved_5          = 0x5,
    TAB                 = 0x6,
    TABx2               = 0x7,
    NEWLINE             = 0x8,
    reserved_9          = 0x9,
    NEWLINE_SPACE       = 0xA,
    NEWLINE_SPACEx2     = 0xB,
    reserved_C          = 0xC,
    reserved_D          = 0xD,
    NEWLINE_TAB         = 0xE,
    NEWLINE_TABx2       = 0xF
}; // enum JSS

//
// definition of style, reused multiple times below
//
#if false // @@@ disable members that are not implemented

#define JSONSTYLESTRUCT                                                        \
    {                                                                          \
        bool cr : 1;                                                           \
        bool lf : 1;                                                           \
                                                                               \
        bool hardTab : 1;                                                      \
        unsigned int tabCount : 4;                                             \
                                                                               \
        /* [ <gap A> value, <gap B> value <gap C> ] */                         \
        unsigned int gapA : 4;                                                 \
        unsigned int gapB : 4;                                                 \
        unsigned int gapC : 4;                                                 \
                                                                               \
        /* { <gap 1> "string" <gap 2> : <gap 3> value, <gap 4> "string" <gap 5> : <gap 6> value <gap 7> } */ \
        unsigned int gap1 : 4;                                                 \
        unsigned int gap2 : 4;                                                 \
        unsigned int gap3 : 4;                                                 \
        unsigned int gap4 : 4;                                                 \
        unsigned int gap5 : 4;                                                 \
        unsigned int gap6 : 4;                                                 \
        unsigned int gap7 : 4;                                                 \
                                                                               \
        unsigned int emptyArray : 2;                                           \
        unsigned int emptyObject : 2;                                          \
        unsigned int sva : 2;                                                  \
        unsigned int svo : 2;                                                  \
    }

#else

// @@@ all that is implemented currently
#define JSONSTYLESTRUCT                                                        \
    {                                                                          \
        bool hardTab : 1;                                                      \
        unsigned int tabCount : 4;                                             \
    }

#endif // true

namespace internal
{

// used to measure the size of the bitfield-based structure
struct MeasureJSONStyle JSONSTYLESTRUCT;

// If well packed, the style structure will fit into 64 bits, but if not, we
// can use 128 bits.
template<size_t bytes>
using VALUE_TYPE =
    std::conditional_t<(bytes <= 8),
                       uint64_t,
                       std::conditional_t<(bytes > 8) && (bytes <= 16),
                                          __uint128_t,
                                          void>
                      >;
using VALUE_T = VALUE_TYPE<sizeof(MeasureJSONStyle)>;

// used to define the DEFAULTJSONCONFIG before use as default value (format
// must match the bitfield packing, so no hard-wired constant can be used)
union ForwardJSONStyle
{
    struct JSONSTYLESTRUCT;
    VALUE_T value;
};

} // namespace internal

// built-in default, which can be replaced by user-configured default
constexpr internal::ForwardJSONStyle DEFAULTJSONCONFIG =
{
    {
#if false // @@@ disable members that are not implemented

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

#endif // false

        .hardTab = false,
        .tabCount = 2,

#if false // @@@ disable members that are not implemented

        .gapA = JSS::NEWLINE_TAB,
        .gapB = JSS::SPACE,
        .gapC = JSS::SPACE,

        .gap1 = JSS::NEWLINE_TAB,
        .gap2 = JSS::BLANK,
        .gap3 = JSS::SPACE,
        .gap4 = JSS::NEWLINE_TAB,
        .gap5 = JSS::BLANK,
        .gap6 = JSS::SPACE,
        .gap7 = JSS::NEWLINE,

        .emptyArray = JSS::SPACE,
        .emptyObject = JSS::SPACE,
        .sva = JSS::SAMELINE,
        .svo = JSS::SAMELINE

#endif // false

    }
};

// a union to allow access to individual style members as well as treat as
// integer for use with {fmt}
union JSONStyle
{
public:
    struct JSONSTYLESTRUCT;
    internal::VALUE_T value;

    JSONStyle(uint64_t val = DEFAULTJSONCONFIG.value)
      : value(val)
    {}

    operator internal::VALUE_T() const
    {
        return value;
    }
};

namespace internal
{

// wrapper of JSONStyle that expands configuration to strings used in output
class JSONStyleHelper
{
public:
    string tab;  // expanded tab

    JSONStyle mStyle;
    VALUE_T mLastStyleValue;

    JSONStyleHelper(uint64_t value = DEFAULTJSONCONFIG.value)
      : mLastStyleValue(0)
    {
        if (!value)
            value = DEFAULTJSONCONFIG.value;

        mStyle.value = value;
        updateExpansions();
    }

    JSONStyleHelper operator=(uint64_t value)
    {
        mStyle.value = value;
        updateExpansions();
        return *this;
    }

    JSONStyleHelper operator=(const JSONStyle& style)
    {
        mStyle.value = style.value;
        updateExpansions();
        return *this;
    }

    void updateExpansions()
    {
        if (!mLastStyleValue || (mLastStyleValue != mStyle.value))
        {
            tab = mStyle.hardTab
                  ? string(mStyle.tabCount, '\t')
                  : string(mStyle.tabCount, ' ');
            mLastStyleValue = mStyle.value;
        }
    }
};

} // namespace internal

// base class that handles formatting
struct Base
{
protected:
    static constexpr size_t FORMAT_ARG_INDEX = 0;
    static constexpr size_t STYLE_ARG_INDEX = 1;
    static constexpr size_t PER_CALL_ARG_INDEX = 2;
    static constexpr size_t INDENT_ARG_INDEX = 3;

    // results of parsing for use in format()
    vector<string> mArgData;
    vector<unsigned int> mNestedArgIndex;

    // from format arg (int due to code in FormatToValue(const char*))
    int mFormatSetting;

    // from style arg
    internal::JSONStyleHelper mJSONStyleHelper;

    // from indent
    size_t mIndentSetting;

    // from per call parms
    bool mDisableBras;

    string mBraIndent;  // expanded brace/bracket indent
    string mDataIndent; // expanded data indent

    //
    // header-defined static variables to hold user-define defaults
    //
    static int& DefaultFormat()
    {
        static int defaultFormat = 0;
        return defaultFormat;
    };

    static internal::JSONStyleHelper& DefaultJSONStyleHelper()
    {
        static internal::JSONStyleHelper defaultStyleHelper(0);
        return defaultStyleHelper;
    };

public:
    //
    // user access to user-defined defaults
    //
    static int GetDefaultFormat()
    {
        return DefaultFormat();
    }

    static const JSONStyle& GetDefaultJSONStyle()
    {
        return DefaultJSONStyleHelper().mStyle;
    }

    Base()
      : mArgData{ "" },
        mNestedArgIndex{ 0 },
        mFormatSetting(GetDefaultFormat()),
        mJSONStyleHelper(GetDefaultJSONStyle().value),
        mIndentSetting(0),
        mDisableBras(false)
    {}

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
        // generic handling of N comma-separated arguments, including recursive braces

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

        return it;
    } // parse()



    // This function must be called by each Base child immediately on
    //  entry to the format() function. It completes the updating of the style
    //  object based on arguments provided, if necessary.
    template<typename FormatContext>
    void resolveArgs(FormatContext& ctx)
    {
        using namespace std::string_literals;
        using fmt::format_to;
        using namespace fmtster::internal;

        // ensure vectors are large enough for unchecked processing below
        mArgData.resize(4, "");
        mNestedArgIndex.resize(4, 0);



        //
        // format
        //
        if (mNestedArgIndex[FORMAT_ARG_INDEX])
        {
            auto formatArg = ctx.arg(mNestedArgIndex[FORMAT_ARG_INDEX]);
            auto formatSetting = visit_format_arg(
                [](auto value) -> int
                {
                    // This construct is required because at compile time all
                    //  paths are linked, even though that are not allowed at
                    //  run time, and without this, the return value doens't
                    //  match the function return value in some cases, so the
                    //  compile fails.
                    using val_t = simplify_type<decltype(value)>;
                    if constexpr (std::is_integral_v<val_t>)
                    {
                        return FormatToValue(value); // to check for supported formats
                    }
                    else if constexpr (internal::is_string_v<val_t>)
                    {
                        return FormatToValue(value); // to convert formats
                    }
                    else
                    {
                        throw fmt::format_error(F("fmtster: unsupported nested argument type for format: {} (only integers and strings accepted)",
                                                  typeid(value).name()));
                    }
                },
                formatArg
            );

            mFormatSetting = formatSetting;
        }
        else if(!mArgData[FORMAT_ARG_INDEX].empty())
        {
            mFormatSetting = FormatToValue(mArgData[FORMAT_ARG_INDEX]);
        }
        else
        {
            mFormatSetting = GetDefaultFormat();
        }



        //
        // style
        //
        if (mNestedArgIndex[STYLE_ARG_INDEX])
        {
            auto styleArg = ctx.arg(mNestedArgIndex[STYLE_ARG_INDEX]);
            auto styleSetting = visit_format_arg(
                [](auto value) -> uint64_t
                {
                    // This construct is required because at compile time all
                    //  type paths are linked, even though that are not allowed
                    //  at run time, and without this, the return value doens't
                    //  match the function return value in some cases, so the
                    //  compile fails.
                    if constexpr (std::is_integral_v<simplify_type<decltype(value)> >)
                    {
                        return value;
                    }
                    else
                    {
                        throw fmt::format_error(F("fmtster: unsupported nested argument type for style: {} (only integers accepted--pass XXXStyle.value, not XXXStyle)",
                                                  typeid(value).name()));
                    }
                },
                styleArg
            );
            mJSONStyleHelper = internal::JSONStyleHelper(styleSetting);
        }
        else if(!mArgData[STYLE_ARG_INDEX].empty())
        {
            auto styleSetting = ToValue<uint64_t>(mArgData[STYLE_ARG_INDEX]);
            mJSONStyleHelper = internal::JSONStyleHelper(styleSetting);
        }
        else
        {
            mJSONStyleHelper = GetDefaultJSONStyle().value;
        }




        //
        // perm call parms
        //
        string pcpSetting;
        if (mNestedArgIndex[PER_CALL_ARG_INDEX])
        {
            auto pcpArg = ctx.arg(mNestedArgIndex[PER_CALL_ARG_INDEX]);
            pcpSetting = visit_format_arg(
                [](auto value) -> string
                {
                    if constexpr (std::disjunction_v<std::is_same<simplify_type<const char*>, simplify_type<decltype(value)> >,
                                                     fmtster::internal::is_string<simplify_type<decltype(value)> > >)
                    {
                        return value;
                    }
                    else
                    {
                        throw fmt::format_error(F("fmtster: unsupported nested argument type for per call parameters: (only strings accepted)",
                                                  typeid(value).name()));
                    }
                },
                pcpArg);
        }
        else if (!mArgData[PER_CALL_ARG_INDEX].empty())
        {
            pcpSetting = mArgData[PER_CALL_ARG_INDEX];
        }



        //
        // indent
        //
        if (mNestedArgIndex[INDENT_ARG_INDEX])
        {
            auto indentArg = ctx.arg(mNestedArgIndex[INDENT_ARG_INDEX]);
            auto indentSetting = visit_format_arg(
                [](auto value) -> int
                {
                    if constexpr (std::is_integral_v<simplify_type<decltype(value)> >)
                    {
                        return value;
                    }
                    else
                    {
                        throw fmt::format_error(F("fmtster: unsupported nested argument type for indent: (only integers accepted)",
                                                  typeid(value).name()));
                    }
                },
                indentArg);

            mIndentSetting = indentSetting;
        }
        else if (!mArgData[INDENT_ARG_INDEX].empty())
        {
            mIndentSetting =
                ToValue<decltype(mIndentSetting)>(mArgData[INDENT_ARG_INDEX]);
        }
        else
        {
            mIndentSetting = 0;
        }




        mJSONStyleHelper.updateExpansions();

        // expand indenting
        mBraIndent.clear();
        for (auto i = mIndentSetting; i; --i)
            mBraIndent += mJSONStyleHelper.tab;
        mDataIndent = mBraIndent + mJSONStyleHelper.tab;




        // parse those parms
        bool negate = false;
        for (const auto c : pcpSetting)
        {
            switch (c)
            {
            case 'b':
                mDisableBras = negate;
                break;

            case 'f':
                if (!negate)
                    DefaultFormat() = mFormatSetting;
                break;

            case 's':
                if (!negate)
                    DefaultJSONStyleHelper() = mJSONStyleHelper;
                break;

            default:
                ;
            }

            negate = (c == '-');
        }
    } // resolveArgs()
}; // struct FmtterBase

} // namespace fmtster

// fmt::formatter<> used for all containers
template<typename T, typename Char>
struct fmt::formatter<T,
                      Char,
                      std::enable_if_t<fmtster::internal::is_container_v<T> > >
  : fmtster::Base
{
    template<typename FormatContext>
    using FCIt_t = decltype(std::declval<FormatContext>().out());

    // templated function inner loop function for ALL CONTAINERS except
    // multimaps (this can be used for single data per element & map-like
    // containers because map-like containers use a std::pair<> for each
    // element)
    template<typename FormatContext, typename C = T>
    std::enable_if_t<std::negation_v<fmtster::internal::is_multimappish<C> > >
        format_loop(const C& c, FCIt_t<FormatContext>& itFC)
    {
        using namespace fmtster::internal;

        auto itC = c.begin();
        while (itC != c.end())
        {
            const auto indent = mDisableBras ? mBraIndent : mDataIndent;

            std::string fmtStr;

            bool isFirstElement = (itC == c.begin());
            bool newLine = !mDisableBras || !isFirstElement;
            if (newLine)
                fmtStr = "\n";

            // get current element value
            const auto& val = *itC;

            // this is done ahead to determine comma insertion
            itC++;

            bool isLastElement = (itC == c.end());

            // NOTE: decltype(val) SHOULD be able to be used here, instead of
            //       typename C::value_type. But for some reason, while the
            //       verified (printing typeid(val).name() vs typeid(typename
            //       C::valuetype).name()) types are the same, is_fmtsterable<>
            //       can't seem to handle the former.
            if (is_fmtsterable_v<typename C::value_type>)
            {
                if (!is_pair_v<simplify_type<decltype(val)> >)
                    fmtStr += indent;

                fmtStr += isLastElement ? "{:{},{},{},{}}" : "{:{},{},{},{}},";
                auto pcp = is_pair_v<simplify_type<decltype(val)> > ? "-b" : "";
                itFC = fmt::format_to(itFC,
                                      fmtStr,
                                      EscapeIfJSONString(val),
                                      mFormatSetting,
                                      mJSONStyleHelper.mStyle.value,
                                      pcp,
                                      mIndentSetting);
            }
            else
            {
                fmtStr += indent;

                if (fmtster::internal::is_string_v<decltype(val)>)
                    fmtStr += isLastElement ? "\"{}\"" : "\"{}\",";
                else
                    fmtStr += isLastElement ? "{}" : "{},";
                itFC = fmt::format_to(itFC, fmtStr, EscapeIfJSONString(val));
            }
        }
    } // format_loop() (all containers except multimap)

    // templated function inner loop function for multimaps
    template<typename FormatContext, typename C = T>
    std::enable_if_t<fmtster::internal::is_multimappish_v<C> >
        format_loop(const C& c, FCIt_t<FormatContext>& itFC)
    {
        using namespace fmtster::internal;

        auto itC = c.begin();
        while (itC != c.end())
        {
            const auto indent = mDisableBras ? mBraIndent : mDataIndent;

            std::string fmtStr;

            bool isFirstElement = (itC == c.begin());
            bool newLine = !mDisableBras || !isFirstElement;
            if (newLine)
                fmtStr = "\n";

            // output the key
            const auto& key = itC->first;
            if (fmtster::internal::is_string_v<decltype(key)>)
                fmtStr += "{}\"{}\" : ";
            else
                fmtStr += "{}{} : ";
            itFC = fmt::format_to(itFC, fmtStr, indent, EscapeIfJSONString(key));

            // insert each value with the same key into a temp vector to
            // print
            std::vector<typename C::mapped_type> vals;
            do
            {
                vals.insert(vals.begin(), itC->second);
                itC++;
            } while ((itC != c.end()) && (itC->first == key));

            fmtStr = (itC != c.end()) ? "{:{},{},{},{}}," : "{:{},{},{},{}}";
            itFC = format_to(itFC,
                            fmtStr,
                            vals,
                            mFormatSetting,
                            mJSONStyleHelper.mStyle.value,
                            "",
                            mIndentSetting);
        }
    } // format_loop() (multimaps)

    template<typename FormatContext>
    auto format(const T& sc, FormatContext& ctx)
    {
        using fmt::format_to;
        using namespace fmtster::internal;

        resolveArgs(ctx);

        auto itFC = ctx.out();

        // output opening bracket/brace (if enabled)
        if (!mDisableBras)
        {
            itFC = format_to(itFC, is_braceable_v<T> ? "{{" : "[");
            mIndentSetting++;
        }

        const bool empty = (sc.end() == sc.begin());

        if (empty && !mDisableBras)
        {
            itFC = format_to(itFC, is_braceable_v<T> ? " }}" : " ]");
        }
        else
        {
            format_loop<FormatContext, T>(sc, itFC);

            if (!mDisableBras)
            {
                // output closing brace
                itFC = format_to(itFC,
                                is_braceable_v<T> ? "\n{}}}" : "\n{}]",
                                mBraIndent);
            }
        }

        return itFC;
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
  : fmtster::Base
{
    template<typename FormatContext>
    auto format(const std::pair<T1, T2>& p, FormatContext& ctx)
    {
        using fmt::format_to;
        using namespace fmtster::internal;

        resolveArgs(ctx);

        auto itFC = ctx.out();

        // output opening bracket/brace (if enabled)
        if (!mDisableBras)
        {
            itFC = format_to(itFC, "{{\n");
            mIndentSetting++;
        }

        // WARNING: a pair that doesn't have a string first is not JSON compliant

        // key
        std::string fmtStr = fmtster::internal::is_string_v<T1>
                            ? "{}\"{}\" : "
                            : "{}{} : ";
        itFC = format_to(ctx.out(),
                        fmtStr,
                        mDisableBras ? mBraIndent : mDataIndent,
                        EscapeIfJSONString(p.first));

        // value
        if (fmtster::internal::is_fmtsterable_v<T2>)
        {
            itFC = format_to(itFC,
                            "{:{},{},{},{}}",
                            p.second,
                            mFormatSetting,
                            mJSONStyleHelper.mStyle.value,
                            "",
                            mIndentSetting);
        }
        else
        {
            fmtStr = fmtster::internal::is_string_v<T2>
                    ? "\"{}\""
                    : fmtStr = "{}";
            itFC = format_to(itFC,
                            fmtStr,
                            EscapeIfJSONString(p.second));
        }

        // output closing bracket/brace (if enabled)
        if (!mDisableBras)
        {
            itFC = format_to(itFC, "\n{}}}", mBraIndent);
        }

        return itFC;
    }
}; // struct fmt::formatter<std::pair<> >

// fmt::formatter<> for std::tuple<> (wraps group of heterogeneous objects
// known at compile time)
template<typename... Ts>
struct fmt::formatter<std::tuple<Ts...> >
  : fmtster::Base
{
    template<typename FormatContext>
    auto format(const std::tuple<Ts...>& tup, FormatContext& ctx)
    {
        using fmt::format_to;
        using namespace fmtster::internal;

        resolveArgs(ctx);

        auto itFC = ctx.out();

        const auto indent = mDisableBras ? mBraIndent : mDataIndent;

        // output opening brace (if enabled)
        if (!mDisableBras)
        {
            itFC = format_to(itFC, "{{");
            mIndentSetting++;
        }

        // get number of items in the tuple
        auto count = sizeof...(Ts);

        const bool empty = !count;

        if (empty && !mDisableBras)
        {
            // output space & closing brace at the same time
            itFC = format_to(itFC, " }}");
        }
        else
        {
            auto fn =
                [&](const auto& elem)
                {
                    std::string fmtStr;
                    if (!mDisableBras || (count != sizeof...(Ts)))
                        fmtStr = "\n";

                    if (fmtster::internal::is_fmtsterable_v<decltype(elem)>)
                    {
                        fmtStr += (--count) ? "{:{},{},{},{}}," : "{:{},{},{},{}}";
                        auto pcp = is_pair_v<simplify_type<decltype(elem)> > ? "-b" : "";
                        itFC = format_to(itFC,
                                         fmtStr,
                                         elem,
                                         mFormatSetting,
                                         mJSONStyleHelper.mStyle.value,
                                         pcp,
                                         mIndentSetting);
                    }
                    else
                    {
                        fmtStr += indent;

                        if (fmtster::internal::is_string_v<decltype(elem)>)
                            fmtStr += (--count) ? "\"{}\"," : "\"{}\"";
                        else
                            fmtStr += (--count) ? "{}," : "{}";

                        itFC = format_to(itFC,
                                         fmtStr,
                                         EscapeIfJSONString(elem));
                    }
                };
            std::apply([&](const auto&... elems){(fn(elems), ...);}, tup);

            // output closing brace (if enabled)
            if (!mDisableBras)
            {
                itFC = format_to(itFC, "\n{}}}", mBraIndent);
            }
        }

        return itFC;
    }
}; // struct fmt::formatter<std::tuple<> >

// fmt::formatter<> for fmtster::JSONStyle
template<>
struct fmt::formatter<fmtster::JSONStyle>
    : fmtster::Base
{
    template<typename FormatContext>
    auto format(const fmtster::JSONStyle& style, FormatContext& ctx)
    {
        using namespace std::string_literals;
        using std::make_pair;

        resolveArgs(ctx);

        auto itFC = ctx.out();

        const auto tup = std::make_tuple(
#if false // @@@ disable members that are not implemented
            make_pair("cr"s, style.cr),
            make_pair("lf"s, style.lf),
#endif // false
            make_pair("hardTab"s, style.hardTab),
            make_pair("tabCount"s, style.tabCount)
#if false // @@@ disable members that are not implemented
            ,
            make_pair("gapA"s, style.gapA),
            make_pair("gapB"s, style.gapB),
            make_pair("gapC"s, style.gapC),
            make_pair("gap1"s, style.gap1),
            make_pair("gap2"s, style.gap2),
            make_pair("gap3"s, style.gap3),
            make_pair("gap4"s, style.gap4),
            make_pair("gap5"s, style.gap5),
            make_pair("gap6"s, style.gap6),
            make_pair("gap7"s, style.gap7),
            make_pair("emptyArray"s, style.emptyArray),
            make_pair("emptyObject"s, style.emptyObject),
            make_pair("sva"s, style.sva),
            make_pair("svo"s, style.svo)
#endif // false
        );
        auto pcp = mDisableBras ? "-b" : "";
        itFC = fmt::format_to(itFC,
                            "{:{},{},{},{}}",
                            tup,
                            mFormatSetting,
                            mJSONStyleHelper.mStyle.value,
                            pcp,
                            mIndentSetting);

        return itFC;
    }
};

#undef JSONSTYLESTRUCT
