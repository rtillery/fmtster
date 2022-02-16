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

#include <chrono>
using ex_clock_t = std::chrono::system_clock;
using ex_time_point_t = std::chrono::time_point<ex_clock_t>;
#include <iomanip>
#include <iostream>
using std::cout;
using std::endl;
#include <map>
using std::map;
#include <sstream>
using std::stringstream;
#include <string>
using std::string;
using namespace std::string_literals;
#include <tuple>
using std::tuple;
#include <utility>
using std::make_pair;
#include <variant>
using std::variant;
#include <vector>
using std::vector;

#include "fmtster.h"
using fmtster::F;

#include <list>
#include <cassert>


template<typename T1, typename T2>
constexpr auto mp(T1 f, T2 s)
{
    return std::make_pair(f, s);
};

template<typename ...Ts>
constexpr auto mt(Ts... es)
{
    return std::make_tuple(es...);
}

struct Person1
{
    string name;
    ex_time_point_t birthdate;
    float salary;
    vector<string> phones;
    map<string, string> family;
};

struct Person2
{
    string name;
    ex_time_point_t birthdate;
    float salary;
    vector<string> phones;
    map<string, string> family;
};

template<>
struct fmt::formatter<Person1>
  : fmtster::Base
{
    template<typename FormatContext>
    auto format(const Person1& p, FormatContext& ctx)
    {
        // direct approach (take special care with commas and carriage returns!)

        resolveArgs(ctx);

        auto itFC = ctx.out();

        const auto indent = mDisableBras ? mBraIndent : mDataIndent;

        // output opening brace (if enabled)
        if (!mDisableBras)
        {
            itFC = format_to(itFC, "{{\n");
            mIndentSetting++;
        }

        itFC = format_to(itFC,
                         "{:{},{},{},{}},\n",
                         mp("name"s, p.name),
                         mIndentSetting,
                         "-b",
                         mStyleValue,
                         mFormatSetting);

        // @@@ TODO: Wrap this section for use with C++20
        stringstream ss;
        auto t = ex_clock_t::to_time_t(p.birthdate);
        auto tm = *std::localtime(&t);
        ss << std::put_time(&tm, "%x");

        itFC = format_to(itFC,
                         "{:{},{},{},{}},\n",
                         mp("birthdate"s, ss.str()),
                         mIndentSetting,
                         "-b",
                         mStyleValue,
                         mFormatSetting);
        itFC = format_to(itFC,
                         "{:{},{},{},{}},\n",
                         mp("salary"s, p.salary),
                         mIndentSetting,
                         "-b",
                         mStyleValue,
                         mFormatSetting);
        itFC = format_to(itFC,
                         "{:{},{},{},{}},\n",
                         mp("phones"s, p.phones),
                         mIndentSetting,
                         "-b",
                         mStyleValue,
                         mFormatSetting);

        // Note this entry's format string does not end a comma
        itFC = format_to(itFC,
                         "{:{},{},{},{}}",
                         mp("family"s, p.family),
                         mIndentSetting,
                         "-b",
                         mStyleValue,
                         mFormatSetting);

        if (!mDisableBras)
            itFC = fmt::format_to(itFC, "\n{}}}", mBraIndent);

        return itFC;
    }
};

template<>
struct fmt::formatter<Person2>
  : fmtster::Base
{
    template<typename FormatContext>
    auto format(const Person2& p, FormatContext& ctx)
    {
        // tuple approach

        resolveArgs(ctx);

        // @@@ TODO: Wrap this section for use with C++20
        stringstream ss;
        auto t = ex_clock_t::to_time_t(p.birthdate);
        auto tm = *std::localtime(&t);
        ss << std::put_time(&tm, "%x");

        auto tup = mt(
            mp("name"s, p.name),
            mp("birthdate"s, ss.str()),
            mp("salary"s, p.salary),
            mp("phones"s, p.phones),
            mp("family"s, p.family)
        );
        return format_to(ctx.out(),
                         "{:{},{},{},{}}",
                         tup,
                         mIndentSetting,
                         mDisableBras ? "-b" : "",
                         mStyleValue,
                         mFormatSetting);
    }
};

using Personnel1 = vector<Person1>;
using Personnel2 = vector<Person2>;

template<typename T>
T GetPersonnel()
{
    // birthdates
    static std::tm tm1{ 0, 0, 0,
                        1, 1 - 1, 1970 - 1900 };
    static std::tm tm2{ 0, 0, 0,
                        31, 12 - 1, 1980 - 1900 };
    return T
    {
        {
            .name = "John Doe",
            .birthdate = ex_clock_t::from_time_t(::timelocal(&tm1)),
            .salary = 60000,
            .phones = { "1-800-555-1212", "867-5309" },
            .family = { { "sister", "Jane Doe" },
                        { "mother", "Janet Doe" },
                        { "brother", "Jake Doe" },
                        { "father", "James Doe" } }
        }, {
            .name = "Jane Doe",
            .birthdate = ex_clock_t::from_time_t(::timelocal(&tm2)),
            .salary = 60001,
            .phones = { "1-888-555-8888", "736-5000" },
            .family = { { "brother", "John Doe" },
                        { "mother", "Janet Doe" },
                        { "brother", "Jake Doe" },
                        { "father", "James Doe" } }
        }
    };
}

struct Color
{
    string hue; // color family
    vector<tuple<string, float> > primaries;
};

template<>
struct fmt::formatter<Color>
  : fmtster::Base
{
    template<typename FormatContext>
    auto format(const Color& color, FormatContext& ctx)
    {
        using std::make_pair;

        resolveArgs(ctx);

        auto tup = std::make_tuple(
            make_pair("hue"s, color.hue),
            make_pair("primaries"s, color.primaries)
        );
        return format_to(ctx.out(),
                         "{:{},{},{},{}}",
                         tup,
                         mIndentSetting,
                         mDisableBras ? "-b" : "",
                         mStyleValue,
                         mFormatSetting);
    }
};



using VALUE_T = fmtster::internal::VALUE_T;
using JSONStyleHelper = fmtster::internal::JSONStyleHelper;

template<typename TKEY, typename TOBJ>
class LRU
{
protected:
public:
    using LRUList_t = std::list<TKEY>;
    using Cache_t = std::map<TKEY, std::tuple<TOBJ, typename LRUList_t::iterator> >;
    static constexpr size_t OBJ_INDEX = 0;
    static constexpr size_t ITERATOR_INDEX = 1;

    mutable LRUList_t mLRUList;
    mutable Cache_t mCache;

    const size_t mCacheLimit;

    LRU(size_t limit = 1)
      : mCacheLimit(limit ? limit : 1)
    {
        assert(limit);
    }

    TOBJ& getObj(const TKEY& key) const
    {
cout << __LINE__ << endl;
        // get or create object in cache
        auto [itTupPr, inserted] = mCache.emplace(key, make_pair(0.0, mLRUList.end()));
        auto& pr = std::get<1>(*itTupPr);
        auto& obj = std::get<0>(pr);
        auto it = std::get<1>(pr);
//         auto [obj, it] = *itTupObj;
cout << F("tupObj: {} : {}, iterator", key, obj) << endl;

cout << __LINE__ << endl;
//         auto it = std::get<ITERATOR_INDEX>(*itTupObj);
cout << __LINE__ << endl;
        if (!inserted) // it == LRUList_t::end())
        {
cout << __LINE__ << endl;
            // erase least recently used obj(s) if cache is full, to make room
            // for new object
            while (mLRUList.size() >= mCacheLimit)
            {
cout << __LINE__ << endl;
                mCache.erase(*(mLRUList.rend()));
                mLRUList.pop_back();
            }

cout << __LINE__ << endl;
            // push newly created obj reference into front of LRU list
            mLRUList.push_front(key);
        }
        else if (it != mLRUList.begin())
        {
            // move existing obj reference to front of LRU list
cout << __LINE__ << endl;
                mLRUList.splice(mLRUList.begin(), mLRUList, it);
        }

cout << __LINE__ << endl;
        return obj; // std::get<OBJ_INDEX>(tupObj);
    }
};


template<typename T>
void DumpLRU(const T& lru)
{
    cout << F("lru ({}):", lru.mLRUList.size()) << endl;
    for (auto val : lru.mLRUList)
    {
        auto tup = lru.mCache[val];
        cout << F("{:1}\n", make_pair(val, std::get<T::OBJ_INDEX>(tup)));
    }
    cout << endl;
cout << __LINE__ << endl;
}


int main()
{
cout << __LINE__ << endl;
    LRU<string, float> lru(5);
cout << __LINE__ << endl;
    DumpLRU(lru);
cout << __LINE__ << endl;
    lru.getObj("pi") = 3.14;
cout << __LINE__ << endl;
    DumpLRU(lru);
cout << __LINE__ << endl;

    cout << "\n\n" << endl;


    // Based on https://json.org/example.html
    auto GlossSeeAlso = vector<string>{ "GML", "XML" };
    auto GlossDef = mt(mp("para"s, "A meta-markup language, used to create markup languages such as DocBook."s),
                       mp("GlossSeeAlso"s, GlossSeeAlso));
    auto GlossEntry = mt(mp("ID"s, "SGML"s),
                         mp("SortAs"s, "SGML"s),
                         mp("GlossTerm"s, "Standard Generalized Markup Language"s),
                         mp("Acronym"s, "SGML"s),
                         mp("Abbrev"s, "ISO 8879:1986"s),
                         mp("GlossDef"s, GlossDef),
                         mp("GlossSee"s, "markup"s));
    auto GlossList = mp("GlossEntry"s, GlossEntry);
    auto GlossDiv = mt(mp("title"s, "S"s),
                       mp("GlossList"s, GlossList));
    auto glossary = mt(mp("title"s, "example glossary"s),
                       mp("GlossDiv"s, GlossDiv));
    auto obj = mt(mp("glossary"s, glossary));
    cout << F("{}", obj) << endl;


    cout << "\n\n" << endl;


    // vector of Person1, where Person1 has a specialized fmt::formatter<>
    // based on fmtster
    size_t personNumber = 0;
    for (auto person : GetPersonnel<Personnel1>())
        cout << F("{} : {}\n", personNumber++, person) << endl;

    cout << "\n\n" << endl;

    // vector of Person2, where Person2 has a specialized fmt::formatter<>
    // based on fmtster
    personNumber = 0;
    for (auto person : GetPersonnel<Personnel2>())
        cout << F("{} : {}\n", personNumber++, person) << endl;

    cout << "\n\n" << endl;

    // map of string to Color, where Color has a specialized fmt::formatter<>
    // based on fmtster
    map<string, Color> colors
    {
        {
            "Burgundy",
            {
                "red",
                {
                    { "red", 1.0 }
                }
            }
        },
        {
            "Gray",
            {
                "none",
                {
                    { "red", 1.0/3.0 },
                    { "blue", 1.0/3.0 },
                    { "yellow", 1.0/3.0 }
                }
            }
        }
    };

    // make 4-space tab the default for JSON
    fmtster::JSONStyle style;
    style.tabCount = 4;
    F("{:,s,{},j}", mt(), style.value);

    // print list of customized colors using specialized template above
    cout << F("Colors: {}", colors) << endl;

    // restore original default style for JSON
    F("{:,s,{},j}", mt(), fmtster::JSONStyle{}.value);

    cout << "\n\n" << endl;

    // example of combining simple container/struct/class container contents
    // into the same JSON object

    // output opening bracket for re-arranged list of colors
    cout << "Colors: [\n";
    size_t count = colors.size();
    for (auto pr : colors)
    {
        // pr is a std::pair<string, vector<tuple<string, float> > >

        // manual use of a brace requires manual addition of the tab
        cout << "  {\n";

        // "-b" disables braces around the objects
        // 2 initial indents: 1 for the brace (above) + 1 more for the data inside
        cout << F("{:2,-b},\n", mp("name"s, std::get<0>(pr)));
        cout << F("{:2,-b}\n", std::get<1>(pr));

        // care must be taken to properly handle JSON commas
        cout << (--count ? "  },\n" : "  }\n");
    }

    // output closing bracket
    cout << "]" << endl;
}
