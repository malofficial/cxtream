Introduction {#mainpage}
============

\tableofcontents

Preface
-------
---

__cxtream__ is a C++ library for efficient data processing. Its main purpose is to simplify
and acclelerate data preparation for deep learning models, but it is generic enough to be used
in many other areas.

**This project is under heavy development. The API is continuously changing without regard
to backward compatibility.**

Quick Start
-----------
---

Before proceeding, please install __cxtream__ by following the
\ref installation "installation guide".

__cxtream__ provides many smaller utilities, such as loading \ref CSV "CSV files" to
\ref Dataframe "dataframes", \ref Base64 "base64 decoders",
\ref Groups "data splitting" and [much more](modules.html). However, probably the most
useful funcionality of the library is \ref Stream "data stream processing".

Before we dive into details, we recommend to get familiar with the concept of C++
ranges and [Range-v3](http://ericniebler.github.io/range-v3/) library by Eric Niebler.
Ranges will soon become a fundamental part of C++ standard and Range-v3 library
is an experimental implementation of the proposal. To sum up the idea,
a range can be thought of as a pair of begin/end iterators, with potentially
infinite distance.

Now, a stream in the context of __cxtream__ is a range of tuples, where the tuples
contain so-called _columns_.  Each column contains a single _batch_ of data of
the same type. To demonstrate this concept on an example, let us assume that
we have a table of four users and each user consists of two
columns: `login` and `age`:

| login | age |
|-------|-----|
| marry | 24  |
| ted   | 41  |
| anna  | 16  |
| josh  | 59  |

A stream made of such data with batch size of two may look as following:

```{.cpp}
CXTREAM_DEFINE_COLUMN(login, std::string)  # helper macro to define a column of strings
CXTREAM_DEFINE_COLUMN(age, int)

std::tuple<login, age> data1 = {{"marry", "ted"}, {24, 41}},
std::tuple<login, age> data2 = {{"anna", "josh"}, {16, 59}};
std::vector<std::tuple<login, age>> stream = {data1, data2};
```

Of course, the stream may as well be loaded from a file or a database and may even
be infinite. __cxtream__ provides \ref Stream "many tools" to manipulate such
streams using the pipe "|" operator. The following example demonstrates the main idea of
stream pipelining (see the full example in [example.cpp](example_8cpp_source.html)):

```{.cpp}
namespace cxs = cxtream::stream;
using cxs::from; using cxs::to; using cxs::by; using cxs::dim;

CXTREAM_DEFINE_COLUMN(login, std::string)  // helper macro to define a column of strings
CXTREAM_DEFINE_COLUMN(age, int)

std::vector<std::string> logins = {"marry", "ted", "anna", "josh"};
std::vector<int>           ages = {     24,    41,     16,     59};

auto stream = ranges::view::zip(logins, ages)

  // create a batched stream out of the raw data
  | cxs::create<login, age>(2)

  // make everyone older by one year
  | cxs::transform(from<age>, to<age>, [](int a) { return a + 1; })

  // increase each letter in the logins by one (i.e., a->b, e->f ...)
  | cxs::transform(from<login>, to<login>, [](char c) { return c + 1; }, dim<2>)

  // increase the ages by the length of the login
  | cxs::transform(from<login, age>, to<age>, [](std::string l, int a) {
        return a + l.length();
    })

  // probabilistically rename 50% of the people to "buzz"
  | cxs::transform(from<login>, to<login>, 0.5, [](std::string) -> std::string {
        return "buzz";
    })

  // drop the login column from the stream
  | cxs::drop<login>

  // introduce the login column back to the stream
  | cxs::transform(from<age>, to<login>, [](int a) {
        return "person_" + std::to_string(a) + "_years_old";
    })

  // filter only people older than 30 years
  | cxs::filter(from<login, age>, by<age>, [](int a) { return a > 30; })

  // asynchronously buffer the stream during iteration
  | cxs::buffer(2);

// extract the ages from the stream to std::vector
ages = cxs::unpack(stream, from<age>);
assert((ages == std::vector<int>{45, 64}));
```

For more information, please refer to the [modules](modules.html) documentation section
and browse the API reference.

Multidimensional data
---------------------
---

In __cxtream__, when we talk about multidimensional data, we are talking about nested
ranges. For instance, `std::vector<int>` is a one dimensional vector and
`std::vector<std::vector<int>>` is two dimensional. TODO


TODO example.


Python
------
---

For more information, please refer to the \ref Python "Python binding API" documentation. TODO example.

Example
-------
---

Please refer to [Cognexa/cxMNIST](https://github.com/Cognexa/cxMNIST) repository for
a comprehensive usage example.
