<!---
 * Copyright (c) 2021 Harman International.  All rights reserved.
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
**`fmtster`** (format-ster) is designed to work with the
[{fmt}](https://fmt.dev/latest/index.html) library. It provides
`fmt::formatter` templates that allow
[`std` C++ containers](https://en.cppreference.com/w/cpp/container) to be passed
to the `fmt::format()` family of commands, and result in a `std::string`
serialization of the contents.

An example of its use:
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

`fmtster` is obviously useful for serialization, but it is also a useful
debugging aid, allowing quick dumping of `std` C++ containers to a `std::string`
for logging during development.<br>
<br>
***(In the future, it is intended that `fmtster` will be modfied to work with the
C++20 `std::format` feature.)***

---
<br>

## **Options**
In keeping with the design of {fmt}, `fmtster` provides the user optional style
configuration specifiers. However, please note that the style format for
`fmtster` differs from the {fmt} design, because the options being controlled
are appropriate to container serialization. *Individual type formatting is not
currently supported.*

<br>

### Serialization Format

The first style value (after the colon, before the closing brace or a comma)
indicates the serialization format. Following this (delimited by a
comma), zero or more style-specific specifiers can be added, themselves
separated by commas. Optional specifiers can be left empty and the default
values will be used. (Commas can be omitted after the last specifier provided.)

**EXAMPLE**:

> ```
> fmt::format("    {1}: {0:,,4,1}", myvector, "myvector")
> ```
> This serializes a `std::vector<>` formatted as:
> * JSON serialization format (the first field is empty, so the default serialization format is used)
> * default JSON style (the second field is empty, so the default JSON style is
> used)
> * four space tabs (overriding the default of two spaces)
> * indented by one tab unit (four spaces; the opening bracket may not be indented, depending on the second field)
>
> Output will look something like this:
> ```
>     myvector: [
>         "entry1",
>         "entry2"
>     ]
> ```

The serialization formats supported and their associated specification values
are:

* **JSON**
  * 0
  * json *(pending)*
  * j *(pending)*

* **XML** *(pending)*

The JSON serialization format is the default.

---
<br>

## **Serialization Format Details**
<br>

### **JSON**
The JSON format as specified at http://www.json.org
<br>
<br>

#### **JSON Options**
The JSON format specifier includes four optional fields, including the
serialization format specifier itself:

> `{:0,<style>,<tab>,<indent>}`<br>
> `{:json,<style>,<tab>,<indent>}` *(pending)*<br>
> `{:j,<style>,<tab>,<indent>}` *(pending)*<br>

**Style**

The second field holds the JSON serialization format style specifier, to provide
settings for multiple characteristics of the JSON output.<br>
<br>
The serialization format style allows user choices for things like the position
of opening and closing braces and brackets, the spacing between punctuation and
values, as well as exceptional choices like grouping short arrays on one line or
placing single entry JSON object on one line, etc.<br>
<br>
<br>
**NOTE: The method of specifying the serialization format style is under
consideration:**

Some proposed methods for how this field might be specified are:

>```
> Bitfields:   {:0,6,<tab>,<indent>}
> Tags:        {:j,spc-after-lead-brac|one-line-one-element-array,<tab>,<indent>}
> Struct ptr:  {:0,0x123456789ABCDEF0,<tab>,<indent>}
> ```
<br>

**Tab**

The third field indicates the type and length of the tabs:

* Positive values indicate the number of spaces per tab (e.g. 4 indicates a tab consisting of 4 spaces)
* Negative values indicate the number of tab characters per tab (e.g. -2 indictes a tab consisting of 2 hard tab ('\t') chracters)
* 0 indicates no tabs

<br>

**Indent**

The fourth field indicates the number of tab units used on all output lines.
This is added to the tab used internally for formatting the serialization, so
that the entire output is indented together.

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
