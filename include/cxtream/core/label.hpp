/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_LABEL_HPP
#define CXTREAM_CORE_LABEL_HPP

#include <random>
#include <vector>

#include <range/v3/action/shuffle.hpp>
#include <range/v3/algorithm/copy.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/take.hpp>

#include <cxtream/core/dataframe.hpp>

namespace cxtream {


  /* make a single set of random labels */


  template<typename Prng = std::mt19937>
  std::vector<std::size_t> make_labels(
    std::size_t size,
    std::vector<double> ratio,
    Prng&& gen = Prng{std::random_device{}()})
  {
    namespace view = ranges::view;

    // scale to [0, 1]
    double ratio_sum = ranges::accumulate(ratio, 0.);
    for (auto& r : ratio) r /= ratio_sum;

    std::vector<std::size_t> labels(size);
    std::vector<std::size_t> indexes = view::iota(0UL, size);
    ranges::action::shuffle(indexes, gen);

    std::size_t label = 0;
    std::size_t done = 0;
    for (double r : ratio) {
      std::size_t count = r * size;
      for (std::size_t index : indexes | view::drop(done) | view::take(count)) {
        labels[index] = label;
      }
      ++label;
      done += count;
    }

    return labels;
  }


  /* make many sets of random labels */


  // fixed labels - those are the same for all sets (test in machine learning)
  // volatile labels - those are changing in each set (train, valid in machine learning)

  template<typename Prng = std::mt19937>
  std::vector<std::vector<std::size_t>> make_many_labels(
    std::size_t n,
    std::size_t size,
    const std::vector<double>& fixed_ratio,
    const std::vector<double>& volatile_ratio,
    Prng&& gen = Prng{std::random_device{}()})
  {
    namespace view = ranges::view;

    std::size_t fixed_size = fixed_ratio.size();
    auto full_ratio = view::concat(fixed_ratio, volatile_ratio);

    std::vector<std::vector<std::size_t>> all_labels;
    std::vector<std::size_t> initial_labels = make_labels(size, full_ratio, gen);

    for (std::size_t i = 0; i < n; ++i) {
      auto labels = initial_labels;
      // select those labels, which are volatile (those will be replaced)
      auto labels_volatile =
          labels
        | view::filter([fixed_size](std::size_t l){ return l >= fixed_size; });
      // count the number of volatile labels
      std::size_t volatile_count = ranges::distance(labels_volatile);
      // generate the replacement
      auto labels_volatile_new = make_labels(volatile_count, volatile_ratio, gen);
      for (std::size_t& l : labels_volatile_new) l += fixed_size;
      // replace
      ranges::copy(labels_volatile_new, labels_volatile.begin());
      // store
      all_labels.emplace_back(std::move(labels));
    }

    return all_labels;
  }


} // end namespace cxtream
#endif
