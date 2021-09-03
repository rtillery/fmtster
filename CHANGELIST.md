# **fmtster** Change List
## 0.2.0
* Added support for `std::pair<>`
* *Known issues*
  * Only tested with **{fmt}** version 8.0.1
  * Only ASCII char types supported
  * Only JSON serialization supported
    * C-style strings (`char*`) not propertly supported (not quoted)
    * Only indent (tab & initial) configuration available
    * No escape of characters in JSON strings
  * (See https://github.com/HarmanFOSS/fmtster/issues for more known issues)
---
---
## 0.1.0

**First release**

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
