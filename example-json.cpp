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

struct Person
{
    string name;
    ex_time_point_t birthdate;
    float salary;
    vector<string> phones;
    map<string, string> family;
};



template<>
struct fmt::formatter<Person> : fmtster::FmtsterBase
{
    template<typename FormatContext>
    auto format(const Person& p, FormatContext& ctx)
    {
        // this could use the tuple approach shown for the opt example in
        //  main(), but this to illustrates use of the fmt::format_to() call
        //  and care that needs to be taken with JSON commas

        auto itOut = mDisableBras ?
                     ctx.out() :
                     fmt::format_to(ctx.out(), "{{\n");

        const string FMTSTR_NOCOMMA = F("{{:{},{},{},{}}}",
                                        mFormatSetting,
                                        mStyleHelper.mStyle.value,
                                        "",
                                        mIndentSetting);
        const string FMTSTR = FMTSTR_NOCOMMA + ",\n";
        itOut = format_to(itOut, FMTSTR, mp("name"s, p.name));

        // @@@ TODO: Wrap this section for use with C++20
        stringstream ss;
        auto t = ex_clock_t::to_time_t(p.birthdate);
        auto tm = *std::localtime(&t);
        ss << std::put_time(&tm, "%x");
        itOut = format_to(itOut, FMTSTR, mp("birthdate"s, ss.str()));

        itOut = format_to(itOut, FMTSTR, mp("salary"s, p.salary));
        itOut = format_to(itOut, FMTSTR, mp("phones"s, p.phones));
        itOut = format_to(itOut, FMTSTR_NOCOMMA, mp("family"s, p.family));

        if (!mDisableBras)
            itOut = fmt::format_to(itOut, "\n}}");

        return itOut;
    }
};

using Personnel = vector<Person>;

Personnel GetPersonnel()
{
    static std::tm tm1{ 0, 0, 0,
                        1, 1 - 1, 1970 - 1900 };
    static std::tm tm2{ 0, 0, 0,
                        31, 12 - 1, 1980 - 1900 };
    return Personnel
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

int main()
{
    vector<int> v = { 1, 2 };
//     cout << F("blanks: {}", v) << endl;
// cout << __LINE__ << endl;

//     cout << F("explicit default style as direct number: {:,0}", v) << endl;

fmtster::JSONStyle style;
style.tabCount = 8;
    cout << F("8 char tabs: {:,{}}", v, style.value) << endl;

//     cout << F("braceless: {:,,-b}", v) << endl;

//     cout << F("Indent 2 units: {:,,,2}", v) << endl;

//     // Based on https://json.org/example.html
//     auto GlossSeeAlso = vector<string>{ "GML", "XML" };
//     auto GlossDef = mt(mp("para"s, "A meta-markup language, used to create markup languages such as DocBook."s),
//                        mp("GlossSeeAlso"s, GlossSeeAlso));
//     auto GlossEntry = mt(mp("ID"s, "SGML"s),
//                          mp("SortAs"s, "SGML"s),
//                          mp("GlossTerm"s, "Standard Generalized Markup Language"s),
//                          mp("Acronym"s, "SGML"s),
//                          mp("Abbrev"s, "ISO 8879:1986"s),
//                          mp("GlossDef"s, GlossDef),
//                          mp("GlossSee"s, "markup"s));
//     auto GlossList = mp("GlossEntry"s, GlossEntry);
//     auto GlossDiv = mt(mp("title"s, "S"s),
//                        mp("GlossList"s, GlossList));
//     auto glossary = mt(mp("title"s, "example glossary"s),
//                        mp("GlossDiv"s, GlossDiv));
//     auto obj = mt(mp("glossary"s, glossary));
//     cout << F("{}", obj) << endl;


//     cout << "\n\n" << endl;


//     string buildAJSON = "{\n";

//     size_t personNumber = 0;
//     for (auto person : GetPersonnel())
//         cout << F("{}:\n{:,1,,1}\n", personNumber++, person) << endl;


//     cout << "\n\n" << endl;


//     fmtster::JSONStyle style;
//     style.config().tabCount = 4;
//     cout << F("style: {}\n\n", style) << endl;
//     map<string, int> msi = { { "one", 1 }, { "two" , 2 } };
//     try{ cout << F("{}", msi) << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }
//     try{ cout << F("{:0}", msi) << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }
//     try{ cout << F("{:j}", msi) << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }
//     try{ cout << F("{:J}", msi) << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }
//     try{ cout << F("{:json}", msi) << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }
//     try{ cout << F("{:JSON}", msi) << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }
//     try{ cout << F("{:1}", msi) << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }
//     try{ cout << F("{:xml}", msi) << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }
//     try{ cout << F("{:XML}", msi) << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }

//     try{ cout << F("{:{}}", msi, "j") << endl; } catch(fmt::format_error& ex) { cout << ex.what() << endl; }

// cout << "** " << __LINE__ << endl;
//     cout << F("{:,8}", msi) << endl;
// cout << "** " << __LINE__ << endl;
//     cout << F("{:,{}}", msi, 8) << endl;
// cout << "** " << __LINE__ << endl;
//     cout << F("{:,{},1,4}", msi, 8) << endl;
// cout << "** " << __LINE__ << endl;
//     cout << F("{:,{},,4}", msi, 8) << endl;
// cout << "** " << __LINE__ << endl;
//     cout << F("{:,{},,{}}", msi, 8, 4) << endl;
// cout << "** " << __LINE__ << endl;
//     cout << F("{:,{},{},{}}", msi, 8, 1, 4) << endl;
// cout << "** " << __LINE__ << endl;
// // not ready for prime time
// //     cout << F("{:{},{},{},{}}", msi, 0, -1, 0, 2) << endl;
// // cout << "** " << __LINE__ << endl;
}
