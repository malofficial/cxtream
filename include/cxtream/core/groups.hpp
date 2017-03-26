/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_GROUPS_HPP
#define CXTREAM_CORE_GROUPS_HPP

#include <cxtream/core/dataframe.hpp>

#include <range/v3/action/shuffle.hpp>
#include <range/v3/algorithm/copy.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/take.hpp>

#include <random>
#include <vector>

namespace cxtream {


  /* generate a single set of random groups */


  template<typename Prng = std::mt19937>
  std::vector<std::size_t> generate_groups(
    std::size_t size,
    std::vector<double> ratio,
    Prng&& gen = Prng{std::random_device{}()})
  {
    namespace view = ranges::view;

    // scale to [0, 1]
    double ratio_sum = ranges::accumulate(ratio, 0.);
    for (auto& r : ratio) r /= ratio_sum;

    std::vector<std::size_t> groups(size);
    std::vector<std::size_t> indexes = view::iota(0UL, size);
    ranges::action::shuffle(indexes, gen);

    std::size_t group = 0;
    std::size_t done = 0;
    for (double r : ratio) {
      std::size_t count = r * size;
      for (std::size_t index : indexes | view::drop(done) | view::take(count)) {
        groups[index] = group;
      }
      ++group;
      done += count;
    }

    return groups;
  }


  /* generate many sets of random groups */


  // fixed groups - those are the same for all sets (test in machine learning)
  // volatile groups - those are changing in each set (train, valid in machine learning)

  template<typename Prng = std::mt19937>
  std::vector<std::vector<std::size_t>> generate_many_groups(
    std::size_t n,
    std::size_t size,
    const std::vector<double>& volatile_ratio,
    const std::vector<double>& fixed_ratio,
    Prng&& gen = Prng{std::random_device{}()})
  {
    namespace view = ranges::view;

    std::size_t volatile_size = volatile_ratio.size();
    auto full_ratio = view::concat(volatile_ratio, fixed_ratio);

    std::vector<std::vector<std::size_t>> all_groups;
    std::vector<std::size_t> initial_groups = generate_groups(size, full_ratio, gen);

    for (std::size_t i = 0; i < n; ++i) {
      auto groups = initial_groups;
      // select those groups, which are volatile (those will be replaced)
      auto groups_volatile =
          groups
        | view::filter([volatile_size](std::size_t l){ return l < volatile_size; });
      // count the number of volatile groups
      std::size_t volatile_count = ranges::distance(groups_volatile);
      // generate the replacement
      auto groups_volatile_new = generate_groups(volatile_count, volatile_ratio, gen);
      // replace
      ranges::copy(groups_volatile_new, groups_volatile.begin());
      // store
      all_groups.emplace_back(std::move(groups));
    }

    return all_groups;
  }


} // end namespace cxtream
#endif
