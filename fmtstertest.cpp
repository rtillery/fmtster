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

#include <iostream>
using std::cout;
using std::endl;

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
using std::void_t;

#include <gtest/gtest.h>

#include "fmtster.h"
using fmtster::F;

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

// Based on https://stackoverflow.com/a/28309612/3705286
//
#define MAKEHAS(FN) \
template <typename T, typename ...Args> \
class has_ ## FN \
{ \
    template <typename C, \
              typename = decltype( std::declval<C>().FN(std::declval<Args>()...) )> \
    static std::true_type test(int); \
    template <typename C> \
    static std::false_type test(...); \
\
public: \
    static constexpr bool value = decltype(test<T>(0))::value; \
    using type = T; \
}; \
\
template<typename T, typename ...Args> \
using has_ ## FN ## _t = typename has_ ## FN<T, Args...>::type;
//
/////////////////////////////////////////////////////////

MAKEHAS(emplace);
MAKEHAS(push_back);
MAKEHAS(push_front);
MAKEHAS(size);
MAKEHAS(begin);

template<typename T, typename = void>
struct is_adapter : std::false_type
{};

template<typename T>
struct is_adapter<T, void_t<typename T::container_type> >
    : std::true_type
{};

template<typename T, typename = void>
struct is_keyval_container : std::false_type
{};

template<typename T>
struct is_keyval_container<T, void_t<typename T::mapped_type> >
    : std::true_type
{};

/* container creation templates */

// template<typename C, typename = void>
// C CreateContainer();

template<typename C>
std::enable_if_t<has_push_back<C, typename C::value_type>::value
                 && !is_keyval_container<C>::value,
                 C> CreateContainer()
{
    C cData;
    for (auto e : GetValueContainerData<typename C::value_type>())
        cData.push_back(e);
    return cData;
}

template<typename C>
std::enable_if_t<!has_push_back<C, typename C::value_type>::value
                 && has_emplace<C, typename C::value_type>::value
                 && !is_keyval_container<C>::value,
                 C> CreateContainer()
{
    C cData;
    for (auto e : GetValueContainerData<typename C::value_type>())
        cData.emplace(e);
    return cData;
}

template<typename C>
std::enable_if_t<!has_push_back<C, typename C::value_type>::value
                 && !has_emplace<C, typename C::value_type>::value
                 && has_push_front<C, typename C::value_type>::value
                 && !is_keyval_container<C>::value,
                 C> CreateContainer()
{
    C cData;
    for (auto e : GetValueContainerData<typename C::value_type>())
        cData.push_front(e);
    return cData;
}

template<class T>
struct is_array : std::false_type
{};

template<class T, std::size_t N>
struct is_array<std::array<T,N> > : std::true_type
{};

template<typename C>
std::enable_if_t<is_array<C>::value,
                 C> CreateContainer()
{
    C cData;
    size_t i = 0;
    for (auto e : GetValueContainerData<typename C::value_type>())
        cData[i++] = e;
    return cData;
}

template<typename C>
std::enable_if_t<is_keyval_container<C>::value,
                 C> CreateContainer()
{
    C cData;
    for (auto& [key, val] : GetKeyValueContainerData<typename C::key_type, typename C::mapped_type>())
        cData.emplace(key, val);
    return cData;
}

/* reference creation templates */

template<typename C>
std::enable_if_t<has_size<C>::value
                 && has_begin<C>::value
                 && !is_keyval_container<C>::value,
                 string> GetReference(const C& data)
{
    const string strFmt(std::is_same<typename C::value_type, string>::value ?
                        "  \"{}\"{}\n" :
                        "  {}{}\n");
    string ref("[\n");
    auto len = data.size();
    for (auto e : data)
        ref += F(strFmt, e, --len ? "," : "");
    ref += "]";
    return ref;
}

template<typename C>
std::enable_if_t<!has_size<C>::value
                 && !is_keyval_container<C>::value,
                 string> GetReference(const C& data)
{
    const string strFmt(std::is_same<typename C::value_type, string>::value ?
                        "  \"{}\"{}\n" :
                        "  {}{}\n");
    string ref("[\n");
    auto it = data.begin();
    while (it != data.end())
    {
        auto itThis = it;
        it++;
        ref += F(strFmt, *itThis, (it != data.end()) ? "," : "");
    }
    ref += "]";
    return ref;
}

template<typename C>
std::enable_if_t<has_size<C>::value
                 && has_begin<C>::value
                 && is_keyval_container<C>::value,
                 string> GetReference(const C& data)
{
    const string strFmt(std::is_same<typename C::mapped_type, string>::value ?
                        "  \"{}\": \"{}\"{}\n" :
                        "  \"{}\": {}{}\n");
    string ref("{\n");
    auto len = data.size();
    for (auto [key, val] : data)
        ref += F(strFmt, key, val, --len ? "," : "");
    ref += "}";
    return ref;
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
std::enable_if_t<is_adapter<C>::value,
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
TEST_F(FmtsterTest, CONT ## _of_ ## KEYTYPE ## s__to_ ## VALTYPE ## s_to_JSON) \
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

/* tests */

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

    cout << endl;
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
// VALUECONTAINERTESTS(queue)
// VALUECONTAINERTESTS(priority_queue)

array<string, 3> CreateContainer()
{
    array<string, 3> a3;

    return a3;
}

// key/value container tests

KEYVALUECONTAINERTESTS(map)
KEYVALUECONTAINERTESTS(unordered_map)
// KEYVALUECONTAINERTESTS(multimap)
// KEYVALUECONTAINERTESTS(unordered_multimap)

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
        "  \"map1\": {\n"
        "    \"entry1\": \"value1\",\n"
        "    \"entry2\": \"value2\"\n"
        "  },\n"
        "  \"map2\": {\n"
        "    \"entry3\": \"value3\",\n"
        "    \"entry4\": \"value4\"\n"
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
        "    \"map1\": {\n"
        "        \"entry1\": \"value1\",\n"
        "        \"entry2\": \"value2\"\n"
        "    },\n"
        "    \"map2\": {\n"
        "        \"entry3\": \"value3\",\n"
        "        \"entry4\": \"value4\"\n"
        "    }\n"
        "}";
    string str = F("{:,,4}", mapofmapofstrings);
cout << str << endl;
    EXPECT_EQ(ref, str);
}

TEST_F(FmtsterTest, map_of_vectors_of_strings_to_JSON)
{
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
        "  \"vec1\": [\n"
        "    \"vec1val1\",\n"
        "    \"vec1val2\"\n"
        "  ],\n"
        "  \"vec2\": [\n"
        "    \"vec2val1\",\n"
        "    \"vec2val2\"\n"
        "  ]\n"
        "}";
    string str = F("{}", mapofvectorofstrings);
cout << str << endl;
    EXPECT_EQ(ref, str);
}
