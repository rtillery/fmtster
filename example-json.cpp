/* Copyright (c) 2021 Harman International Industries, Incorporated.  All rights
 * reserved.
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
using std::chrono::time_point;
using std::chrono::high_resolution_clock;
using std::chrono::nanoseconds;
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
#include <variant>
using std::variant;
#include <vector>
using std::vector;

#include "fmtster.h"
using fmtster::F;


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
    auto format(const Person1& p, FormatContext& ctx) const
    {
        // direct approach (take special care with commas and carriage returns!)

        auto& d = *mpData; // alias helper for accessing data

        resolveArgs(ctx);

        auto itFC = ctx.out();

        const auto indent = d.mDisableBras ? d.mBraIndent : d.mDataIndent;

        // output opening brace (if enabled)
        if (!d.mDisableBras)
        {
            itFC = format_to(itFC, "{{\n");
            d.mIndentSetting++;
        }

        itFC = format_to(itFC,
                         "{:{},{},{},{}},\n",
                         mp("name", p.name),
                         d.mIndentSetting,
                         "-b",
                         d.mStyleValue,
                         d.mFormatSetting);

        // @@@ TODO: Wrap this section for use with C++20
        stringstream ss;
        auto t = ex_clock_t::to_time_t(p.birthdate);
        auto tm = *std::localtime(&t);
        ss << std::put_time(&tm, "%x");

        itFC = format_to(itFC,
                         "{:{},{},{},{}},\n",
                         mp("birthdate", ss.str()),
                         d.mIndentSetting,
                         "-b",
                         d.mStyleValue,
                         d.mFormatSetting);
        itFC = format_to(itFC,
                         "{:{},{},{},{}},\n",
                         mp("salary", p.salary),
                         d.mIndentSetting,
                         "-b",
                         d.mStyleValue,
                         d.mFormatSetting);
        itFC = format_to(itFC,
                         "{:{},{},{},{}},\n",
                         mp("phones", p.phones),
                         d.mIndentSetting,
                         "-b",
                         d.mStyleValue,
                         d.mFormatSetting);

        // Note this entry's format string does not end a comma
        itFC = format_to(itFC,
                         "{:{},{},{},{}}",
                         mp("family", p.family),
                         d.mIndentSetting,
                         "-b",
                         d.mStyleValue,
                         d.mFormatSetting);

        if (!d.mDisableBras)
            itFC = fmt::format_to(itFC, "\n{}}}", d.mBraIndent);

        return itFC;
    }
};

template<>
struct fmt::formatter<Person2>
  : fmtster::Base
{
    template<typename FormatContext>
    auto format(const Person2& p, FormatContext& ctx) const
    {
        auto& d = *mpData; // alias helper for accessing data

        // tuple approach

        resolveArgs(ctx);

        // @@@ TODO: Wrap this section for use with C++20
        stringstream ss;
        auto t = ex_clock_t::to_time_t(p.birthdate);
        auto tm = *std::localtime(&t);
        ss << std::put_time(&tm, "%x");

        auto tup = mt(
            mp("name", p.name),
            mp("birthdate", ss.str()),
            mp("salary", p.salary),
            mp("phones", p.phones),
            mp("family", p.family)
        );
        return format_to(ctx.out(),
                         "{:{},{},{},{}}",
                         tup,
                         d.mIndentSetting,
                         d.mDisableBras ? "-b" : "",
                         d.mStyleValue,
                         d.mFormatSetting);
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
    auto format(const Color& color, FormatContext& ctx) const
    {
        auto& d = *mpData; // alias helper for accessing data

        using std::make_pair;

        resolveArgs(ctx);

        auto tup = std::make_tuple(
            make_pair("hue", color.hue),
            make_pair("primaries", color.primaries)
        );
        return format_to(ctx.out(),
                         "{:{},{},{},{}}",
                         tup,
                         d.mIndentSetting,
                         d.mDisableBras ? "-b" : "",
                         d.mStyleValue,
                         d.mFormatSetting);
    }
};

class Benchmark
{
    const high_resolution_clock::time_point mStart;
    const string mFn;

public:
    Benchmark(const string& fn) :
        mStart(high_resolution_clock::now()),
        mFn(fn)
    {}
    ~Benchmark()
    {
        const auto dur = high_resolution_clock::now() - mStart;
        const double dbl = (double)dur.count() / 1000000000;
        cout << F(">>>>>>>> {:0.9f} secs: {}() <<<<<<<<", dbl, mFn) << endl;
    }
};

// #define BENCHMARK
#define BENCHMARK Benchmark bench(__func__)

using BoolMap_t = map<string, bool>;

string Serialize_BoolMap_Defaults(const BoolMap_t& boolmap)
{
    string str;
    {
        BENCHMARK;
        for (int i = 100000; i; --i)
            str = F("{}", boolmap);
    }
    return str;
}

string Serialize_BoolMap_EightCharStyleAndDefault(const BoolMap_t& boolmap)
{
    fmtster::JSONStyle eightCharStyle;
    eightCharStyle.tabCount = 8;
    fmtster::JSONStyle initialStyle;

    string str;
    {
        BENCHMARK;
        for (int i = 100000; i; --i)
        {
            str = F("{:,,{},j}", boolmap, eightCharStyle.value);
            str = F("{:,,{},j}", boolmap, initialStyle.value);
        }
    }
    return str;
}

string Serialize_BoolMap_EightCharAndHardTabAndDefault(const BoolMap_t& boolmap)
{
    fmtster::JSONStyle eightCharStyle;
    eightCharStyle.tabCount = 8;
    fmtster::JSONStyle hardTabStyle;
    hardTabStyle.hardTab = true;
    fmtster::JSONStyle initialStyle;

    string str;
    {
        BENCHMARK;
        for (int i = 100000; i; --i)
        {
            str = F("{:,,{},j}", boolmap, eightCharStyle.value);
            str = F("{:,,{},j}", boolmap, hardTabStyle.value);
            str = F("{:,,{},j}", boolmap, initialStyle.value);
        }
    }
    return str;
}

int main()
{
    // Benchmarks
    map<string, bool> boolmap =
    {
        { "true", true }, { "false", false }, { "maybe", false }
    };

    string str = Serialize_BoolMap_Defaults(boolmap);
    str = Serialize_BoolMap_EightCharStyleAndDefault(boolmap);
    str = Serialize_BoolMap_EightCharAndHardTabAndDefault(boolmap);
 
    // Based on https://json.org/example.html
    auto GlossSeeAlso = vector<string>{ "GML", "XML" };
    auto GlossDef = mt(mp("para", "A meta-markup language, used to create "
                                  "markup languages such as DocBook."),
                       mp("GlossSeeAlso", GlossSeeAlso));
    auto GlossEntry = mt(mp("ID", "SGML"),
                         mp("SortAs", "SGML"),
                         mp("GlossTerm",
                            "Standard Generalized Markup Language"),
                         mp("Acronym", "SGML"),
                         mp("Abbrev", "ISO 8879:1986"),
                         mp("GlossDef", GlossDef),
                         mp("GlossSee", "markup"));
    auto GlossList = mp("GlossEntry", GlossEntry);
    auto GlossDiv = mt(mp("title", "S"),
                       mp("GlossList", GlossList));
    auto glossary = mt(mp("title", "example glossary"),
                       mp("GlossDiv", GlossDiv));
    auto obj = mt(mp("glossary", glossary));
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
        cout << F("{:2,-b},\n", mp("name", std::get<0>(pr)));
        cout << F("{:2,-b}\n", std::get<1>(pr));

        // care must be taken to properly handle JSON commas
        cout << (--count ? "  },\n" : "  }\n");
    }

    // output closing bracket
    cout << "]" << endl;
}
