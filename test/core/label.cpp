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

#include <random>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/view/filter.hpp>

#include <cxtream/core/label.hpp>

#include "common.hpp"

using namespace ranges;
using namespace cxtream;
using namespace boost;


// test with a seeded random generator
std::mt19937 prng{1000003};


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
  std::vector<std::size_t> labels = make_labels(10, {1.5, 1.5}, prng);
  BOOST_TEST(labels.size() == 10UL);
  BOOST_TEST(n_labels(labels, 0) == 5UL);
  BOOST_TEST(n_labels(labels, 1) == 5UL);

  auto sorted_labels = labels;
  sorted_labels |= action::sort;
  BOOST_CHECK(sorted_labels != labels);
}


BOOST_AUTO_TEST_CASE(test_make_labels_zero_ratio)
{
  std::vector<std::size_t> labels = make_labels(10, {1.5, 0, 1.5}, prng);
  BOOST_TEST(labels.size() == 10UL);
  BOOST_TEST(n_labels(labels, 0) == 5UL);
  BOOST_TEST(n_labels(labels, 1) == 0UL);
  BOOST_TEST(n_labels(labels, 2) == 5UL);

  auto sorted_labels = labels;
  sorted_labels |= action::sort;
  BOOST_CHECK(sorted_labels != labels);
}


BOOST_AUTO_TEST_CASE(test_make_many_labels)
{
  std::vector<std::vector<std::size_t>> labels =
    make_many_labels(2, 20, {0.3, 0.3}, {0.2, 0.2}, prng);

  BOOST_TEST(labels.size() == 2UL);
  BOOST_TEST(labels[0].size() == 20UL);
  BOOST_TEST(labels[1].size() == 20UL);

  // check that the labels have correct ratio
  for (std::size_t j = 0; j < 2; ++j) {
    BOOST_TEST(n_labels(labels[j], 0) == 6UL);
    BOOST_TEST(n_labels(labels[j], 1) == 6UL);
    BOOST_TEST(n_labels(labels[j], 2) == 4UL);
    BOOST_TEST(n_labels(labels[j], 3) == 4UL);
  }

  // check that all the fixed labels (i.e., 2+) are the same
  for (std::size_t i = 0; i < 10; ++i) {
    if (labels[0][i] >= 2UL)
      BOOST_TEST(labels[0][i] == labels[0][i]);
  }
  
  // check that the labels differ
  BOOST_CHECK(labels[0] != labels[1]);
}


BOOST_AUTO_TEST_CASE(test_make_many_labels_zero_ratio)
{
  std::vector<std::vector<std::size_t>> labels =
    make_many_labels(2, 20, {0.3, 0, 0, 0.3}, {0.2, 0, 0, 0.2}, prng);

  BOOST_TEST(labels.size() == 2UL);
  BOOST_TEST(labels[0].size() == 20UL);
  BOOST_TEST(labels[1].size() == 20UL);

  // check that the labels have correct ratio
  for (std::size_t j = 0; j < 2; ++j) {
    BOOST_TEST(n_labels(labels[j], 0) == 6UL);
    BOOST_TEST(n_labels(labels[j], 3) == 6UL);
    BOOST_TEST(n_labels(labels[j], 4) == 4UL);
    BOOST_TEST(n_labels(labels[j], 7) == 4UL);
  }

  // check that all the fixed labels (i.e., 4+) are the same
  for (std::size_t i = 0; i < 10; ++i) {
    if (labels[0][i] >= 4UL)
      BOOST_TEST(labels[0][i] == labels[0][i]);
  }
  
  // check that the labels differ
  BOOST_CHECK(labels[0] != labels[1]);
}
