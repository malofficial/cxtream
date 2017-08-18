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
#define BOOST_TEST_MODULE transform2_test

#include "transform.hpp"

using namespace cxtream::stream;

BOOST_AUTO_TEST_CASE(test_probabilistic_simple)
{
    CXTREAM_DEFINE_COLUMN(dogs, int)
    std::vector<int> data = {3, 1, 5, 7, 2, 13};
    auto rng = data
      | create<dogs>()
      | transform(from<dogs>, to<dogs>, 1.0, [](int dog) { return 1; }, prng)
      | transform(from<dogs>, to<dogs>, 0.5, [](int dog) { return 2; }, prng)
      | transform(from<dogs>, to<dogs>, 0.0, [](int dog) { return 3; }, prng);

    std::vector<int> generated = unpack(rng, from<dogs>, dim<1>);
    BOOST_CHECK(generated.size() == 6);
    long number1 = ranges::count(generated, 1);
    long number2 = ranges::count(generated, 2);
    long number3 = ranges::count(generated, 3);
    BOOST_TEST(number1 >= 1);
    BOOST_TEST(number1 <= 5);
    BOOST_TEST(number1 == 6 - number2);
    BOOST_TEST(number3 == 0);
}
