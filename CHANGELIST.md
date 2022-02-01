# **`fmtster` Change List**
## **0.4.0**
* Extended support for multiple specifications of format (first argument: 0,
  j..., J... (where ... is zero or more chars)); default format is JSON
* Altered JSON style setting to take numeric `value` from the newly added
  `JSONStyle` union (configure with the anonymous structure members, pass the
  `value` to **fmtster**; default JSON style is specified by the
  `fmtster::DEFAULTJSONSTYLE` constant structure
* Moved the tab specifier into `JSONStyle`
* Repurposed third argument to allow specific per-call options using characters
  to enable each option:
  * `-` - negate (disable) the option which follows
  * `f` - make the format specified in the first argument (JSON, etc.) the new
     default format (default is disabled; since only the JSON format is
     supported as of this release this option does very little at present
  * `s` - make the style specified in the second argument the new default for
    the associated format (default is disabled)
  * `b` - add braces/brackets to the container output (default is enabled; the
    setting only applies to the outermost container--containers within the
    outer container will utilize braces/brackets; this is useful for manual
    construction of JSON output, especially when a user wants to combine
    multiple container contents in the same JSON object)
  * `!` - dump the contents of the `JSONStyle` object (as JSON) instead of the
    contents of the container (default is disabled)
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
