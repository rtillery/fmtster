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
#include <cassert>
#include <cstdint>
#include <fmt/core.h>
#include <fmt/format.h>
#include <list>
#include <mutex>
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

//
// Short helper alias for fmt::format() used by adding "using fmtster::F;" to
// client code
//
template<typename... Args>
string F(std::string_view fmt, const Args&... args)
{
    return fmt::format(fmt, args...);
}

//
// Enumeration for use in fmtster::JSONStyle (defined below)
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
// Definition of XXXStyle structures, reused multiple times below
//
#if false // @@@ TODO: Implement members commented out below

#define JSONSTYLESTRUCT                                                        \
    {                                                                          \
        /* note that these two booleans can never both be false, so they  */   \
        /* double as a method of differentiating a default-meaning 0 from */   \
        /* an actual configuration setting (i.e. 0 always means default)  */   \
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

// @@@ TODO: See above (only these are currently implemented)
#define JSONSTYLESTRUCT                                                        \
    {                                                                          \
        bool hardTab : 1;                                                      \
        unsigned int tabCount : 4;                                             \
    }

#endif // true

// forward declaration
struct Base;

namespace internal
{

// Needed in the has_FN<> macro below
template<typename>
struct fmtster_true
  : true_type
{};

//
// Template to simplify a type to its base for comparisons below
//
template<typename T>
using simplify_type = std::remove_cv_t<std::remove_reference_t<T> >;

//
// Macro to create has_FN<> templates and has_FN_v<> helpers
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
// Macro to create has_TYPE<> templates and has_TYPE_v<> helpers
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
// Macro to create is_ID<> templates and is_ID_v<> helpers
//     COND provides the conditional value used to check traits
//
// NOTE 1: Any spaces or angle brackets in the COND argument are
//         misinterpreted by the parser, so use of parentheses is necessary
// NOTE 2: COND _can_ be a logical grouping of xxx_v<> values, but it appears
//         that all of the terms are evaluated, regardless of order, making
//         some combinations fail before the SFINAE can kick in.
//         std::conjunction_v<> and std::disjunction_v<> seem to use lazy
//         evaluation, so use of them is often necessary
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

fmtster_MAKEIS(fmtsterable,
               (std::is_base_of_v<fmtster::Base, fmt::formatter<simplify_type<T> > >));
fmtster_MAKEIS(braceable, (disjunction_v<is_mappish<T>,
                                         is_multimappish<T>,
                                         is_pair<T>,
                                         is_tuple<T> >));

//
// Used to measure the sizes of the bitfield-based XXXStyle structures to
// determine the size of the large (unsigned) integer necessary to use when
// the unions are declared.
//
struct MeasureJSONStyle JSONSTYLESTRUCT;

// If well packed, each style structure will fit into 64 bits, but if not, we
// can use 128 bits. NOTE: VALUE_T is meant to be the maximum size necessary
// to be used for all XXXStyle unions.
template<size_t bytes>
using VALUE_T_SELECTOR =
    std::conditional_t<(bytes <= 8),
                       uint64_t,
                       std::conditional_t<((bytes > 8) && (bytes <= 16)),
                                          __uint128_t,
                                          void>
                      >;
using VALUE_T = VALUE_T_SELECTOR<16 /* sizeof(MeasureJSONStyle) */>;

//
// Used to define the DEFAULTxxxCONFIG instances before their use as default
// values in the XXXStyle constructors.
//
union ForwardJSONStyle
{
    struct JSONSTYLESTRUCT;
    VALUE_T value;
};

} // namespace internal

//
// Built-in defaults, which can be replaced by user-configured defaults
// for each format.
//
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

//
// {fmt} does not currently allow custom types to be passed as a nested
// argument. The workaround is to define the XXXStyle objects as unions
// with the large (unsigned) integer (size calculated above). This also
// requires another argument to specify which of the XXXStyle objects was
// passed.
// These are defined as unions between the large (unsigned) integer and
// anonymous structures to allow the user to directly access to individual
// style members.
union JSONStyle
{
public:
    struct JSONSTYLESTRUCT;
    internal::VALUE_T value;

    JSONStyle(internal::VALUE_T val = DEFAULTJSONCONFIG.value)
      : value(val ? val : DEFAULTJSONCONFIG.value)
    {}
};

namespace internal
{

//
// Templated structure used to cache XXXStyleHelper expansions associated with
// XXXStyle unions (via XXXStyle.value).
//
template<typename TVALUE, typename TSTYLEHELPER, typename Compare = std::map<int, int>::key_compare>
struct LRUCache
{
    using LRUList_t = std::list<TVALUE>; // FRONT (index 0) is NEWEST

    using Cache_t = std::map<TVALUE, 
                             std::pair<std::shared_ptr<TSTYLEHELPER>,
                                       typename LRUList_t::const_iterator>,
                             Compare>;
    static constexpr size_t PAIR_KEY_INDEX = 0;
    static constexpr size_t PAIR_T_INDEX = 1;
    static constexpr size_t SHARED_PTR_OBJ_INDEX = 0;
    static constexpr size_t LIST_ITERATOR_INDEX = 1;

    const size_t mCacheLimit;

    std::mutex mMutex;

    LRUList_t mLRUList;
    Cache_t mCacheMap;

    LRUCache(size_t limit = 1)
      : mCacheLimit(limit ? limit : 1)
    {
        assert(limit);
    }

    //
    // A shared_ptr<> is returned here to ensure that eviction of an object
    // in one thread, does not free an object in use by another thread.
    //
    std::shared_ptr<TSTYLEHELPER> getStyleHelper(const TVALUE& value)
    {
        // To avoid screwing up the containers, allow only one thread at a
        // time to manipulate them.
        std::scoped_lock lock(mMutex);

        // Get existing or create new object from/in the cache.
        auto [itMap, inserted] =
            mCacheMap.emplace(value,
                              make_pair(std::make_shared<TSTYLEHELPER>(value), // XXXStyleHelper(value)
                                        mLRUList.end()));
        auto& prMapElem = *itMap;
        auto& prMappedVal = std::get<PAIR_T_INDEX>(prMapElem);
        auto& sptrObj = std::get<SHARED_PTR_OBJ_INDEX>(prMappedVal);

        if (inserted)
        {
            while (mLRUList.size() >= mCacheLimit)
            {
                mCacheMap.erase(*(mLRUList.rbegin()));
                mLRUList.pop_back();
            }

            mLRUList.emplace_front(value);
        }
        else
        {
            auto& itListMapped = std::get<LIST_ITERATOR_INDEX>(prMappedVal);
            if (itListMapped != mLRUList.begin())
            {
                // Move already existing value to front of LRU list
                mLRUList.splice(mLRUList.begin(), mLRUList, itListMapped);
            }
        }

        return sptrObj;
    }
}; // struct LRUCache<>

//
// Base struct used by all serialization format style helpers.
//
struct StyleHelper
{
    VALUE_T mStyleValue;

    //
    // Common expanded strings
    //
    string tab;

    StyleHelper(VALUE_T value)
      : mStyleValue(value)
    {}

    // Needed to enable polymorphism
    virtual ~StyleHelper() = default;

    //
    // Functions to convert string types to a desired numeric type.
    //
    template<typename T>
    T toValue(const char* sz, const string& throwArg = "")
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
    T toValue(const string& str)
    {
        return toValue<T>(str.c_str());
    }
    
    //
    // Functions to convert input types to a format index.
    //
    int formatToValue(__int128_t i)
    {
        if (i != 0)
            throw fmt::format_error(F("fmtster: unsupported format argument value: {}", i));
        return i;
    }
    template<typename T>
    int formatToValue(T i)
    {
        return formatToValue((__int128_t)i);
    }
    int formatToValue(const char* const sz)
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
                format = toValue<int>(sz, "format");
            }
        }
        return formatToValue(format);
    }
    int formatToValue(const string& str)
    {
        return formatToValue(str.c_str());
    }
    int formatToValue(const fmt::basic_string_view<char>& str)
    {
        return formatToValue(str.data());
    }
}; // StyleHelper

// forward declaration
class JSONStyleHelper;

//
// Behind-the-scenes workhorse based on JSONStyle.
//

using JSONLRUCache_t = LRUCache<VALUE_T, JSONStyleHelper>;

class JSONStyleHelper
  : public StyleHelper
{
protected:
    static VALUE_T& DefaultStyleValue()
    {
        static VALUE_T defaultStyleValue = JSONStyleHelper(0).mStyleValue;
        return defaultStyleValue;
    }

    void expand()
    {
        const JSONStyle style(mStyleValue);
        tab = style.hardTab
              ? string(style.tabCount, '\t')
              : string(style.tabCount, ' ');
    }

public:
    static JSONLRUCache_t& GetLRUCache()
    {
        static JSONLRUCache_t lruCache(5);
        return lruCache;
    }

    static VALUE_T GetDefaultStyleValue()
    {
        return DefaultStyleValue();
    }

    static void SetDefaultStyleValue(VALUE_T value)
    {
        DefaultStyleValue() = value;
    }

    JSONStyleHelper(VALUE_T val = DEFAULTJSONCONFIG.value)
      : StyleHelper(val ? val : DEFAULTJSONCONFIG.value)
    {
        expand();
    }

    //
    // Functions used to escape string the JSON way (or just return the value
    // if any other type). Design needed to keep compiler happy below.
    //
    template<typename T>
    T escapeIfString(const T& val)
    {
        return val;
    }
    // escape string the JSON way
    string escapeIfString(const string& strIn)
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
    } // escapeIfString()
}; // class JSONStyleHelper

} // namespace internal

//
// Main fmtster::Base class that handles fmtster formatting including parsing
// the arguments (nested or not) and helper functions to resolve them.
//
struct Base
{
protected:
    // Order of the fmster arguments
    static constexpr size_t INDENT_ARG_INDEX = 0;
    static constexpr size_t PER_CALL_ARG_INDEX = 1;
    static constexpr size_t STYLE_ARG_INDEX = 2;
    static constexpr size_t FORMAT_ARG_INDEX = 3;

    // Results of parse() for use in format()
    vector<string> mArgData;
    vector<unsigned int> mNestedArgIndex;

    // From format arg (int due to code in formatToValue(const char*))
    int mFormatSetting;

    // From style arg
    internal::VALUE_T mStyleValue;
    std::shared_ptr<internal::StyleHelper> mpStyleHelper;

    //
    // From per call parms
    //
    bool mDisableBras;

    // From indent arg
    size_t mIndentSetting;

    // Expanded indent strings
    string mBraIndent;  // brace/bracket indent
    string mDataIndent; // data indent

    static int& DefaultFormat()
    {
        static int defaultFormat = 0;
        return defaultFormat;
    };

    static void SetDefaultFormat(int format)
    {
        if (format != 0)
            throw fmt::format_error(F("fmtster: Shouldn't get here ({}), because unsupported format should have already been thrown", __LINE__));
        DefaultFormat() = format;
    }

    // Function to pass along string to specified format type helper for
    // escaping.
    template<typename T>
    T escapeIfString(int format, const T& val)
    {
        switch (format)
        {
        case 0:
            return dynamic_cast<internal::JSONStyleHelper*>(mpStyleHelper.get())->escapeIfString(val);

        default:
            throw fmt::format_error(F("fmtster: Shouldn't get here ({}), because unsupported format should have already been thrown", __LINE__));
        }
    }

    //
    // resolveFormatArg()
    //
    template<typename FormatContext>
    void resolveFormatArg(FormatContext& ctx)
    {
        using namespace fmtster::internal;

        if (mNestedArgIndex[FORMAT_ARG_INDEX])
        {
            auto formatArg = ctx.arg(mNestedArgIndex[FORMAT_ARG_INDEX]);
            auto formatSetting = visit_format_arg(
                [this](auto value) -> int
                {
                    // This construct is required because at compile time all
                    //  paths are linked, even though that are not allowed at
                    //  run time, and without this, the return value doens't
                    //  match the function return value in some cases, so the
                    //  compile fails.
                    using val_t = simplify_type<decltype(value)>;
                    if constexpr (std::is_integral_v<val_t>)
                    {
                        return mpStyleHelper->formatToValue(value); // to check for supported formats
                    }
                    else if constexpr (internal::is_string_v<val_t>)
                    {
                        return mpStyleHelper->formatToValue(value); // to convert formats
                    }
                    else
                    {
                        throw fmt::format_error(F("fmtster: unsupported nested argument type for format: {} (only integers and strings accepted)",
                                                  typeid(value).name()));
                    }
                },
                formatArg
            );

            mFormatSetting = mpStyleHelper->formatToValue(formatSetting);
        }
        else if(!mArgData[FORMAT_ARG_INDEX].empty())
        {
            mFormatSetting = mpStyleHelper->formatToValue(mArgData[FORMAT_ARG_INDEX]);
        }
    } // resolveFormatArg()

    //
    // resolveStyleArg()
    //
    template<typename FormatContext>
    void resolveStyleArg(FormatContext& ctx)
    {
        using namespace fmtster::internal;

        if (mNestedArgIndex[STYLE_ARG_INDEX])
        {
            auto styleArg = ctx.arg(mNestedArgIndex[STYLE_ARG_INDEX]);
            mStyleValue = visit_format_arg(
                [](auto value) -> VALUE_T
                {
                    // This construct is required because at compile time all
                    //  type paths are linked, even though they are not allowed
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
        }
        else if(!mArgData[STYLE_ARG_INDEX].empty())
        {
            mStyleValue = mpStyleHelper->toValue<VALUE_T>(mArgData[STYLE_ARG_INDEX]);
        }

        switch (mFormatSetting)
        {
        case 0:
            mpStyleHelper = JSONStyleHelper::GetLRUCache().getStyleHelper(mStyleValue);
            break;

        default:
            throw fmt::format_error(F("fmtster: Shouldn't get here ({}), because unsupported format should have already been thrown", __LINE__));
        }
    } // resolveStyleArg()

    //
    // resolvePCPArg()
    //
    template<typename FormatContext>
    string resolvePCPArg(FormatContext& ctx)
    {
        using namespace fmtster::internal;

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
                    else if constexpr (std::is_same_v<simplify_type<fmt::string_view>, simplify_type<decltype(value)> >)
                    {
                        return value.data();
                    }
                    else
                    {
                        throw fmt::format_error(F("fmtster: unsupported nested argument type for per call parameters: {} (only strings accepted)",
                                                  typeid(value).name()));
                    }
                },
                pcpArg);
        }
        else if (!mArgData[PER_CALL_ARG_INDEX].empty())
        {
            pcpSetting = mArgData[PER_CALL_ARG_INDEX];
        }

        return pcpSetting;
    } // resolvePCPArg()

    //
    // resolveIndentArg()
    //
    template<typename FormatContext>
    void resolveIndentArg(FormatContext& ctx)
    {
        using namespace fmtster::internal;

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
                        throw fmt::format_error(F("fmtster: unsupported nested argument type for indent: {} (only integers accepted)",
                                                  typeid(value).name()));
                    }
                },
                indentArg);

            mIndentSetting = indentSetting;
        }
        else if (!mArgData[INDENT_ARG_INDEX].empty())
        {
            mIndentSetting =
                mpStyleHelper->toValue<decltype(mIndentSetting)>(mArgData[INDENT_ARG_INDEX]);
        }
        else
        {
            mIndentSetting = 0;
        }

    } // resolveIndentArg()

public:
    static int GetDefaultFormat()
    {
        return DefaultFormat();
    }

    // Enable specialized functions for each format style (defained below Base
    // definition until C++20?)
    template<typename T>
    static const T GetDefaultStyle();

    Base()
      : mArgData{ "" },
        mNestedArgIndex{ 0 },
        mFormatSetting(GetDefaultFormat()),
        mIndentSetting(0),
        mDisableBras(false)
    {
        switch (mFormatSetting)
        {
        case 0:
            mStyleValue = internal::JSONStyleHelper::GetDefaultStyleValue();
            break;

        default:
            throw fmt::format_error(F("fmtster: Shouldn't get here ({}), because unsupported format should have already been thrown", __LINE__));
        }
    }

    //
    // Generic parser for however many comma-separated arguments are provided,
    // including support for nested arguments (requires call to
    // Base::resolveArgs() in Base children's format()).
    // 
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
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

    //
    // This function must be called by each Base child immediately on
    // entry to the format() function. It completes the updating of the style
    // object based on arguments provided, including nested, if necessary.
    // Only the first four aguments are processed.
    //
    template<typename FormatContext>
    void resolveArgs(FormatContext& ctx)
    {
        using namespace std::string_literals;
        using fmt::format_to;

        // Ensure vectors are large enough for unchecked processing below
        mArgData.resize(4, "");
        mNestedArgIndex.resize(4, 0);

        // Resolve each argument
        resolveFormatArg(ctx);
        resolveStyleArg(ctx);
        auto pcpSetting = resolvePCPArg(ctx);
        resolveIndentArg(ctx);

        // Configure indentation
        mBraIndent.clear();
        for (auto i = mIndentSetting; i; --i)
            mBraIndent += mpStyleHelper->tab;
        mDataIndent = mBraIndent + mpStyleHelper->tab;

        //
        // Parse the per call parms
        //
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
                    SetDefaultFormat(mFormatSetting);
                break;

            case 's':
                if (!negate)
                {
                    switch (mFormatSetting)
                    {
                    case 0:
                        internal::JSONStyleHelper::SetDefaultStyleValue(mStyleValue);
                        break;

                    default:
                        throw fmt::format_error(F("fmtster: Shouldn't get here ({}), because unsupported format should have already been thrown", __LINE__));
                    }
                }
                break;

            default:
                ;
            }

            negate = (c == '-');

        } /// for(const auto c : pcpSetting)

    } // resolveArgs()

}; // struct FmtterBase

//
// Specialization for each XXXStyle.
//
template<>
const JSONStyle Base::GetDefaultStyle()
{
    return JSONStyle(internal::JSONStyleHelper::GetDefaultStyleValue());
}

} // namespace fmtster

//
// fmt::formatter<> used for all containers (not adapters)
//
template<typename T, typename Char>
struct fmt::formatter<T,
                      Char,
                      std::enable_if_t<fmtster::internal::is_container_v<T> > >
  : fmtster::Base
{
    template<typename FormatContext>
    using FCIt_t = decltype(std::declval<FormatContext>().out());

    //
    // Templated function inner loop for ALL CONTAINERS EXCEPT MULTIMAPS.
    // (This is able to be used for single data per element & map-like
    // containers, because map-like containers use a std::pair<> for each
    // element.)
    //
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

            using SimpleValType = simplify_type<decltype(val)>;

            auto escVal = escapeIfString(mFormatSetting, val);

            if (is_fmtsterable_v<SimpleValType>)
            {
                if (!is_pair_v<SimpleValType>)
                    fmtStr += indent;

                fmtStr += isLastElement ? "{:{},{},{},{}}" : "{:{},{},{},{}},";
                auto pcp = is_pair_v<simplify_type<decltype(val)> > ? "-b" : "";
                itFC = fmt::format_to(itFC,
                                      fmtStr,
                                      escVal,
                                      mIndentSetting,
                                      pcp,
                                      mStyleValue,
                                      mFormatSetting);
            }
            else
            {
                fmtStr += indent;

                if (fmtster::internal::is_string_v<decltype(val)>)
                    fmtStr += isLastElement ? "\"{}\"" : "\"{}\",";
                else
                    fmtStr += isLastElement ? "{}" : "{},";
                itFC = fmt::format_to(itFC,
                                      fmtStr,
                                      escVal);
            }
        }
    } // format_loop() (all containers except multimap)

    //
    // Templated function inner loop for MULTIMAPS.
    //
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
            itFC = fmt::format_to(itFC,
                                  fmtStr,
                                  indent,
                                  escapeIfString(mFormatSetting, key));

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
                            mIndentSetting,
                            "",
                            mStyleValue,
                            mFormatSetting);
        }
    } // format_loop() (multimaps)

    //
    // format()
    //
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

    } // format()

}; // struct fmt::formatter< containers >

//
// fmt::formatter<> for adapters.
//
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

//
// fmt::formatter<> for std::pair<>.
//
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
                        escapeIfString(mFormatSetting, p.first));

        // value
        if (fmtster::internal::is_fmtsterable_v<T2>)
        {
            itFC = format_to(itFC,
                            "{:{},{},{},{}}",
                            p.second,
                            mIndentSetting,
                            "",
                            mStyleValue,
                            mFormatSetting);
        }
        else
        {
            fmtStr = fmtster::internal::is_string_v<T2>
                    ? "\"{}\""
                    : fmtStr = "{}";
            itFC = format_to(itFC,
                            fmtStr,
                            escapeIfString(mFormatSetting, p.second));
        }

        // output closing bracket/brace (if enabled)
        if (!mDisableBras)
        {
            itFC = format_to(itFC, "\n{}}}", mBraIndent);
        }

        return itFC;
    }
}; // struct fmt::formatter<std::pair<> >

//
// fmt::formatter<> for std::tuple<> (wraps group of heterogeneous objects
// known at compile time).
//
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
                                         mIndentSetting,
                                         pcp,
                                         mStyleValue,
                                         mFormatSetting);
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
                                         escapeIfString(mFormatSetting, elem));
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

//
// fmt::formatter<> for dumping fmtster::JSONStyle.
//
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
            make_pair("value"s, style.value),
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
                            mIndentSetting,
                            pcp,
                            mStyleValue,
                            mFormatSetting);

        return itFC;
    }
};

#undef JSONSTYLESTRUCT
