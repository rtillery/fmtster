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
[{fmt}](https://fmt.dev/latest/index.html) library. It provides
`fmt::formatter` templates that allow
[`std` C++ containers](https://en.cppreference.com/w/cpp/container), including
compound containers (containers within containers) to be passed to the
`fmt::format()` family of commands, and result in a `std::string` serialization
of the contents.

The simplest example of its use:
> ```
> std::map<std::string, std::map<std::string, std::vector<std::string> > > mydata;
> ...
> std::cout << fmt::format("mydata:\n{}", mydata) << std::endl;
> ```
and its (default) output:
> ```
> mydata:
> {
>   "map1" : {
>     "map1vec1" : [
>       "map1vec1entry1",
>       "map1vec1entry2"
>     ],
>     "map1vec2" : [
>       "map1vec2entry1",
>       "map1vec2entry2"
>     ]
>   },
>   "map2" : {
>     "map2vec1" : [
>       "map2vec1entry1",
>       "map2vec1entry2"
>     ]
>   }
> }
> ```

<br>

***(Coming soon (hopefully), `fmtster` will be modfied to work with the C++20
`std::format` feature.)***

---

## **Uses**

Obviously `fmtster` is useful for **serialization**, but it is also useful for
formatting a **debug log**, allowing quick dumping of `std` C++ containers
during development.<br>

---
<br>

## **Formatting Options**
As with {fmt}, `fmtster` provides the user optional formatting configuration
settings. However, please note that the settings for `fmtster` differ from
the {fmt} design, because the options being controlled are appropriate for
container serialization.

***NOTE**: Integral type formatting is not currently supported.*

There are (currently) four parameters for setting the format style:
* Serialization Format
* Format Style
* Tab Specification
* Initial Indent

All of these parameters are optional. If they are not provided, the default
value is used. Some examples:
> `fmt::format("{}", container);`<br>
> The default values are used for all parameters.

> `fmt::format("{:,1}", container);`<br>
> The default values are used for all parameters except the second, which
> is set to 1.

> `fmt::format("{:,,4}", container);`<br>
> The default values are used for all parameters except the third, which
> is set to 4.

> `fmt::format("{:,,,1}", container);`<br>
> The default values are used for all parameters except the fourth, which
> is set to 1.

> `fmt::format("{:,1,,2}, container);`<br>
> The default values are used for all parameters except the second and
> fourth, which are set to values of 1 and 2, respectively.

<br>

---
<br>

### **Serialization Format**

The first value (after the colon, before the closing brace or a comma)
indicates the serialization format.

The serialization formats supported and their associated specification values
are:

* **JSON**
  * 0
  * json *(possible future alias)*
  * j *(possible future alias)*

* **XML** *(pending)*
* etc.

<br>

The ***default*** is 0 (JSON serialization format).

<br>

### **JSON**
The JSON format as specified at http://www.json.org
<br>

The JSON format specifier includes four optional fields, including the
serialization format specifier itself:

> `{:0,<style>,<tab>,<indent>}`<br>
> `{:json,<style>,<tab>,<indent>}` *(possible future alias)*<br>
> `{:j,<style>,<tab>,<indent>}` *(possible future alias)*<br>

<br>

**Style**

The second field holds the JSON serialization format style specifier, to provide
settings for multiple characteristics of the JSON output.<br>
<br>
The serialization format style allows user choices for things like the position
of opening and closing braces and brackets, the spacing between punctuation and
values, as well as exceptional choices like grouping short or empty arrays on
one line or placing single entry JSON objects on a single line, etc.<br>
<br>
**NOTE: The method of specifying the serialization format style is under
consideration.**
<br>

At present, there is only one JSON style option supported. A value of 0 (the
default) will cause the output to use the default layout (see examples), with
braces or brackets (as appropriate for JSON) around the object. A value of 1
will remove the brackets/braces. The initiial indent (see below) is used for
the closing brace/bracket with a setting of 0 and is used for the data if set
to 1. (The opening brace is not currently indented.)

<br>

**Tab**

The third field indicates the type and length of the tabs:

* Positive values indicate the number of spaces per tab (e.g. 4 indicates a tab
  consisting of 4 spaces)
* Negative values indicate the number of tab characters per tab (e.g. -2
  indictes a tab consisting of 2 hard tab ('\t') chracters)
* 0 indicates no tabs

<br>

**Indent**

The fourth field indicates the starting number of tab units. This is added to
the tab used internally for formatting the serialization, so that the entire
output is indented together.

**NOTE**: The indent specifier may be ignored when outputting the first line of
a given container. This is due to the need to differentiate between tab levels
during recusive serialization of containers within containers.<br>
<br>
\*\* *A table will needs to be placed here to explain the behavior and options to the user.* \*\*

<br>

### **XML**
The XML format as specified at https://www.w3.org/standards/xml/core

**Support of XML is TBD.**

---
<br>

## **Global Serialization Format Specification**
<br>

*Providing serialization formatting on each use of
`fmtster` can become burdensome. A method to provide a global format
specification to override the default settings seems like a worthwhile addition.
However, consideration of the side-effects, such as use of `fmtster` by multiple
modules in the same process, will necessitate more consideration of an exact
approach.*

---
<br>

## **Examples**
<br>

See `example-json.cpp` for quick start to using `fmtster`.
