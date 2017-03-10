/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0.
 *  (see http://www.boost.org/LICENSE_1_0.txt)
 *********************************************************/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE batch_view_test

#include <vector>
#include <memory>

#include <boost/test/unit_test.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/move.hpp>

#include <cxtream/core/batch.hpp>

#include "common.hpp"

using namespace ranges;
using namespace cxtream;
using namespace boost;


auto generate_batched_data(std::vector<int> batch_sizes)
{
  std::vector<std::tuple<Unique, Shared>> data;
  int counter = 0;
  for (std::size_t i = 0; i < batch_sizes.size(); ++i) {
    std::vector<std::unique_ptr<int>> unique_data;
    std::vector<std::shared_ptr<int>> shared_data;
    for (int j = 0; j < batch_sizes[i]; ++j) {
      unique_data.emplace_back(std::make_unique<int>(counter));
      shared_data.emplace_back(std::make_shared<int>(counter));
      ++counter;
    }
    data.emplace_back(std::make_tuple(std::move(unique_data),
                                      std::move(shared_data)));
  }
  return data;
}


auto generate_regular_batched_data(int batches, int batch_size)
{
  return generate_batched_data(std::vector<int>(batches, batch_size));
}


template<typename Data>
void check_20_elems_batch_size_2(Data data)
{
    auto rng = data | view::move | batch(3);
    using tuple_type = decltype(*ranges::begin(rng));
    static_assert(std::is_same<std::tuple<Unique, Shared>&, tuple_type>{});
    using first_type = decltype(std::get<0>(*ranges::begin(rng)).value[0]);
    static_assert(std::is_same<std::unique_ptr<int>&, first_type>{});
    using second_type = decltype(std::get<1>(*ranges::begin(rng)).value[0]);
    static_assert(std::is_same<std::shared_ptr<int>&, second_type>{});

    // iterate through batches
    std::vector<int> result_unique;
    std::vector<int> result_shared;
    int batch_n = 0;
    int n = 0;
    for (auto&& tuple : rng) {
      // the last batch should be smaller
      if (batch_n == 6) {
        BOOST_TEST(std::get<0>(tuple).value.size() == 2);
        BOOST_TEST(std::get<1>(tuple).value.size() == 2);
      }
      else {
        BOOST_TEST(std::get<0>(tuple).value.size() == 3);
        BOOST_TEST(std::get<1>(tuple).value.size() == 3);
      }
      BOOST_CHECK(is_same_batch_size(tuple));

      // iterate through batch values
      for (auto& elem : std::get<0>(tuple).value) {
        result_unique.push_back(*elem);
        ++n;
      }
      for (auto& elem : std::get<1>(tuple).value) {
        result_shared.push_back(*elem);
      }
      ++batch_n;
    }
    BOOST_TEST(batch_n == 7);
    BOOST_TEST(n == 20);

    auto desired = ranges::view::iota(0, 20) | ranges::to_vector;
    BOOST_TEST(result_unique == desired, test_tools::per_element{});
    BOOST_TEST(result_shared == desired, test_tools::per_element{});
}


BOOST_AUTO_TEST_CASE(batch_view_test)
{
  {
    // batch out of larger batches
    auto data = generate_regular_batched_data(3, 4);

    auto rng = data | view::move | batch(1);
    using tuple_type = decltype(*ranges::begin(rng));
    static_assert(std::is_same<std::tuple<Unique, Shared>&, tuple_type>{});
    using first_type = decltype(std::get<0>(*ranges::begin(rng)).value[0]);
    static_assert(std::is_same<std::unique_ptr<int>&, first_type>{});
    using second_type = decltype(std::get<1>(*ranges::begin(rng)).value[0]);
    static_assert(std::is_same<std::shared_ptr<int>&, second_type>{});

    // iterate through batches
    std::vector<int> result_unique;
    std::vector<int> result_shared;
    int n = 0;
    for (auto&& tuple : rng) {
      // the batch should be only a single element
      BOOST_TEST(std::get<0>(tuple).value.size() == 1);
      BOOST_TEST(std::get<1>(tuple).value.size() == 1);
      // remember the values
      result_unique.push_back(*(std::get<0>(tuple).value[0]));
      result_shared.push_back(*(std::get<1>(tuple).value[0]));
      ++n;
    }
    BOOST_TEST(n == 12);

    auto desired = ranges::view::iota(0, 12) | ranges::to_vector;
    BOOST_TEST(result_unique == desired, test_tools::per_element{});
    BOOST_TEST(result_shared == desired, test_tools::per_element{});
  }

  {
    // batch out of smaller batches
    check_20_elems_batch_size_2(generate_regular_batched_data(10, 2));
  }

  {
    // batch out of iregularly sized batches
    check_20_elems_batch_size_2(generate_batched_data({0, 1, 2, 0, 5, 2, 0, 1, 7, 0, 2, 0, 0}));
  }

  {
    // batch out of empty batches
    auto data = generate_batched_data({0, 0, 0, 0});

    auto rng = data | view::move | batch(1);

    BOOST_CHECK(rng.begin() == rng.end());
  }

  {
    // batch out of empty range
    auto data = generate_batched_data({});

    auto rng = data | view::move | batch(1);

    BOOST_CHECK(rng.begin() == rng.end());
  }
}
