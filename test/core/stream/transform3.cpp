/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

// The tests for stream::transform are split to multiple
// files to speed up compilation in case of multiple CPUs.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE transform3_test

#include "transform.hpp"

using namespace cxtream::stream;

BOOST_AUTO_TEST_CASE(test_probabilistic_dim2_move_only)
{
    auto data = generate_move_only_data();

    auto rng = data
      | ranges::view::move
      | create<Int, UniqueVec>(2)
      | drop<Int>
      // create IntVec column
      | transform(from<UniqueVec>, to<IntVec>, [](auto&&) {
            return 7;
        }, dim<2>)
      // probabilistically transform a single columns to itself
      | transform(from<UniqueVec>, to<UniqueVec>, 0.0,
          [](std::unique_ptr<int>& ptr) {
            return std::make_unique<int>(20);
        }, prng, dim<2>)
      // probabilistically transform two columns to two columns
      | transform(from<UniqueVec, IntVec>, to<IntVec, UniqueVec>, 0.5,
          [](std::unique_ptr<int>& ptr, int val) {
            return std::make_tuple(val, std::make_unique<int>(19));
        }, prng, dim<2>)
      // probabilistically transform two columns to one column
      | transform(from<IntVec, UniqueVec>, to<UniqueVec>, 0.5,
          [](int, std::unique_ptr<int>& ptr) {
            return std::make_unique<int>(19);
        }, prng, dim<2>)
      | unique_vec_to_int_vec();  // the original IntVec gets overwritten here

    std::vector<int> generated = unpack(rng, from<IntVec>, dim<2>);
    long number19 = ranges::count(generated, 19);
    long number20 = ranges::count(generated, 20);
    BOOST_TEST(generated.size() == 6);
    BOOST_TEST(number19 >= 3);
    BOOST_TEST(number20 == 0);
}
