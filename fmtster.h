#pragma once

/* Copyright (c) 2021 Harman International.  All rights reserved.
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
#include <fmt/format.h>
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
using std::enable_if;
using std::is_same;
using std::integral_constant;

template<typename ...Args>
std::string F(std::string_view fmt, const Args&... args)
{
    return fmt::format(fmt, args...);
}

namespace internal
{

template<typename T, typename = void>
struct has_const_iterator : std::false_type
{};

template<typename T>
struct has_const_iterator<T, void_t<typename T::const_iterator> >
    : std::true_type
{};

// https://stackoverflow.com/a/30848101/3705286
//
template<typename, template<typename> class, typename = void_t<> >
struct detect : std::false_type
{};

template<typename T, template<typename> class Fn>
struct detect<T, Fn, std::void_t<Fn<T> > > : std::true_type
{};

template<typename T>
using has_begin_t = decltype(std::declval<T>().begin());

template<typename T>
using has_end_t = decltype(std::declval<T>().end());

template<typename T>
using has_begin = detect<T, has_begin_t>;

template<typename T>
using has_end = detect<T, has_end_t>;
//
///////////////////////////////////////

template<typename T>
struct is_container
    : integral_constant<bool,
                        has_const_iterator<T>::value
                        && has_begin<T>::value
                        && has_end<T>::value>
{};

template<>
struct is_container<string> : std::false_type
{};

// based on https://stackoverflow.com/a/35293958/3705286
//
template<typename T, typename U = void>
struct is_mappish : std::false_type
{};

template<typename T>
struct is_mappish<T, void_t<typename T::key_type,
                            typename T::mapped_type,
                            decltype(std::declval<T&>()[std::declval<const typename T::key_type&>()])>>
    : std::true_type
{};
//
///////////////////////////////////////

// stack: https://stackoverflow.com/a/29325258/3705286

template<typename, typename = void>
struct is_adapter : std::false_type
{};

template<typename T>
struct is_adapter<T, void_t<typename T::container_type> >
    : std::true_type
{};

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

} // namespace fmtster

template<typename T, typename Char>
struct fmt::formatter<T,
                      Char,
                      std::enable_if_t<fmtster::internal::is_container<T>::value
                                       && !fmtster::internal::is_mappish<T>::value> >
    : fmtster::FmtsterBase
{
    template<typename FormatContext>
    auto format(const T& sc, FormatContext& ctx)
    {
        // get starting output iterator
        auto itOut = ctx.out();

        // output opening bracket
        itOut = format_to(itOut, "[\n");

        // begin constructing format string for possibly recursive F() call
        // below, used for each entry in the map:
        //   {indent}{tab}
        const string fmtPrefix("{}{}");

        // iterate through and output each entry
        // all this iterator fun is to detect the last entry for the comma
        //  decision below without using sc.size(), which isn't available for
        //  all sequential containers (.e.g forrward_list<>)
        auto itSC = sc.begin();
        while (itSC != sc.end())
        {
            auto val = *itSC;
            itSC++;
            string nextFmtStr(fmtPrefix);
            nextFmtStr += appendValueFormatString(val, itSC != sc.end());
            // use format above
            itOut = format_to(itOut, nextFmtStr, mIndent, mTab, val);
        }
        // output closing brace
        itOut = format_to(itOut, "{}]", mIndent);
        return itOut;
    }
};

template<typename T, typename Char>
struct fmt::formatter<T,
                      Char,
                      std::enable_if_t<fmtster::internal::is_container<T>::value
                                       && fmtster::internal::is_mappish<T>::value> >
    : fmtster::FmtsterBase
{
    template<typename FormatContext>
    auto format(const T& ac, FormatContext& ctx)
    {
        // get starting output iterator
        auto itOut = ctx.out();

        // output opening brace
        itOut = format_to(itOut, "{{\n");

        // begin constructing format string for possibly recursive F() call
        // below, used for each entry in the map:
        //   {indent}{tab}\"{key}\"
        const string fmtPrefix("{}{}\"{}\": ");

        // iterate through and output each entry
        auto remainingElements = ac.size();
        for (const auto [key, val] : ac)
        {
            string nextFmtStr(fmtPrefix);
            nextFmtStr += appendValueFormatString(val, --remainingElements != 0);
            itOut = format_to(itOut, nextFmtStr, mIndent, mTab, key, val);
        }

        // output closing brace
        itOut = format_to(itOut, "{}}}", mIndent);

        return itOut;
    }
};

template<typename T>
struct fmt::formatter<stack<T> >
{
    string mStrFmt;

    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        auto itCtxEnd = find(ctx.begin(), ctx.end(), '}');
        mStrFmt = "{" + string(ctx.begin(), itCtxEnd) + "}";
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
    auto format(const stack<T>& ac, FormatContext& ctx)
    {
        return format_to(ctx.out(), mStrFmt, GetAdapterContainer(ac));
    }
};
