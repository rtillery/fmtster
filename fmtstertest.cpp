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

#include "fmtster.h"
using fmtster::F;

#include <gtest/gtest.h>

#include <iostream>
using std::cout;
using std::endl;

using fmtster::internal::is_adapter_v;
using fmtster::internal::is_mappish;
using fmtster::internal::is_mappish_v;
using fmtster::internal::is_multimappish;
using fmtster::internal::is_multimappish_v;
using fmtster::internal::fmtster_true;

#include <algorithm>
using std::find;
#include <array>
using std::array;
#include <vector>
using std::vector;
#include <forward_list>
using std::forward_list;
#include <list>
using std::list;
#include <stack>
using std::stack;
#include <queue>
using std::queue;
using std::priority_queue;
#include <deque>
using std::deque;
#include <set>
using std::set;
using std::multiset;
#include <unordered_set>
using std::unordered_set;
using std::unordered_multiset;

#include <map>
using std::map;
using std::multimap;
#include <unordered_map>
using std::unordered_map;
using std::unordered_multimap;

#include <string>
using std::string;
using namespace std::string_literals;

#include <tuple>
using std::tuple;
using std::make_tuple;

#include <type_traits>
using std::true_type;
using std::false_type;
using std::conjunction;
using std::conjunction_v;
using std::disjunction;
using std::disjunction_v;
using std::negation;
using std::enable_if_t;
using std::is_same_v;

#include <utility>
using std::declval;
using std::pair;
using std::make_pair;

/* test data */
template<typename T>
vector<T> GetValueContainerData();

template<>
vector<string> GetValueContainerData() { return { "one", "two", "three" }; }

template<>
vector<int> GetValueContainerData() { return { 1, 2, 3 }; }

template<>
vector<float> GetValueContainerData() { return { 1.1, 2.2, 3.3 }; }

template<>
vector<bool> GetValueContainerData() { return { false, true, false }; }

template<typename K, typename V>
map<K, V> GetKeyValueContainerData();

template<>
map<string, string> GetKeyValueContainerData()
{
    return map<string, string>
        {
            { "key1", "value1" },
            { "key2", "value2" },
            { "key3", "value3" }
        };
}

template<>
map<string, int> GetKeyValueContainerData()
{
    return {
              { "key1", 1 },
              { "key2", 2 },
              { "key3", 3 }
           };
}

template<>
map<string, float> GetKeyValueContainerData()
{
    return {
              { "key1", 1.1 },
              { "key2", 2.2 },
              { "key3", 3.3 }
           };
}

template<>
map<string, bool> GetKeyValueContainerData()
{
    return {
              { "key1", false },
              { "key2", true },
              { "key3", false }
           };
}

/* container checking macros */

fmtster_MAKEHASFN(emplace);
fmtster_MAKEHASFN(push_back);
fmtster_MAKEHASFN(push_front);
fmtster_MAKEHASFN(size);
fmtster_MAKEHASFN(begin);
fmtster_MAKEHASFN(hash_function);

fmtster_MAKEIS(keyval_container, (disjunction_v<is_mappish<T>, is_multimappish<T> >));

/* container creation templates */

// template<typename C, typename = void>
// C CreateContainer();

template<typename C>
enable_if_t<conjunction_v<has_push_back<C, typename C::value_type>,
                          negation<is_mappish<C> > >,
            C> CreateContainer()
{
    C cData;
    for (auto e : GetValueContainerData<typename C::value_type>())
        cData.push_back(e);
    return cData;
}

template<typename C>
enable_if_t<conjunction_v<negation<has_push_back<C, typename C::value_type> >,
                          has_emplace<C, typename C::value_type>,
                          negation<is_keyval_container<C> > >,
                 C> CreateContainer()
{
    C cData;
    for (auto e : GetValueContainerData<typename C::value_type>())
        cData.emplace(e);
    return cData;
}

template<typename C>
enable_if_t<conjunction_v<negation<has_push_back<C, typename C::value_type> >,
                          negation<has_emplace<C, typename C::value_type> >,
                          has_push_front<C, typename C::value_type>,
                          negation<is_keyval_container<C> > >,
                 C> CreateContainer()
{
    C cData;
    for (auto e : GetValueContainerData<typename C::value_type>())
        cData.push_front(e);
    return cData;
}

template<class T>
struct is_array : false_type
{};
template<class T, std::size_t N>
struct is_array<array<T,N> > : true_type
{};
template<typename ...Ts>
inline constexpr bool is_array_v = is_array<Ts...>::value;

template<typename C>
enable_if_t<is_array_v<C>, C> CreateContainer()
{
    C cData;
    size_t i = 0;
    for (auto e : GetValueContainerData<typename C::value_type>())
        cData[i++] = e;
    return cData;
}

template<typename C>
enable_if_t<is_mappish_v<C>, C> CreateContainer()
{
    C cData;
    for (auto& [key, val] : GetKeyValueContainerData<typename C::key_type, typename C::mapped_type>())
        cData.emplace(key, val);
    return cData;
}

template<typename C>
enable_if_t<is_multimappish_v<C>, C> CreateContainer()
{
    C cData;
    for (auto& [key, val] : GetKeyValueContainerData<typename C::key_type, typename C::mapped_type>())
        cData.emplace(key, val);
    return cData;
}

/* reference creation templates */

// some single item element containers have a size member
template<typename C>
enable_if_t<conjunction_v<has_size<C>,
                          has_begin<C>,
                          negation<is_keyval_container<C> > >,
                 string> GetReference(const C& data)
{
    const string strFmt(is_same_v<typename C::value_type, string> ?
                        "\n  \"{}\"{}" :
                        "\n  {}{}");
    string ref(data.empty() ? "[ " : "[");
    auto len = data.size();
    for (auto e : data)
        ref += F(strFmt, e, --len ? "," : "");
    if (!data.empty())
        ref += "\n";
    ref += "]";
    return ref;
}

// some single item element containers have no size member
template<typename C>
enable_if_t<conjunction_v<negation<has_size<C> >,
                          negation<is_keyval_container<C> > >,
                 string> GetReference(const C& data)
{
    const string strFmt(is_same_v<typename C::value_type, string>
                        ? "\n  \"{}\"{}"
                        : "\n  {}{}");
    string ref(data.empty() ? "[ " : "[");
    auto it = data.begin();
    while (it != data.end())
    {
        auto itThis = it;
        it++;
        ref += F(strFmt, *itThis, (it != data.end()) ? "," : "");
    }
    if (!data.empty())
        ref += "\n";
    ref += "]";
    return ref;
}

template<typename C>
enable_if_t<conjunction_v<has_size<C>,
                          has_begin<C>,
                          is_mappish<C> >,
                 string> GetReference(const C& data)
{
    const string strFmt(is_same_v<typename C::mapped_type, string>
                        ? "\n  \"{}\" : \"{}\"{}"
                        : "\n  \"{}\" : {}{}");
    string ref(data.empty() ? "{ " : "{");
    auto len = data.size();
    for (auto [key, val] : data)
        ref += F(strFmt, key, val, --len ? "," : "");
    if (!data.empty())
        ref += "\n";
    ref += "}";
    return ref;
}

template<typename C>
enable_if_t<conjunction_v<has_size<C>,
                          has_begin<C>,
                          is_multimappish<C> >,
            string> GetReference(const C& data)
{
    using mapped_type = typename C::mapped_type;

    if (is_same_v<mapped_type, string>)
    {
        if (has_hash_function_v<C>)
            return
                "{\n  \"key3\" : [\n"
                "    \"value3\"\n"
                "  ],\n"
                "  \"key2\" : [\n"
                "    \"value2\"\n"
                "  ],\n"
                "  \"key1\" : [\n"
                "    \"value1\"\n"
                "  ]\n"
                "}";
        else
            return
                "{\n  \"key1\" : [\n"
                "    \"value1\"\n"
                "  ],\n"
                "  \"key2\" : [\n"
                "    \"value2\"\n"
                "  ],\n"
                "  \"key3\" : [\n"
                "    \"value3\"\n"
                "  ]\n"
                "}";
    }
    else if (is_same_v<mapped_type, bool>)
    {
        if (has_hash_function_v<C>)
            return
                "{\n"
                "  \"key3\" : [\n"
                "    false\n"
                "  ],\n"
                "  \"key2\" : [\n"
                "    true\n  ],\n"
                "  \"key1\" : [\n"
                "    false\n"
                "  ]\n"
                "}";
        else
            return
                "{\n"
                "  \"key1\" : [\n"
                "    false\n"
                "  ],\n"
                "  \"key2\" : [\n"
                "    true\n  ],\n"
                "  \"key3\" : [\n"
                "    false\n"
                "  ]\n"
                "}";
    }
    else if (is_same_v<mapped_type, int>)
    {
        if (has_hash_function_v<C>)
            return
                "{\n"
                "  \"key3\" : [\n"
                "    3\n"
                "  ],\n"
                "  \"key2\" : [\n"
                "    2\n"
                "  ],\n"
                "  \"key1\" : [\n"
                "    1\n"
                "  ]\n"
                "}";
        else
            return
                "{\n"
                "  \"key1\" : [\n"
                "    1\n"
                "  ],\n"
                "  \"key2\" : [\n"
                "    2\n"
                "  ],\n"
                "  \"key3\" : [\n"
                "    3\n"
                "  ]\n"
                "}";
    }
    else if (is_same_v<mapped_type, float> || is_same_v<mapped_type, double>)
    {
        if (has_hash_function_v<C>)
            return
                "{\n"
                "  \"key3\" : [\n"
                "    3.3\n"
                "  ],\n"
                "  \"key2\" : [\n"
                "    2.2\n"
                "  ],\n"
                "  \"key1\" : [\n"
                "    1.1\n"
                "  ]\n"
                "}";
        else
            return
                "{\n"
                "  \"key1\" : [\n"
                "    1.1\n"
                "  ],\n"
                "  \"key2\" : [\n"
                "    2.2\n"
                "  ],\n"
                "  \"key3\" : [\n"
                "    3.3\n"
                "  ]\n"
                "}";
    }
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

template<typename C>
enable_if_t<is_adapter_v<C>,
            string> GetReference(const C& data)
{
    return GetReference(GetAdapterContainer(data));
}

/* test macros */

#define VALUECONTAINERTEST_INTERNAL(DATA) \
{ \
    auto data = DATA; \
    string str = F("{}", data); \
cout << str << endl; \
    auto ref = GetReference(data); \
/* cout << ref << endl; */ \
    EXPECT_EQ(ref, str) << F("ref: {}\nstr: {}", ref, str); \
}

#define ARRAYCONTAINERTEST(TYPE) \
TEST_F(FmtsterTest, array_of_ ## TYPE ## s_to_JSON) \
VALUECONTAINERTEST_INTERNAL((CreateContainer<array<TYPE, 3> >()))

#define EMPTYARRAYCONTAINERTEST(TYPE) \
TEST_F(FmtsterTest, empty_array_of_ ## TYPE ## s_to_JSON) \
VALUECONTAINERTEST_INTERNAL((array<TYPE, 0>{}))

#define VALUECONTAINERTEST(CONT, TYPE) \
TEST_F(FmtsterTest, CONT ## _of_ ## TYPE ## s_to_JSON) \
VALUECONTAINERTEST_INTERNAL(CreateContainer<CONT<TYPE> >())

#define EMPTYVALUECONTAINERTEST(CONT, TYPE) \
TEST_F(FmtsterTest, empty_ ## CONT ## _of_ ## TYPE ## s_to_JSON) \
VALUECONTAINERTEST_INTERNAL(CONT<TYPE>{})

#define KEYVALUECONTAINERTEST(CONT, KEYTYPE, VALTYPE) \
TEST_F(FmtsterTest, CONT ## _of_ ## KEYTYPE ## s_to_ ## VALTYPE ## s_to_JSON) \
VALUECONTAINERTEST_INTERNAL((CreateContainer<CONT<KEYTYPE,VALTYPE> >()))

#define ARRAYCONTAINERTESTS() \
EMPTYARRAYCONTAINERTEST(string) \
ARRAYCONTAINERTEST(string) \
EMPTYARRAYCONTAINERTEST(int) \
ARRAYCONTAINERTEST(int) \
EMPTYARRAYCONTAINERTEST(float) \
ARRAYCONTAINERTEST(float) \
EMPTYARRAYCONTAINERTEST(bool) \
ARRAYCONTAINERTEST(bool)

#define VALUECONTAINERTESTS(CONT) \
EMPTYVALUECONTAINERTEST(CONT, string) \
VALUECONTAINERTEST(CONT, string) \
EMPTYVALUECONTAINERTEST(CONT, int) \
VALUECONTAINERTEST(CONT, int) \
EMPTYVALUECONTAINERTEST(CONT, float) \
VALUECONTAINERTEST(CONT, float) \
EMPTYVALUECONTAINERTEST(CONT, bool) \
VALUECONTAINERTEST(CONT, bool)

#define KEYVALUECONTAINERTESTS(CONT) \
KEYVALUECONTAINERTEST(CONT, string, string) \
KEYVALUECONTAINERTEST(CONT, string, int) \
KEYVALUECONTAINERTEST(CONT, string, float) \
KEYVALUECONTAINERTEST(CONT, string, bool)

/* test object */

class FmtsterTest : public ::testing::Test
{
};


/*
 *
 * tests
 *
 */

// some simple reference output (test does not fail)
TEST_F(FmtsterTest, Reference)
{
    float fSmall = 3.1415926535897932384626;
    float fLarge = 6.0234567e17;
    float fWhole = 2.000000000;

    cout << F("float: {}", fSmall) << endl;
    cout << F("float: {}", fLarge) << endl;
    cout << F("float: {}", fWhole) << endl;

    double dSmall = 3.1415926535897932384626;
    double dLarge = 6.0234567e17;
    double dWhole = 2.000000000;

    cout << F("double: {}", dSmall) << endl;
    cout << F("double: {}", dLarge) << endl;
    cout << F("double: {}", dWhole) << endl;

    cout << F("bool: {}", false) << endl;
    cout << F("bool: {}", true) << endl;

    cout << F("null: {}", nullptr) << endl;

    string s1 = "abc\0\0def";
    string s2 = "abc\0\0def"s;
    cout << "s1: " << s1.size() << " \"" << s1 << "\"\n";
    cout << "s2: " << s2.size() << " \"" << s2 << "\"" << endl;

    cout << '\n' << endl;
}

std::string ReplaceString(std::string subject,
                          const std::string& search,
                          const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

TEST_F(FmtsterTest, JSONStyle)
{
    // initial default style
    string REF =
        "{\n"
        "  \"cr\" : false,\n"
        "  \"lf\" : true,\n"
        "  \"hardTab\" : false,\n"
        "  \"tabCount\" : 2,\n"
        "  \"gapA\" : 14,\n"
        "  \"gapB\" : 2,\n"
        "  \"gapC\" : 2,\n"
        "  \"gap1\" : 14,\n"
        "  \"gap2\" : 0,\n"
        "  \"gap3\" : 2,\n"
        "  \"gap4\" : 14,\n"
        "  \"gap5\" : 0,\n"
        "  \"gap6\" : 2,\n"
        "  \"gap7\" : 8,\n"
        "  \"emptyArray\" : 2,\n"
        "  \"emptyObject\" : 2,\n"
        "  \"sva\" : 1,\n"
        "  \"svo\" : 1\n"
        "}";
    fmtster::JSONStyle style;
    string str;
    str = F("{}", style);
    ASSERT_EQ(REF, str) << F("REF JSONStyle: {},\ninitial default JSONStyle: {}", REF, str);

    // current default should match initial default
    str = F("{}", fmtster::FmtsterBase::GetDefaultJSONStyle());
    ASSERT_EQ(REF, str) << F("REF JSONStyle: {},\ncurrent default str: {}", REF, str);

    // change style object & reference to use single hard tabs & compare (print with default)
    style.hardTab = true;
    style.tabCount = 1;
    REF = ReplaceString(REF, "\"hardTab\" : false", "\"hardTab\" : true");
    REF = ReplaceString(REF, "\"tabCount\" : 2", "\"tabCount\" : 1");
    str = F("{}", style);
    ASSERT_EQ(REF, str) << F("REF JSONStyle: {},\ncurrent default str: {}", REF, str);

    // change reference string to use hard tabs
    REF = ReplaceString(REF, "  ", "\t");
    str = F("{:,{}}", style, style.value);
    ASSERT_EQ(REF, str) << F("REF JSONStyle: {},\ncurrent default str: {}", REF, str);

    // make style object new default
    F("{:,{},s}", make_tuple(), style.value);
    ASSERT_EQ(style.value, fmtster::FmtsterBase::GetDefaultJSONStyle().value);

    // print current default (with current default style) & compare to reference
    str = F("{}", fmtster::FmtsterBase::GetDefaultJSONStyle());
    ASSERT_EQ(REF, str) << F("REF JSONStyle: {},\ncurrent default str: {}", REF, str);

    // return current default to initial default for following tests
    str = F("{:,{},s}", make_tuple(), 0);
    ASSERT_EQ(fmtster::JSONStyle{}.value, fmtster::FmtsterBase::GetDefaultJSONStyle().value) << F("current default JSONStyle: {},\ninitial default str: {}", REF, str);
}

// value container tests

ARRAYCONTAINERTESTS()
VALUECONTAINERTESTS(vector)
VALUECONTAINERTESTS(forward_list)
VALUECONTAINERTESTS(list)
VALUECONTAINERTESTS(deque)
VALUECONTAINERTESTS(set)
VALUECONTAINERTESTS(unordered_set)
VALUECONTAINERTESTS(multiset)
VALUECONTAINERTESTS(unordered_multiset)
VALUECONTAINERTESTS(stack)
VALUECONTAINERTESTS(queue)
VALUECONTAINERTESTS(priority_queue)

array<string, 3> CreateContainer()
{
    array<string, 3> a3;

    return a3;
}

// key/value container tests

KEYVALUECONTAINERTESTS(map)
KEYVALUECONTAINERTESTS(unordered_map)
KEYVALUECONTAINERTESTS(multimap)
KEYVALUECONTAINERTESTS(unordered_multimap)

TEST_F(FmtsterTest, map_of_maps_of_strings_to_strings_to_JSON)
{
    map<string, map<string, string> > mapofmapofstrings =
    {
        {
            "map1",
            {
                { "entry1", "value1" },
                { "entry2", "value2" }
            }
        },
        {
            "map2",
            {
                { "entry3", "value3" },
                { "entry4", "value4" }
            }
        }
    };
    const string ref =
        "{\n"
        "  \"map1\" : {\n"
        "    \"entry1\" : \"value1\",\n"
        "    \"entry2\" : \"value2\"\n"
        "  },\n"
        "  \"map2\" : {\n"
        "    \"entry3\" : \"value3\",\n"
        "    \"entry4\" : \"value4\"\n"
        "  }\n"
        "}";
    string str = F("{}", mapofmapofstrings);
cout << str << endl;
    EXPECT_EQ(ref, str);
}

TEST_F(FmtsterTest, map_of_maps_of_strings_to_strings_to_JSON_4SpaceTab)
{
    map<string, map<string, string> > mapofmapofstrings =
    {
        {
            "map1",
            {
                { "entry1", "value1" },
                { "entry2", "value2" }
            }
        },
        {
            "map2",
            {
                { "entry3", "value3" },
                { "entry4", "value4" }
            }
        }
    };
    const string ref =
        "{\n"
        "    \"map1\" : {\n"
        "        \"entry1\" : \"value1\",\n"
        "        \"entry2\" : \"value2\"\n"
        "    },\n"
        "    \"map2\" : {\n"
        "        \"entry3\" : \"value3\",\n"
        "        \"entry4\" : \"value4\"\n"
        "    }\n"
        "}";
    fmtster::JSONStyle style;
    style.tabCount = 4;
    string str = F("{:,{}}", mapofmapofstrings, style.value);
cout << str << endl;
    EXPECT_EQ(ref, str);
}

TEST_F(FmtsterTest, map_of_vectors_of_strings_to_JSON)
{
//     F("{}", fmtster::JSONStyle{});

    map<string, vector<string> > mapofvectorofstrings =
    {
        {
            "vec1",
            {
                "vec1val1",
                "vec1val2"
            }
        },
        {
            "vec2",
            {
                "vec2val1",
                "vec2val2"
            }
        }
    };
    const string ref =
        "{\n"
        "  \"vec1\" : [\n"
        "    \"vec1val1\",\n"
        "    \"vec1val2\"\n"
        "  ],\n"
        "  \"vec2\" : [\n"
        "    \"vec2val1\",\n"
        "    \"vec2val2\"\n"
        "  ]\n"
        "}";
    string str = F("{}", mapofvectorofstrings);
cout << str << endl;
    EXPECT_EQ(ref, str);
}

TEST_F(FmtsterTest, pairs)
{
    string ref = "{\n  \"foo\" : \"bar\"\n}, {\n  \"foobar\" : 7\n}";
    string str = F("{}, {}", make_pair("foo"s, "bar"s), make_pair("foobar"s, 7));
cout << "ONE:\n" << str << endl;
    EXPECT_EQ(ref, str);

    ref = "\"foo\" : \"bar\", \"foobar\" : 7";
    str = F("{:,,-b}, {:,,-b}", make_pair("foo"s, "bar"s), make_pair("foobar"s, 7));
cout << "TWO:\n" << str << endl;
    EXPECT_EQ(ref, str);

    ref = "{\n    \"foo\" : \"bar\"\n  }, {\n    \"foobar\" : 7\n  }";
    str = F("{:,,,1}, {:,,,1}", make_pair("foo"s, "bar"s), make_pair("foobar"s, 7));
cout << "THREE:\n" << str << endl;
    EXPECT_EQ(ref, str);

    ref = "  \"foo\" : \"bar\", \"foobar\" : 7";
    str = F("{:,,-b,1}, {:,,-b}", make_pair("foo"s, "bar"s), make_pair("foobar"s, 7));
cout << "FOUR:\n" << str << endl;
    EXPECT_EQ(ref, str);
}

TEST_F(FmtsterTest, custom_indent_pairs)
{
    const string ref = "    \"fu\" : \"baz\", \"fubar\" : 3.14";
    fmtster::JSONStyle style;
    style.tabCount = 4;
    string str = F("{:,{},-b,1}, {:,,-b}", make_pair("fu"s, "baz"s), style.value, make_pair("fubar"s, 3.14));
cout << str << endl;
    EXPECT_EQ(ref, str);
}

TEST_F(FmtsterTest, NestedPairs)
{
    string ref =
        "\"foo\" : {\n"
        "    \"bar\" : \"baz\"\n"
        "}";
    fmtster::JSONStyle style;
    style.tabCount = 4;
    string str = F("{:,{},-b}", make_pair("foo"s, make_pair("bar"s, "baz"s)), style.value);
cout << str << endl;
    EXPECT_EQ(ref, str);

    ref =
        "    \"foo\" : {\n"
        "        \"bar\" : \"baz\"\n"
        "    }";
    str = F("{:,{},-b,1}", make_pair("foo"s, make_pair("bar"s, "baz"s)), style.value);
cout << str << endl;
    EXPECT_EQ(ref, str);

    map<string, vector<string> > mapofvectorofstrings =
    {
        {
            "vec1",
            {
                "vec1val1",
                "vec1val2"
            }
        },
        {
            "vec2",
            {
                "vec2val1",
                "vec2val2"
            }
        }
    };

    ref =
        "\"foo\" : {\n"
        "    \"bar\" : {\n"
        "        \"vec1\" : [\n"
        "            \"vec1val1\",\n"
        "            \"vec1val2\"\n"
        "        ],\n"
        "        \"vec2\" : [\n"
        "            \"vec2val1\",\n"
        "            \"vec2val2\"\n"
        "        ]\n"
        "    }\n"
        "}";
    str = F("{:,{},-b}", make_pair("foo"s, make_pair("bar"s, mapofvectorofstrings)), style.value);
cout << str << endl;
    EXPECT_EQ(ref, str);

    ref =
    "    \"foo\" : {\n"
    "        \"bar\" : {\n"
    "            \"vec1\" : [\n"
    "                \"vec1val1\",\n"
    "                \"vec1val2\"\n"
    "            ],\n"
    "            \"vec2\" : [\n"
    "                \"vec2val1\",\n"
    "                \"vec2val2\"\n"
    "            ]\n"
    "        }\n"
    "    }";
    str = F("{:,{},-b,1}", make_pair("foo"s, make_pair("bar"s, mapofvectorofstrings)), style.value);
cout << str << endl;
    EXPECT_EQ(ref, str);
}

TEST_F(FmtsterTest, EscapedValues)
{
//     F("{}", fmtster::JSONStyle{});

    map<string, vector<string> > mapofvectorofstrings =
    {
        {
            R"(vec\1)",
            {
                R"(\vec"1/\val/1)",
                "vec\b1\x7Fval\n2" R"(\)"
            }
        },
        {
            "vec\xA5" "2",  // Two tricks to insert ¥ char (ASCII 0xA5/165):
                            // 1. Char itself is converted to unicode (at least
                            //    by my editor), so hex escape used.
                            // 2. \xA52 is misinterpreted by gcc as three digit
                            //    value, even though only two digits should be
                            //    read for \x prefix:
                            //    https://en.cppreference.com/w/cpp/language/escape
            {
                R"(vec"2'val)" "\t1",
                "vec\f2\tval\r2"
            }
        }
    };
    const string ref =
        "{\n"
        R"(  "vec\\1" : [)" "\n"
        R"(    "\\vec\"1\/\\val\/1",)" "\n"
        R"(    "vec\b1\u007Fval\n2\\")" "\n"
        "  ],\n"
        R"(  "vec\u00A52" : [)" "\n"
        R"(    "vec\"2'val\t1",)" "\n"
        R"(    "vec\f2\tval\r2")" "\n"
        "  ]\n"
        "}";
    string str = F("{}", mapofvectorofstrings);
cout << str << endl;
    EXPECT_EQ(ref, str);
}

TEST_F(FmtsterTest, Tuple)
{
    const auto tup = make_tuple(
        make_pair("int"s, 25),
        make_pair("string"s, "Hello"s),
        make_pair("float"s, 9.31f),
        make_pair("vector"s, vector<int>{3, 1, 4}),
        make_pair("boolean"s, true));
    fmtster::JSONStyle style;
    style.tabCount = 4;
   string str = F("{:,{},-b,1}", tup, style.value);
    string ref =
        R"(    "int" : 25,)" "\n"
        R"(    "string" : "Hello",)" "\n"
        R"(    "float" : 9.31,)" "\n"
        R"(    "vector" : [)" "\n"
        "        3,\n"
        "        1,\n"
        "        4\n"
        "    ],\n"
        R"(    "boolean" : true)";
cout << str << endl;
    EXPECT_EQ(ref, str);

    str = F("{:,{},,1}", tup, style.value);
    ref =
        "{\n"
        R"(        "int" : 25,)" "\n"
        R"(        "string" : "Hello",)" "\n"
        R"(        "float" : 9.31,)" "\n"
        R"(        "vector" : [)" "\n"
        "            3,\n"
        "            1,\n"
        "            4\n"
        "        ],\n"
        R"(        "boolean" : true)" "\n"
        "    }";
cout << str << endl;
    EXPECT_EQ(ref, str);
}

TEST_F(FmtsterTest, Layers)
{
//     F("{}", fmtster::JSONStyle{});

    // pair
    auto pr = make_pair("key"s, "value"s);
    string str = F("{}", pr);
    EXPECT_EQ("{\n  \"key\" : \"value\"\n}"s, str);
    str = F("{:,,-b}", pr);
    EXPECT_EQ("\"key\" : \"value\""s, str);
    str = F("{:,,,1}", pr);
    EXPECT_EQ("{\n    \"key\" : \"value\"\n  }"s, str);
    str = F("{:,,-b,1}", pr);
    EXPECT_EQ("  \"key\" : \"value\""s, str);
    // tuple
    auto tup = make_tuple("string"s, 1, true, 3.14);
    str = F("{}", tup);
    EXPECT_EQ("{\n  \"string\",\n  1,\n  true,\n  3.14\n}"s, str);
    str = F("{:,,-b}", tup);
    EXPECT_EQ("\"string\",\n1,\ntrue,\n3.14"s, str);
    str = F("{:,,,1}", tup);
    EXPECT_EQ("{\n    \"string\",\n    1,\n    true,\n    3.14\n  }"s, str);
    str = F("{:,,-b,1}", tup);
    EXPECT_EQ("  \"string\",\n  1,\n  true,\n  3.14"s, str);
    auto mm2 = multimap<string, map<string, int> >{ { "mm1"s, { { "one"s, 1 }, { "two"s, 2 }, { "three"s, 3 } } },
                                                    { "mm2"s, { { "four"s, 4 }, { "five"s, 5 } } },
                                                    { "mm1"s, { { "six"s, 6 }, { "seven"s, 7 } } } };
    str = F("{}", mm2);
    EXPECT_EQ("{\n  \"mm1\" : [\n    {\n      \"seven\" : 7,\n      \"six\" : 6\n    },\n    {\n      \"one\" : 1,\n      \"three\" : 3,\n      \"two\" : 2\n    }\n  ],\n  \"mm2\" : [\n    {\n      \"five\" : 5,\n      \"four\" : 4\n    }\n  ]\n}"s,
              str);
    str = F("{:,,-b}", mm2);
    EXPECT_EQ("\"mm1\" : [\n  {\n    \"seven\" : 7,\n    \"six\" : 6\n  },\n  {\n    \"one\" : 1,\n    \"three\" : 3,\n    \"two\" : 2\n  }\n],\n\"mm2\" : [\n  {\n    \"five\" : 5,\n    \"four\" : 4\n  }\n]"s,
              str);
    str = F("{:,,,1}", mm2);
    EXPECT_EQ("{\n    \"mm1\" : [\n      {\n        \"seven\" : 7,\n        \"six\" : 6\n      },\n      {\n        \"one\" : 1,\n        \"three\" : 3,\n        \"two\" : 2\n      }\n    ],\n    \"mm2\" : [\n      {\n        \"five\" : 5,\n        \"four\" : 4\n      }\n    ]\n  }"s,
              str);
    str = F("{:,,-b,1}", mm2);
    EXPECT_EQ("  \"mm1\" : [\n    {\n      \"seven\" : 7,\n      \"six\" : 6\n    },\n    {\n      \"one\" : 1,\n      \"three\" : 3,\n      \"two\" : 2\n    }\n  ],\n  \"mm2\" : [\n    {\n      \"five\" : 5,\n      \"four\" : 4\n    }\n  ]"s,
              str);
}
