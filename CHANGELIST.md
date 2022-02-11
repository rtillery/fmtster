# **`fmtster` Change List**
## **0.5.0**
* Implemented support for multiple ways of specializing format (first argument:
  0, j..., J... (where ... is zero or more chars)); initial default format is
  JSON
* Implemented support for `fmtster::JSONStyle` object (second argument);
  initial (struct) default style is specified by default values in
  `fmtster::JSONStyle`
  ### **NOTE: that this structure must be passed by using the `value` member**
* Moved the tab specifier into `fmtster::JSONStyle`
* Repurposed third argument to allow specific per-call options using characters
  to enable each option.
## **0.4.0**
### **NOTE:** 0.4.x was skipped to emphasize how large the change between 0.3.0 and 0.5.0 was :-)

## **0.3.0**
* Fixed issues with `std::pair<>`
* Added support for `std::tuple<>` (currently only meant to hold `std::pair<>`s
  with the first elements being `std::string`s; the second elements can differ,
  providing heterogeneous object support)
* Added JSON string escaping
* Added support for JSON style setting (second format field) of 1 (in addition
  to 0) to remove surrounding brackets/brackets
* Added example-json.cpp
## **0.2.0**
* Added support for `std::pair<>`
* *Known issues*
  * Only tested with **{fmt}** versions 7.0.3 & 8.0.1
  * Only ASCII char types supported
  * Only JSON serialization supported
    * C-style strings (`char*`) not propertly supported (not quoted)
    * Only indent (tab & initial) configuration available
    * No escape of characters in JSON strings
  * (See https://github.com/HarmanFOSS/fmtster/issues for more known issues)
---
## **0.1.0**
*(Header contained no version.)*
* Support for:
  * `std::array<>`
  * `std::vector<>`
  * `std::deque<>`
  * `std::forward_list<>`
  * `std::list<>`
  * `std::set<>`
  * `std::unordered_set<>`
  * `std::multiset<>`
  * `std::unordered_multiset<>`
  * `std::stack<>`
  * `std::queue<>`
  * `std::priority_queue<>`
  * `std::map<>`
  * `std::unordered_map<>`
  * `std::multimap<>`
  * `std::unordered_multimap<>`
  * Custom containers which have all of these:
    * `const_iterator` member
    * `begin()` function
    * `end()` function
  * Nested containers within containers
* *Known issues*
  * Only tested with **{fmt}** version 7.0.3 and 7.1.3
  * Only ASCII char types supported
  * Only JSON serialization supported
    * C-style strings (`char*`) not propertly supported (not quoted)
    * Only indent (tab & initial) configuration available
    * No escape of characters in JSON strings
  * (See https://github.com/HarmanFOSS/fmtster/issues for more known issues)
