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

#include <algorithm>
#include <exception>
#include <fmt/core.h>
// #include <fmt/format.h>
#include "format.h"
#include <regex>
#include <string>
#include <type_traits>

namespace fmtster
{
using std::find;
using std::exception;
using std::string;
using std::stoi;
using std::declval;
using std::void_t;
using std::enable_if_t;
using std::is_same;
using std::integral_constant;
using std::true_type;
using std::false_type;
using std::conjunction_v;
using std::negation;

// short helper alias for fmt::format() used by adding "using fmtster::F;" to
//  client code
template<typename ...Args>
string F(std::string_view fmt, const Args&... args)
{
    return fmt::format(fmt, args...);
}

namespace internal
{

template<typename>
struct fmtster_true : true_type
{};

#define fmtster_MAKEHASFN(FN)                                               \
template<typename T, typename... Args>                                      \
static auto test_ ## FN(int)                                                \
    -> fmtster_true<decltype(declval<T>().FN(declval<Args>()...))>;         \
                                                                            \
template<typename, typename...>                                             \
static auto test_ ## FN(long) -> false_type;                                \
                                                                            \
template<typename T, typename... Args>                                      \
struct has_ ## FN : decltype(test_ ## FN<T, Args...>(0))                    \
{};                                                                         \
                                                                            \
template<typename ...Ts>                                                    \
inline constexpr bool has_ ## FN ## _v = has_ ## FN<Ts...>::value

#define fmtster_MAKEHASTYPE(TYPE)                                           \
template<typename T, typename = void>                                       \
struct has_ ## TYPE : false_type                                            \
{};                                                                         \
                                                                            \
template<typename T>                                                        \
struct has_ ## TYPE<T, void_t<typename T::TYPE> >                           \
    : true_type                                                             \
{};                                                                         \
                                                                            \
template<typename ...Ts>                                                    \
inline constexpr bool has_ ## TYPE ## _v = has_ ## TYPE<Ts...>::value

#define fmtster_MAKEIS(ID, COND)                                            \
template<typename T, typename = void>                                       \
struct is_ ## ID : false_type                                               \
{};                                                                         \
                                                                            \
template<typename T>                                                        \
struct is_ ## ID<T, enable_if_t<COND> > : true_type                         \
{};                                                                         \
                                                                            \
template<typename ...Ts>                                                    \
inline constexpr bool is_ ## ID ## _v = is_ ## ID<Ts...>::value

fmtster_MAKEHASTYPE(const_iterator);
fmtster_MAKEHASTYPE(key_type);
fmtster_MAKEHASTYPE(mapped_type);
fmtster_MAKEHASTYPE(container_type);

fmtster_MAKEHASFN(begin);
fmtster_MAKEHASFN(end);
fmtster_MAKEHASFN(at);
// fmtster_MAKEHASFN(operator[])
template<typename T, typename U = void>
struct has_operator_index : false_type
{};
template<typename T>
struct has_operator_index<T, void_t<decltype(declval<T&>()[declval<const typename T::key_type&>()])> >
    : true_type
{};

fmtster_MAKEIS(container, (conjunction_v<has_const_iterator<T>, has_begin<T>, has_end<T> >));
// specialization for std::string, which is not considered a container
template<>
struct is_container<string> : false_type
{};
fmtster_MAKEIS(mappish, (conjunction_v<has_key_type<T>, has_mapped_type<T>, has_operator_index<T> >));
fmtster_MAKEIS(multimappish, (conjunction_v<has_key_type<T>, has_mapped_type<T>, negation<has_at<T, typename T::key_type> > >));
fmtster_MAKEIS(adapter, has_container_type_v<T>);

} // namespace internal

struct FmtsterBase
{
    int mFormatSetting = 0;
    int mStyleSetting = 0;
    int mTabSetting = 2;
    int mIndentSetting = 0;
    string mTab;
    string mIndent;

    // Parses the format choice argument id in the format {<format>,<style>,<tab>,<indent>}.
    // format (default is 0: JSON):
    //   0 = JSON
    //     style (default is 0):
    //       0 = Google (https://google.github.io/styleguide/jsoncstyleguide.xml)
    // tab (default is 2 spaces):
    //   positive integers: spaces
    //   0: no tab
    //   negative integers: hard tabs
    // indent (default is 0):
    //   positive integers: number of <tab>s to start the indent
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        using fmtster::F;

        auto itCtxEnd = find(ctx.begin(), ctx.end(), '}');
        const string strFmt(ctx.begin(), itCtxEnd);

        std::regex rx("([^,]*),?([^,]*),?([^,]*),?([^,]*)");
        std::smatch sm;
        std::regex_search(strFmt, sm, rx);

        auto smFormat = sm[1].str();
        if (!smFormat.empty())
        {
            mFormatSetting = stoi(smFormat);
            if (mFormatSetting != 0)
                throw fmt::format_error(F("unsupported output format: \"{}\"", smFormat));
        }

        auto smStyle = sm[2].str();
        if (!smStyle.empty())
        {
            mStyleSetting = stoi(smStyle);
            if (mStyleSetting != 0)
                throw fmt::format_error(F("invalid style (\"{}\") for format \"{}\"", mStyleSetting, mFormatSetting));
        }

        auto smTab = sm[3].str();
        if (!smTab.empty())
        {
            mTabSetting = stoi(smTab);
        }

        auto smIndent = sm[4].str();
        if (!smIndent.empty())
        {
            mIndentSetting = stoi(smIndent);
            if (mIndentSetting < 0)
                throw fmt::format_error(F("invalid indent: \"{}\"", mIndentSetting));
        }

        mTab = ((mTabSetting > 0)       ?
              string(mTabSetting, ' ') :
              string(-mTabSetting, '\t'));
        mIndent.clear();
        for (int i = 0; i < mIndentSetting; i++)
            mIndent += mTab;

        return itCtxEnd;
    }

    template<typename T>
    string appendFormatString(const T&, bool addComma)
    {
        return addComma ? "{},\n" : "{}\n";
    }

    string appendFormatString(const string&, bool addComma)
    {
        return addComma ? "\"{}\",\n" : "\"{}\"\n";
    }

    template <typename T>
    string appendValueFormatString(const T& val, bool addComma)
    {
        using fmtster::F;

        if (internal::is_container<T>{})
        {
            return F("{{:{},{},{},{}}}{}\n",
                     mFormatSetting,
                     mStyleSetting,
                     mTabSetting,
                     mIndentSetting + 1,
                     addComma ? "," : "");
        }
        else
        {
            return appendFormatString(val, addComma);
        }
    }
};

template<typename T>
struct FmtsterMapBase : FmtsterBase
{
    template<typename FormatContext>
    auto format(const T& ac, FormatContext& ctx)
    {
        // get starting output iterator
        auto itOut = ctx.out();

        // output opening brace
        itOut = fmt::format_to(itOut, "{{\n");

        // begin constructing format string for possibly recursive F() call
        // below, used for each entry in the map:
        //   {indent}{tab}\"{key}\"
        const std::string fmtPrefix("{}{}\"{}\": ");

        // iterate through and output each entry
        auto remainingElements = ac.size();
        for (const auto [key, val] : ac)
        {
            std::string nextFmtStr(fmtPrefix);
            nextFmtStr += appendValueFormatString(val, --remainingElements != 0);
            itOut = fmt::format_to(itOut, nextFmtStr, mIndent, mTab, key, val);
        }

        // output closing brace
        itOut = fmt::format_to(itOut, "{}}}", mIndent);

        return itOut;
    }
};

} // namespace fmtster

// single value per entry containers
template<typename T, typename Char>
struct fmt::formatter<T,
                      Char,
                      std::enable_if_t<std::conjunction_v<fmtster::internal::is_container<T>,
                                                          std::negation<fmtster::internal::is_mappish<T> >,
                                                          std::negation<fmtster::internal::is_multimappish<T> > > > >
    : fmtster::FmtsterBase
{
    template<typename FormatContext>
    auto format(const T& sc, FormatContext& ctx)
    {
        // get starting output iterator
        auto itOut = ctx.out();

        // output opening bracket
        itOut = fmt::format_to(itOut, "[\n");

        // begin constructing format string for possibly recursive F() call
        // below, used for each entry in the map:
        //   {indent}{tab}
        const std::string fmtPrefix("{}{}");

        // iterate through and output each entry
        // all this iterator fun is to detect the last entry for the comma
        //  decision below without using sc.size(), which isn't available for
        //  all sequential containers (.e.g forrward_list<>)
        auto itSC = sc.begin();
        while (itSC != sc.end())
        {
            auto val = *itSC;
            itSC++;
            std::string nextFmtStr(fmtPrefix);
            nextFmtStr += appendValueFormatString(val, itSC != sc.end());
            // use format above
            itOut = fmt::format_to(itOut, nextFmtStr, mIndent, mTab, val);
        }
        // output closing brace
        itOut = fmt::format_to(itOut, "{}]", mIndent);
        return itOut;
    }
};

// unique key key/value containers
template<typename T, typename Char>
struct fmt::formatter<T,
                      Char,
                      std::enable_if_t<std::conjunction_v<fmtster::internal::is_container<T>,
                                                          fmtster::internal::is_mappish<T> > > >
    : fmtster::FmtsterMapBase<T>
{};

// multiple key key/value containers
template<typename T, typename Char>
struct fmt::formatter<T,
                      Char,
                      std::enable_if_t<std::conjunction_v<fmtster::internal::is_container<T>,
                                                          fmtster::internal::is_multimappish<T> > > >
    : fmtster::FmtsterMapBase<T>
{};

// adapters
template<typename A, typename Char>
struct fmt::formatter<A,
                      Char,
                      std::enable_if_t<fmtster::internal::is_adapter<A>::value> >
{
    std::string mStrFmt;

    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        auto itCtxEnd = find(ctx.begin(), ctx.end(), '}');
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
};
