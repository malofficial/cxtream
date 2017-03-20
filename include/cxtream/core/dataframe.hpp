/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_DATAFRAME_HPP
#define CXTREAM_CORE_DATAFRAME_HPP

#include <functional>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <range/v3/view/iota.hpp>
#include <range/v3/view/shared.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <cxtream/core/utility/string.hpp>
#include <cxtream/core/utility/tuple.hpp>

namespace cxtream {

  namespace {
    namespace view = ranges::view;
  }


  /* bidirectional index mapper */


  template<typename T>
  class index_mapper
  {
    public:

      index_mapper() = default;

      index_mapper(std::vector<T> values)
        : idx2val_{std::move(values)}
      {
        for (std::size_t i = 0; i < idx2val_.size(); ++i) {
          val2idx_[idx2val_[i]] = i;
        }
        assert(val2idx_.size() == idx2val_.size() && "Index mapper needs unique values.");
      }

      const std::size_t& val2idx(const T& val) const
      {
        return val2idx_.at(val);
      }

      const T& idx2val(const std::size_t& idx) const
      {
        return idx2val_.at(idx);
      }

      bool contains(const T& val) const
      {
        return val2idx_.count(val);
      }

      std::size_t insert(T val)
      {
        assert(!contains(val) && "Index mapper cannot insert already contained value.");
        val2idx_[val] = idx2val_.size();
        idx2val_.emplace_back(std::move(val));
        return idx2val_.size() - 1;
      }

      std::size_t size() const
      {
        return val2idx_.size();
      }

      std::vector<T> values() const
      {
        return idx2val_;
      }

    private:
      std::unordered_map<T, std::size_t> val2idx_;
      std::vector<T> idx2val_;
  };


  template<typename DataTable = std::vector<std::vector<std::string>>>
  class dataframe
  {
    public:


      /* default and data constructors */


      dataframe() = default;

      dataframe(DataTable data, std::vector<std::string> header = {})
        : data_{std::move(data)},
          header_{std::move(header)}
      { }


      /* typed column constructor */


      template<typename... Ts>
      dataframe(std::tuple<Ts...> columns, std::vector<std::string> header = {})
      {
        assert(header.empty() || header.size() == sizeof...(Ts));
        utility::tuple_for_each_with_index(
          [this, &header](auto&& column, auto index){
            std::string col_name = header.empty() ? "" : std::move(header[index]);
            this->col_insert(std::move(column), std::move(col_name));
          }, std::move(columns)
        );
      }


      /* column insertion */


      template<typename Rng,
               typename ToStrFun = std::string(*)(ranges::range_value_t<Rng>)>
      std::size_t col_insert(
        Rng rng,
        std::string col_name = {},
        std::function<std::string(ranges::range_value_t<Rng>)> cvt =
          static_cast<ToStrFun>(utility::to_string))
      {
        assert(n_rows() == 0 || ranges::size(rng) == n_rows());
        assert((!header_.size() || col_name.size()) &&
          "The dataframe has a header, please provide the name of column.");
        if (col_name.size())
          header_.insert(col_name);
        data_.emplace_back(rng | view::transform(cvt));
        return n_cols() - 1;
      }


      /* row insertion */


      template<typename... Ts>
      std::size_t row_insert(
        std::tuple<Ts...> row_tuple,
        std::tuple<std::function<std::string(Ts)>...> cvts =
          std::make_tuple(static_cast<std::string(*)(Ts)>(utility::to_string)...))
      {
        assert(n_cols() == 0 || sizeof...(Ts) == n_cols());
        utility::tuple_for_each_with_index(
          [this, &cvts](auto&& field, auto index){
            this->data_[index].emplace_back(
              std::get<index>(cvts)(field));
          }, std::move(row_tuple)
        );
        return n_rows() - 1;
      }


      /* column drop */


      auto icol_drop(std::size_t col_index)
      {
        assert(col_index < n_cols());
        // remove the column from the header
        if (header_.size()) {
          std::vector<std::string> new_header = header_.values();
          new_header.erase(new_header.begin() + col_index);
          header_ = new_header;
        }
        // remove the column from the data
        data_.erase(data_.begin() + col_index);
      }


      auto col_drop(const std::string& col_name)
      {
        assert(header_.size() && "Dataframe has no header, cannot drop by column name.");
        return icol_drop(header_.val2idx(col_name));
      }


      /* row drop */


      auto row_drop(const std::size_t row_idx)
      {
        assert(row_idx < n_rows());
        for (auto& column : data_) {
          column.erase(column.begin() + row_idx);
        }
      }


      /* raw single column access */


      auto raw_icol(std::size_t col_index)
      {
        return raw_cols()[col_index] | view::all;
      }

      auto raw_icol(std::size_t col_index) const
      {
        return raw_cols()[col_index] | view::all;
      }


      auto raw_col(const std::string& col_name)
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_icol(header_.val2idx(col_name));
      }

      auto raw_col(const std::string& col_name) const
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_icol(header_.val2idx(col_name));
      }


      /* typed single column access */


      template<typename T>
      auto icol(std::size_t col_index,
          std::function<T(std::string)> cvt = utility::string_to<T>) const
      {
        return raw_icol(col_index) | view::transform(cvt);
      }

      template<typename T>
      auto col(const std::string& col_name,
          std::function<T(std::string)> cvt = utility::string_to<T>) const
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return icol<T>(header_.val2idx(col_name), std::move(cvt));
      }


      /* raw multi column access */


      auto raw_cols()
      {
        return data_ | view::transform(view::all);
      }

      auto raw_cols() const
      {
        return data_ | view::transform(view::all);
      }

      auto raw_icols(std::vector<std::size_t> col_indexes)
      {
        return raw_icols_impl(this, std::move(col_indexes));
      }

      auto raw_icols(std::vector<std::size_t> col_indexes) const
      {
        return raw_icols_impl(this, std::move(col_indexes));
      }

      auto raw_cols(const std::vector<std::string>& col_names)
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_icols(colnames2idxs(col_names));
      }

      auto raw_cols(const std::vector<std::string>& col_names) const
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_icols(colnames2idxs(col_names));
      }


      /* typed multi column access */


      template<typename... Ts>
      auto icols(std::vector<std::size_t> col_indexes,
                 std::tuple<std::function<Ts(std::string)>...> cvts =
                   std::make_tuple(utility::string_to<Ts>...)) const
      {
        assert(sizeof...(Ts) == ranges::size(col_indexes));
        return utility::tuple_transform_with_index(
          [raw_cols = raw_icols(std::move(col_indexes))](auto&& cvt, auto i){
            return raw_cols[i] | ranges::view::transform(std::move(cvt));
          }, std::move(cvts)
        );
      }

      template<typename... Ts>
      auto cols(const std::vector<std::string>& col_names,
                std::tuple<std::function<Ts(std::string)>...> cvts =
                  std::make_tuple(utility::string_to<Ts>...)) const
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return icols<Ts...>(colnames2idxs(col_names), std::move(cvts));
      }


      /* raw row access */


      auto raw_rows()
      {
        return raw_rows_impl(this);
      }

      auto raw_rows() const
      {
        return raw_rows_impl(this);
      }

      auto raw_irows(std::vector<std::size_t> col_indexes)
      {
        return raw_irows_impl(this, std::move(col_indexes));
      }

      auto raw_irows(std::vector<std::size_t> col_indexes) const
      {
        return raw_irows_impl(this, std::move(col_indexes));
      }

      auto raw_rows(const std::vector<std::string>& col_names)
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_irows(colnames2idxs(col_names));
      }

      auto raw_rows(const std::vector<std::string>& col_names) const
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_irows(colnames2idxs(col_names));
      }


      /* typed row access */


      template<typename... Ts>
      auto irows(std::vector<std::size_t> col_indexes,
                 std::tuple<std::function<Ts(std::string)>...> cvts =
                   std::make_tuple(utility::string_to<Ts>...)) const
      {
        return std::experimental::apply(
          view::zip,
          icols<Ts...>(std::move(col_indexes), std::move(cvts))
        );
      }

      template<typename... Ts>
      auto rows(const std::vector<std::string>& col_names,
                std::tuple<std::function<Ts(std::string)>...> cvts =
                  std::make_tuple(utility::string_to<Ts>...)) const
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return irows<Ts...>(colnames2idxs(col_names), std::move(cvts));
      }


      /* typed indexed single row access */


      template<typename IndexT, typename ColT>
      std::unordered_map<IndexT, ColT>
      index_irow(std::size_t index_col_index,
                 std::size_t col_index,
                 std::function<IndexT(std::string)> index_cvt =
                   utility::string_to<IndexT>,
                 std::function<ColT(std::string)> col_cvt =
                   utility::string_to<ColT>) const
      {
        auto index_col = icol<IndexT>(index_col_index, std::move(index_cvt));
        auto col = icol<ColT>(col_index, std::move(col_cvt));
        return view::zip(index_col, col);
      }


      template<typename IndexT, typename ColT>
      std::unordered_map<IndexT, ColT>
      index_row(const std::string& index_col_name,
                const std::string& col_name,
                std::function<IndexT(std::string)> index_cvt =
                  utility::string_to<IndexT>,
                std::function<ColT(std::string)> col_cvt =
                  utility::string_to<ColT>) const
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return index_irow(
          header_.val2idx(index_col_name),
          header_.val2idx(col_name),
          std::move(index_cvt),
          std::move(col_cvt)
        );
      }


      /* typed indexed multiple row access */


      template<typename IndexT, typename... Ts>
      std::unordered_map<IndexT, std::tuple<Ts...>>
      index_irows(std::size_t index_col_index,
                  std::vector<std::size_t> col_indexes,
                  std::function<IndexT(std::string)> index_cvt =
                    utility::string_to<IndexT>,
                  std::tuple<std::function<Ts(std::string)>...> col_cvts =
                    std::make_tuple(utility::string_to<Ts>...)) const
      {
        auto index_col = icol<IndexT>(index_col_index, std::move(index_cvt));
        auto rows = irows<Ts...>(std::move(col_indexes), std::move(col_cvts));
        return view::zip(index_col, rows);
      }


      template<typename IndexT, typename... Ts>
      std::unordered_map<IndexT, std::tuple<Ts...>>
      index_rows(const std::string& index_col_name,
                 const std::vector<std::string>& col_names,
                 std::function<IndexT(std::string)> index_cvt =
                   utility::string_to<IndexT>,
                 std::tuple<std::function<Ts(std::string)>...> col_cvts =
                   std::make_tuple(utility::string_to<Ts>...)) const
      {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return index_irows(
          header_.val2idx(index_col_name),
          colnames2idxs(col_names),
          std::move(index_cvt),
          std::move(col_cvts)
        );
      }


      /* shape functions */


      std::size_t n_cols() const
      {
        return data_.size();
      }

      std::size_t n_rows() const
      {
        if (n_cols() == 0)
          return 0;
        return data_.front().size();
      }


      /* column_names */


      std::vector<std::string> header() const
      {
        return header_.values();
      }


    private:


      template<typename This>
      static auto raw_irows_impl(This this_ptr, std::vector<std::size_t> col_indexes)
      {
        return view::iota(0UL, this_ptr->n_rows())
          | view::transform([this_ptr, col_indexes=std::move(col_indexes)](std::size_t i) {
              return this_ptr->raw_icols(col_indexes)
                // decltype(auto) to make sure a reference is returned
                | view::transform([i](auto&& col) -> decltype(auto){
                    return col[i];
                  });
            });
      }

      template<typename This>
      static auto raw_rows_impl(This this_ptr)
      {
        return view::iota(0UL, this_ptr->n_rows())
          | view::transform([this_ptr](std::size_t i) {
              return view::iota(0UL, this_ptr->n_cols())
                // decltype(auto) to make sure a reference is returned
                | view::transform([this_ptr, i](std::size_t j) -> decltype(auto){
                    return this_ptr->raw_cols()[j][i];
                  });
            });
      }

      template<typename... Ts>
      std::vector<std::size_t> colnames2idxs(const std::vector<std::string>& col_names) const
      {
        return col_names
          | view::transform([this](const std::string& name){
              return this->header_.val2idx(name);
        });
      }

      template<typename This>
      static auto raw_icols_impl(This this_ptr, std::vector<std::size_t> col_indexes)
      {
        return std::move(col_indexes)
          | view::shared
          | view::transform([this_ptr](std::size_t idx){
              return this_ptr->raw_cols()[idx];
            });
      }


      /* data storage */


      // data
      DataTable data_;

      using header_t = index_mapper<std::string>;
      header_t header_;

  }; // end class dataframe


  template<typename DataTable>
  std::ostream& operator<<(std::ostream& out, const dataframe<DataTable>& df)
  {
    // calculate the width of the columns using their longest field
    std::vector<std::size_t> col_widths =
        df.raw_cols()
      | view::transform([](auto&& col){
          std::vector<std::size_t> elem_sizes = col | view::transform(&std::string::size);
          return ranges::max(elem_sizes) + 2;
        });

    auto header = df.header();
    if (header.size()) {
      // update col_widths using header widths
      col_widths = view::zip(col_widths, header)
        | view::transform([](auto&& tpl){
            return std::max(std::get<0>(tpl), std::get<1>(tpl).size() + 2);
          });

      // print header
      for (std::size_t j = 0; j < header.size(); ++j) {
        out << std::setw(col_widths[j]) << header[j];
        if (j + 1 < header.size()) {
          out << '|';
        } else {
          out << '\n';
        }
      }

      // print header and data separator
      for (std::size_t j = 0; j < header.size(); ++j) {
        out << std::setw(col_widths[j]) << std::setfill('-');
        if (j + 1 < header.size()) {
          out << '-' << '+';
        } else {
          out << '-' << '\n';
        }
      }
      out << std::setfill(' ');
    }

    // print data
    for (std::size_t i = 0; i < df.n_rows(); ++i) {
      for (std::size_t j = 0; j < df.n_cols(); ++j) {
        out << std::setw(col_widths[j]) << df.raw_rows()[i][j];
        if (j + 1 < df.n_cols()) {
          out << '|';
        } else {
          out << '\n';
        }
      }
    }

    return out;
  }

} // end namespace cxtream
#endif
