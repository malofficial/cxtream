/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_CSV_HPP
#define CXTREAM_CORE_CSV_HPP

#include <deque>
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <range/v3/getlines.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>

#include <cxtream/core/dataframe.hpp>
#include <cxtream/core/utility/string.hpp>

namespace cxtream {

  namespace {
    namespace fs = std::experimental::filesystem;
    namespace view = ranges::view;
  }


  /* stream csv parser */

  // beware, escaping double quotes is allowed using backslash, not another double quote
  // escaping is only allowed if the first character is double quote

  std::vector<std::string> parse_csv_row(
    std::string row,
    char separator = ',')
  {
    std::vector<std::string> csv_row;
    std::istringstream in{std::move(row)};
    while (in >> std::ws) {
      std::string field;
      if (in.peek() == '"') {
        in >> std::quoted(field);
        in.ignore(std::numeric_limits<std::streamsize>::max(), separator);
      } else {
        std::getline(in, field, separator);
        field = utility::trim(std::move(field));
      }
      csv_row.emplace_back(std::move(field));
      in >> std::ws;
      in.peek(); // to make sure the stream registers the end of line
    }
    return csv_row;
  }


  dataframe<> read_csv(
    std::istream& in,
    int drop = 0,
    bool has_header = true,
    char separator = ',')
  {
    // header
    std::vector<std::string> header;
    // data
    std::vector<std::deque<std::string>> data;

    // load csv line by line
    auto csv_rows =
       ranges::getlines(in)
     | view::drop(drop)
     | view::transform([separator](std::string line){
         return parse_csv_row(std::move(line), separator);
       });

    // make a row iterator
    auto csv_row_it = ranges::begin(csv_rows);
    std::size_t n_cols;

    // load header if requested
    if (has_header) {
      assert(csv_row_it != ranges::end(csv_rows) && "There has to be at least the header row.");
      std::vector<std::string> csv_row = *csv_row_it;
      n_cols = ranges::size(csv_row);
      header = std::move(csv_row);
      data.resize(n_cols);
      ++csv_row_it;
    }

    // load data
    for (std::size_t i = 0; csv_row_it != ranges::end(csv_rows); ++csv_row_it, ++i) {
      std::vector<std::string> csv_row = *csv_row_it;

      // sanity check row size
      if (i == 0) {
        if (has_header) {
          assert(ranges::size(csv_row) == n_cols && "The first row must have the same "
            "length as the header.");
        } else {
          n_cols = ranges::size(csv_row);
          data.resize(n_cols);
        }
      } else {
        assert(ranges::size(csv_row) == n_cols && "Rows must have the same length.");
      }

      // store columns
      for (std::size_t j = 0; j < ranges::size(csv_row); ++j) {
        data[j].emplace_back(std::move(csv_row[j]));
      }
    }

    return {std::move(data), std::move(header)};
  }


  dataframe<> read_csv(
    const fs::path& file,
    int drop = 0,
    bool header = true,
    char separator = ',')
  {
    std::ifstream fin{file};
    fin.exceptions(std::ifstream::failbit);
    return read_csv(fin, drop, header, separator);
  }


} // end namespace cxtream
#endif
