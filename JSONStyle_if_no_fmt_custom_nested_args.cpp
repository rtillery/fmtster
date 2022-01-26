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

class JSONStyle
{
public:
    string tab = "  ";         // expanded tab

    enum STYLEDEFS
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

    struct JSONStyleConfig
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

    union JSONStyleUnion
    {
        JSONStyleConfig vConfig;
        uint64_t vValue;
    };

    static constexpr JSONStyleUnion DEFAULTCONFIG =
    {
        .vConfig =
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

            .gapA = NEWLINE_TAB,
            .gapB = SPACE,
            .gapC = SPACE,

            .gap1 = NEWLINE_TAB,
            .gap2 = BLANK,
            .gap3 = SPACE,
            .gap4 = NEWLINE_TAB,
            .gap5 = BLANK,
            .gap6 = SPACE,
            .gap7 = NEWLINE,

            .emptyArray = SPACE,
            .emptyObject = SPACE,
            .sva = SAMELINE,
            .svo = SAMELINE,
        } // .vStyle
    };

    JSONStyleUnion mStyle;

    JSONStyle(uint64_t value = DEFAULTCONFIG.vValue)
    {
        if (value)
            mStyle.vValue = value;
        else
            mStyle.vValue = DEFAULTCONFIG.vValue;
    }

    uint64_t& value()
    {
        return mStyle.vValue;
    }
    JSONStyleConfig& config()
    {
        return mStyle.vConfig;
    }

    static_assert(sizeof(uint64_t) == (64 / 8));
    static_assert(sizeof(JSONStyleConfig) <= sizeof(uint64_t));
    static_assert(sizeof(JSONStyleUnion) == sizeof(uint64_t));

//     char (*__kaboom)[sizeof(uint64_t)] = 1;
//     char (*__kaboom)[sizeof(JSONStyleConfig)] = 1;
//     char (*__kaboom)[sizeof(JSONStyleUnion)] = 1;
};

int main()
{
    // cout << "Hello World" << endl;

    cout << F("sizeof(JSONStyle::JSONStyleConfig): {}", sizeof(JSONStyle::JSONStyleConfig)) << endl;
    cout << F("sizeof(JSONStyle::JSONStyleUnion): {}", sizeof(JSONStyle::JSONStyleUnion)) << endl;

    JSONStyle zeroStyle(0);
    cout << F("zeroStyle.config().tabCount: {}", zeroStyle.config().tabCount) << endl;

    JSONStyle defStyle;
    cout << F("defStyle.config().tabCount: {}", defStyle.config().tabCount) << endl;

    JSONStyle style;
    style.config().tabCount = 4;
    cout << F("style.config().tabCount: {}", style.config().tabCount) << endl;

    JSONStyle copyStyle(style.value());
    cout << F("copyStyle.config().tabCount: {}", copyStyle.config().tabCount) << endl;

    return 0;
}
