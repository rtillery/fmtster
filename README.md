<!---
 * Copyright (c) 2021 Harman International Industries, Incorporated.  All rights reserved.
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
-->

# **fmtster**

## **Introduction**

**`fmtster`** (format-ster) is designed to work with the
[{fmt}](https://fmt.dev/latest/index.html) library (and eventually C++20
`std::format`). It provides `fmt::formatter<>` templates that allow [`std` C++
containers](https://en.cppreference.com/w/cpp/container), including compound
containers (containers within containers) to be passed to the `fmt::format()`
family of commands, and produces a `std::string` serialization of the contents.

**NOTE:** The following aliases are used in the source code below for clarity.

    #include <map>
    using std::map;
    #include <string>
    using std::string;
    #include <vector>
    using std::vector;
    #include <iostream>
    using std::cout;
    using std::endl;

The simplest example of its use:

    map<string, map<string, vector<string> > > mydata = { ... };
    ...
    cout << fmt::format("mydata:\n{}", mydata) << endl;
and its (default) output:

    mydata:
    {
      "map1" : {
        "map1vec1" : [
          "map1vec1entry1",
          "map1vec1entry2"
        ],
        "map1vec2" : [
          "map1vec2entry1",
          "map1vec2entry2"
        ]
      },
      "map2" : {
        "map2vec1" : [
          "map2vec1entry1",
          "map2vec1entry2"
        ]
      }
    }
---
<br>

## **Uses**

Obviously `fmtster` is useful for **serialization** into the supported
serialization formats (JSON, etc.), but it is also useful for output to a
**debug log**, providing quick-and-easy view of `std` C++ containers during
software development, without the need for ad-hoc, one-off code.<br>

---
<br>

## **Formatting Arguments**
As with {fmt}, `fmtster` provides the user optional formatting configuration
arguments. However, please note that the arguments for `fmtster` differ from
the {fmt} design, because the options being controlled are specific to
container serialization.

***NOTE**: Unfortunately, integral type formatting is not currently supported.*

There are (currently) four arguments for setting the desired serialization
output, which are provided (like {fmt}) in the braces associated with a
specific value, separated by commas
* Serialization Format
* Format Style
* Per-Call Parameters
* Initial Indent
> `{`\<index>`:`\<format>`,`\<style>`,`\<per-call-parms>`,`\<indent>`}`

<br>

All of these arguments are optional. If they are not provided, their default
value is used. Some examples:
> `fmt::format("{}", container);`<br>
> The default values are used for all arguments.

> `fmt::format("{:json}", container);`<br>
> The default values are used for all arguments except the first.

> `fmt::format("{:,{}}", container, style.value);`<br>
> The default values are used for all arguments except the second, which
> is provided by the `style` object.

> `fmt::format("{:,,-b}", container);`<br>
> The default values are used for all arguments except the third.

> `fmt::format("{:,,,1}", container);`<br>
> The default values are used for all arguments except the fourth.

> `fmt::format("{:,{},,2}, container, style.value);`<br>
> The default values are used for all arguments except the second and
> fourth.

<br>

***Note:*** *As with other {fmt}-supported values, the argument index can be
placed before the colon in `fmtster` format strings.*

---
<br>

## **Serialization Format** (Argument 1)
<br>
The first argument (after the colon, before the closing brace or the first
comma) indicates the serialization format.

The serialization formats supported and their associated specification values
are:

* **JSON** (http://www.json.org)
  * 0 (any valid numerals with a value of 0)
  * j, json, J, JSON, etc. (any text starting with j or J)

<br>

[ **XML** (https://www.w3.org/standards/xml/core), and possibly other
serialization formats be be added later. ]

<br>

The ***default*** serialization format is 0 (JSON).

---
<br>

## **Format Style** (Argument 2)

The second argument holds the JSON serialization format style specifier. This
allows the user to specify the output style of the JSON produced by `fmtster`.
<br>
<br>
The serialization format style allows user choices for things like the position
of opening and closing braces and brackets, the spacing between punctuation and
values, as well as exceptional choices like grouping short or empty arrays on
one line or placing single entry JSON objects on a single line, etc.<br>
<br>
`fmtster::JSONStyle` is the structure which provides these options for JSON.
*(Currently, only the kind of tab (hard or space) and number of tab characters
are configurable.)*
<br>

---
<br>

## **Per-Call Parameters** (Argument 3)

The third argument allows some single-call flags to be provided:

  * `-` ... negate (disable) the option which follows; **no default**
  * `f` ... the serialization format specified in the first argument (JSON,
    etc.) will become the new default serialization format (since only the JSON
    format is supported as of this release this option does nothing at present);
    **default is negated**
  * `s` ... the style specified in the second argument will become the new
    default for the associated serialization format; **default is negated**
  * `b` ... braces/brackets will be added around the container output (this
    setting only applies to the outermost container in a compound container
    (internal containers will always have braces/brackets); using this flag to
    disable braces is useful for manual construction of complex objects into
    the JSON output, especially when combining multiple container contents into
    the same JSON object--see `example-json.cpp`); **default is enabled**

---
<br>

## **Initial Indent** (Argument 4)

The fourth argument indicates the starting number of tab units. This is added to
the tabs used internally for formatting the serialization, so that the entire
output is indented together. A value of 2 indicates two tab units. If the tab is
set to two spaces, that would be a total of four spaces as a starting indent.

**NOTE**: The indent specifier may be ignored on the first line of
serialization, when certain style settings are chosen.<br>

---
<br>

## **Nested Arguments**
<br>

As with other {fmt} arguments, direct values can be used or values can be
provided via additional arguments. For example:

### **Serialization Format**
    // format accepts an integer or (C or C++) string
    cout << F("{:{}}", container, 0) << endl;
    cout << F("{:{}}", container, "json") << endl;
    cout << F("{:{}}", container, "JSON"s) << endl;

### **Format Style***
    // style accepts a very large integer that is a union with a
    // bitfield-based structure holding the configuration settings; like an
    // empty argument, a value of 0 causes the current default style (for the
    // format specified by the first argument)
    fmtster::JSONStyle style;
    style.tabCount = 4;
    cout << F("{:,{}}", container, style.value) << endl;

### **Per-Call Parameters**
    // per-call-parameters accept a (C or C++ string)
    cout << F("{:,,{}}", container, "-b") << endl;

### **Initial Indent**
    // indent accepts an integer
    cout << F("{:,,,{}}", container, indent) << endl;

---
<br>

## **Aliases**
<br>

`fmtster` provides the following aliases (example code below showing how
to utilize them):

<br>

### `F()` ... alias for `fmt::format()`
```
#include <fmtster>
using fmtster::F;

cout << F("{}", container) << endl;
```
<br>

***NOTE:*** *The remainder of this document may use the aliases mentioned
above (as well as other common aliases or namespace uses which may not be
explicitly shown).*

---
<br>

## **Managing The Default Serialization Format And Style**
<br>

### **Default Serialization Format**

`fmtster` initializes with a built-in default serialization format (JSON).

To change the default serialization format, use the per-call-parameter argument
(`f`) in any serialization call using the desired default serialization format:

    // set the default serialization format to JSON
    cout << F("{:json,,f}", container) << endl;

    // the following line will print exactly the same output as the previous output
    cout << F("{}", container) << endl;

    // print the index of the current default serialization format (for debugging)
    cout << F("{}", fmtster::Base::GetDefaultFormat()) << endl;
### **Default Format Style**

`fmtster` initializes with a built-in style format for each serialization
format.

To change the default style for a specific serialization format, use the
per-call-parameter argument (`s`) on any serialization using the desired
serialization format and desired default style for that format:

    // configure the desired default JSON style
    fmtster::JSONStyle style;
    style.tabCount = 4;

    // use the specified serialization format and associated style and set both
    // to the new defaults for each
    cout << F("{:j,{},fs}", container, style.value) << endl;

    // the following line will print exactly the same output as the previous output
    cout << F("{}", container) << endl;

    // print the current default JSON format style (for debugging)
    cout << F("{}", fmtster::Base::GetDefaultJSONStyle()) << endl;

---

<br>

## **Extending `fmster`**
<br>

`fmtster`'s powerful formatting control can be extended to serialize custom
structures/classes, including those that themselves include containers or other
specialized structure/classes. For example:

    // custom structure
    struct Color
    {
        string hue; // color family
        vector<tuple<string, float> > primaries;
    };

    // specialized template for {fmt}, utilizing fmtster's formatting configuration
    template<>
    struct fmt::formatter<Color>
      : fmtster::Base
    {
        template<typename FormatContext>
        auto format(const Color& color, FormatContext& ctx)
        {
            using std::make_pair;

            // this function must be called to allow fmtster to resolve the
            // arguments which may be passed directly or in nested braces (i.e. as
            // a value argument to the fmt::format() call instead of as a literal
            // int the format string itself)
            resolveArgs(ctx);

            // the easiest way to provide custom support is to use fmtster's
            // support of std::tuple<>, filling it with std::pair<>s, each
            // containing a string with the member name and the actual member
            // value
            auto tup = std::make_tuple(
                make_pair("hue"s, color.hue),
                make_pair("primaries"s, color.primaries)
            );

            // forwarding all the resolved arguments ensures that fmtster can
            // format the serial output as specified in the fmt::format() call
            return format_to(ctx.out(),
                             "{:{},{},{},{}}",
                             tup,
                             mFormatSetting,
                             mStyleValue,
                             mDisableBras ? "-b" : "",
                             mIndentSetting);
        }
    };

    // initialize a style object with the desired format
    fmtster::JSONStyle style;
    style.tabCount = 4;

    // output the custom struct with fmster style
    cout << F("Colors: {:j,{}}", colors, style.value) << endl;
The resulting output of this code:

    Colors: {
        "Burgundy" : {
            "hue" : "red",
            "primaries" : [
                {
                    "red",
                    1.0
                }
            ]
        },
        "Gray" : {
            "hue" : "none",
            "primaries" : [
                {
                    "red",
                    0.33333334
                },
                {
                    "blue",
                    0.33333334
                },
                {
                    "yellow",
                    0.33333334
                }
            ]
        }
    }

See the examples (e.g. `example-json.cpp`) for full source code.
