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
#define BOOST_TEST_MODULE csv_test

#include <iostream>
#include <sstream>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <cxtream/core/csv.hpp>

#include "common.hpp"

using namespace ranges;
using namespace cxtream;
using namespace boost;


const std::string simple_csv{
  "Id,  A,   B \n"
  " 1, a1, 1.1 \n"
  " 2, a2, 1.2 \n"
  " 3, a3, 1.3 \n"
};


BOOST_AUTO_TEST_CASE(test_read_csv_from_istream)
{
  std::istringstream simple_csv_ss{simple_csv};
  const dataframe<> df = read_csv(simple_csv_ss);
  BOOST_TEST(df.n_cols() == 3);
  BOOST_TEST(df.n_rows() == 3);
  test_ranges_equal(df.header(), std::vector<std::string>{"Id", "A", "B"});
  test_ranges_equal(df.raw_cols()[0], std::vector<std::string>{"1", "2", "3"});
  test_ranges_equal(df.raw_cols()[1], std::vector<std::string>{"a1", "a2", "a3"});
  test_ranges_equal(df.raw_cols()[2], std::vector<std::string>{"1.1", "1.2", "1.3"});
}


BOOST_AUTO_TEST_CASE(test_read_csv_from_istream_no_header)
{
  std::istringstream simple_csv_ss{simple_csv};
  const dataframe<> df = read_csv(simple_csv_ss, 1, false);
  BOOST_TEST(df.n_cols() == 3);
  BOOST_TEST(df.n_rows() == 3);
  BOOST_TEST(df.header().empty());
  test_ranges_equal(df.raw_cols()[0], std::vector<std::string>{"1", "2", "3"});
  test_ranges_equal(df.raw_cols()[1], std::vector<std::string>{"a1", "a2", "a3"});
  test_ranges_equal(df.raw_cols()[2], std::vector<std::string>{"1.1", "1.2", "1.3"});
}


BOOST_AUTO_TEST_CASE(test_read_quoted_csv_from_istream)
{
  const std::string quoted_csv{
    R"(  "Column, 1", )" "\t" R"("Column, 2"  , " Column \"3\" ")""\n"
    R"(Field 1, "Field, 2"  , " Field 3 "    )""\n"
    R"("Field 1",   "Field, 2 " ,    " Field 3 "    )""\n"
  };
  std::istringstream simple_csv_ss{quoted_csv};
  const dataframe<> df = read_csv(simple_csv_ss);
  BOOST_TEST(df.n_cols() == 3);
  BOOST_TEST(df.n_rows() == 2);
  test_ranges_equal(df.header(), std::vector<std::string>{"Column, 1", "Column, 2", " Column \"3\" "});
  test_ranges_equal(df.raw_rows()[0], std::vector<std::string>{"Field 1", "Field, 2", " Field 3 "});
  test_ranges_equal(df.raw_rows()[1], std::vector<std::string>{"Field 1", "Field, 2 ", " Field 3 "});
}
