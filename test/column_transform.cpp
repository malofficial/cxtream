/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE column_transform_test

#include <memory>
#include <ostream>
#include <tuple>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/generate_n.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/zip.hpp>

#include <column_transform.hpp>

// make the tuple print visible for boost test
// this is forbidden by the standard (simple workarounds?)
namespace std { using stream::utility::operator<<; }


using namespace stream;
using namespace ranges;
using namespace boost;
using namespace utility;


template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec)
{
  out << "{";
  for (std::size_t i = 0; i < vec.size(); ++i) {
    out << vec[i];
    if (i + 1 != vec.size())
      out << ", ";
  }
  out << "}";
  return out;
}


inline namespace columns {
  STREAM_DEFINE_COLUMN(A, int)
  STREAM_DEFINE_COLUMN(B, double)
  STREAM_DEFINE_COLUMN(Unique, std::unique_ptr<int>)
  STREAM_DEFINE_COLUMN(Shared, std::shared_ptr<int>)
}


bool operator==(const A& lhs, const A& rhs)
{ return lhs.value == rhs.value; }
bool operator==(const B& lhs, const B& rhs)
{ return lhs.value == rhs.value; }
std::ostream& operator<<(std::ostream& out, const A& rhs)
{ return out << rhs.value; }
std::ostream& operator<<(std::ostream& out, const B& rhs)
{ return out << rhs.value; }


BOOST_AUTO_TEST_CASE(column_transform_test)
{
  {
    // transform (int){'f', ..., 'i'} to (char){'a', ..., 'd'}
    // the char column is appended
    auto data =
        view::iota((int)'f', (int)'j')
      | view::transform(std::make_tuple<int>)
      | partial_transform(from<int>, to<char>, [](std::tuple<int> t){
          return std::make_tuple((char)(std::get<0>(t) - 5));
        })
      | to_vector;

    auto desired = 
        view::zip(view::iota((char)'a', (char)'e'),
                  view::iota((int)'f', (int)'j'))
      | view::transform([](auto t){ return std::tuple<char, int>(t); })
      | to_vector;

    BOOST_TEST(data == desired, test_tools::per_element{});
  }

  {
    // transform a single column to itself
    std::vector<std::tuple<A, B>> data = {{{3},{5.}}, {{1},{2.}}};
    auto generated =
        data
      | column_transform(from<A>, to<A>, [](const int &v){
          return std::make_tuple(v - 1);
        })
      | to_vector;

    std::vector<std::tuple<A, B>> desired = {{{2},{5.}}, {{0},{2.}}};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // transform two columns to a single column
    std::vector<std::tuple<A, B>> data = {{{3},{5.}}, {{1},{2.}}};
    auto generated =
        data
      | column_transform(from<A, B>, to<B>, [](int i, double d){
          return std::make_tuple((double)(i + d));
        })
      | to_vector;

    std::vector<std::tuple<B, A>> desired = {{{3 + 5.}, {3}}, {{1 + 2.}, {1}}};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // transform a single column to two columns
    std::vector<std::tuple<A>> data = {{{3}}, {{1}}};
    auto generated =
        data
      | column_transform(from<A>, to<A, B>, [](int i){
          return std::make_tuple(i + i, (double)(i * i));
        })
      | to_vector;

    std::vector<std::tuple<A, B>> desired = {{{6}, {9.}}, {{2}, {1.}}};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // for_each of two columns
    std::vector<std::tuple<A, B>> data = {{{3},{5.}}, {{1},{2.}}};
    auto generated =
        data
      | column_for_each(from<A, B>, [](const int &v, char c){ })
      | to_vector;

    std::vector<std::tuple<A, B>> desired = {{{3},{5.}}, {{1},{2.}}};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // create a new column
    auto generated =
        view::iota(0, 10)
      | column_create<A>
      | to_vector;

    std::vector<std::tuple<A>> desired = view::iota(0, 10);

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

  {
    // transform move-only column
    std::vector<std::tuple<A, Unique>> data;
    data.emplace_back(3, std::make_unique<int>(5));
    data.emplace_back(1, std::make_unique<int>(2));

    auto generated =
         data
       | view::move
       | column_transform(from<Unique>, to<Unique, B>,
           [](const std::unique_ptr<int> &ptr){
             return std::make_tuple(std::make_unique<int>(*ptr), (double)*ptr);
         })
       | to_vector;

    // check unique pointers
    std::vector<int> desired_ptr_vals{5, 2};
    for (int i = 0; i < 2; ++i) {
      BOOST_TEST(*(std::get<0>(generated[i]).value[0]) == desired_ptr_vals[i]);
    }

    // check other
    auto to_check =
        generated
      | view::move
      | column_drop<Unique>
      | to_vector;

    std::vector<std::tuple<B, A>> desired;
    desired.emplace_back(5., 3);
    desired.emplace_back(2., 1);
    BOOST_TEST(to_check == desired, test_tools::per_element{});
  }

  {
    // drop column
    std::vector<std::tuple<A, B>> data = {{{3},{5.}}, {{1},{2.}}};
    auto generated =
        data
      | column_drop<A>
      | to_vector;

    std::vector<std::tuple<B>> desired = {{{5.}}, {{2.}}};

    BOOST_TEST(generated == desired, test_tools::per_element{});
  }

}
