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

#include <range/v3/algorithm/find_first_of.hpp>
#include <range/v3/getlines.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/transform.hpp>

#include <cxtream/core/dataframe.hpp>
#include <cxtream/core/utility/string.hpp>

namespace cxtream {


  /* istream csv row parser */

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


  /* istream csv parser */


  dataframe<> read_csv(
    std::istream& in,
    int drop = 0,
    bool has_header = true,
    char separator = ',')
  {
    // header
    std::vector<std::string> header;
    // data
    std::vector<std::vector<std::string>> data;

    // load csv line by line
    auto csv_rows =
       ranges::getlines(in)
     | ranges::view::drop(drop)
     | ranges::view::transform([separator](std::string line){
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


  /* file csv parser */


  dataframe<> read_csv(
    const std::experimental::filesystem::path& file,
    int drop = 0,
    bool header = true,
    char separator = ',')
  {
    std::ifstream fin{file};
    fin.exceptions(std::ifstream::badbit);
    return read_csv(fin, drop, header, separator);
  }


  /* ostream csv row writer */


  template<typename Row>
  std::ostream& write_csv_row(
    std::ostream& out,
    Row&& row,
    char separator = ',')
  {
    for(std::size_t i = 0; i < ranges::size(row); ++i) {
      auto& field = row[i];
      // output quoted string if it contains separator, double quote or
      // starts or ends with a whitespace
      if (ranges::find_first_of(field, {separator, '"'}) != ranges::end(field)
          || field != utility::trim(field)) {
        out << std::quoted(field);
      } else {
        out << field;
      }

      // output separator or newline
      if (i + 1 < ranges::size(row)) {
        out << separator;
      } else {
        out << '\n';
      }
    }
    return out;
  }


  /* ostream csv writer */


  template<typename DataTable>
  std::ostream& write_csv(
    std::ostream& out,
    const dataframe<DataTable>& df,
    char separator = ',')
  {
    write_csv_row(out, df.header(), separator);
    for(auto&& row : df.raw_rows()) {
      write_csv_row(out, row, separator);
    }
    return out;
  }


  /* file csv writer */


  template<typename DataTable>
  void write_csv(
    const std::experimental::filesystem::path& file,
    const dataframe<DataTable>& df,
    char separator = ',')
  {
    std::ofstream fout{file};
    fout.exceptions(std::ofstream::badbit);
    write_csv(fout, df, separator);
  }


} // end namespace cxtream
#endif
