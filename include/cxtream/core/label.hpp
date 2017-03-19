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
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/take.hpp>

#include <cxtream/core/dataframe.hpp>

namespace cxtream {

  namespace {
    namespace view = ranges::view;
    namespace action = ranges::action;
  }


  /* make a single set of random labels */


  template<typename Prng = std::mt19937>
  std::vector<std::size_t> make_labels(
    std::size_t size,
    const std::vector<double>& ratio,
    Prng gen = Prng{std::random_device{}()})
  {
    assert(ranges::accumulate(ratio, 0.) <= 1.);
    std::vector<std::size_t> labels(size);
    std::vector<std::size_t> indexes = view::iota(0, size);
    action::shuffle(indexes, gen);

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

} // end namespace cxtream
#endif
