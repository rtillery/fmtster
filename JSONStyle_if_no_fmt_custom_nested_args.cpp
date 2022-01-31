#include <cassert>
#include <cstdint>
#include <fmt/format.h>
#include <string>
#include <iostream>
#include <string>

using namespace std;

template<typename ...Args>
string F(std::string_view fmt, const Args&... args)
{
    return fmt::format(fmt, args...);
}

enum JSS
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

namespace internal
{

union ForwardJSONStyle
{
    struct
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
    uint64_t value;
};

} // namespace internal

constexpr internal::ForwardJSONStyle DEFAULTJSONCONFIG =
{
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
    }
};

union JSONStyle
{
public:
    struct
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
    uint64_t value;

    JSONStyle(uint64_t val = DEFAULTJSONCONFIG.value) : value(val)
    {}
};

static_assert(sizeof(uint64_t) == (64 / 8));
// char (*__kaboom)[sizeof(uint64_t)] = 1;

static_assert(sizeof(JSONStyle) <= sizeof(uint64_t));
// char (*__kaboom)[sizeof(JSONStyle)] = 1;

class JSONStyleHelper
{
public:
    string tab = "  ";  // expanded tab

    JSONStyle mStyle;

    JSONStyleHelper(uint64_t value = DEFAULTJSONCONFIG.value)
    {
        if (value)
            mStyle.value = value;
        else
            mStyle.value = DEFAULTJSONCONFIG.value;
        
        tab = mStyle.hardTab
              ? string(mStyle.tabCount, '\t')
              : string(mStyle.tabCount, ' ');
    }
};

int main()
{
    assert(64 / 8 >= sizeof(JSONStyle));
//     cout << F("sizeof(JSONStyle): {}", sizeof(JSONStyle)) << endl;

    JSONStyle zeroStyle{};
    assert(2 == zeroStyle.tabCount);
//     cout << F("zeroStyle.tabCount: {}", zeroStyle.tabCount) << endl;

    JSONStyle defStyle;
    assert(2 == defStyle.tabCount);
//     cout << F("defStyle.tabCount: {}", defStyle.tabCount) << endl;

    JSONStyle style;
    style.tabCount = 4;
    assert(4 == style.tabCount);
//     cout << F("style.tabCount: {}", style.tabCount) << endl;

    JSONStyle copyStyle(style.value);
    assert(4 == copyStyle.tabCount);
//     cout << F("copyStyle.tabCount: {}", copyStyle.tabCount) << endl;

    return 0;
}
