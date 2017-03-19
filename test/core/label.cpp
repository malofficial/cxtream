/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE label_test

#include <vector>

#include <boost/test/unit_test.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/view/filter.hpp>

#include <cxtream/core/label.hpp>

#include "common.hpp"

using namespace ranges;
using namespace cxtream;
using namespace boost;

std::size_t n_labels(const std::vector<std::size_t>& labels, std::size_t label)
{
  return
    (labels
     | view::filter([label](std::size_t l){ return l == label; }) 
     | to_vector
    ).size();
}

BOOST_AUTO_TEST_CASE(test_make_labels)
{
  std::vector<std::size_t> labels = make_labels(10, {0.5, 0.5});
  BOOST_TEST(labels.size() == 10);
  BOOST_TEST(n_labels(labels, 0) == 5);
  BOOST_TEST(n_labels(labels, 1) == 5);

  auto sorted_labels = labels;
  sorted_labels |= action::sort;
  BOOST_CHECK(sorted_labels != labels);
}
