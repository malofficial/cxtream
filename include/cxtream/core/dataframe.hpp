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

#include <cxtream/core/utility/string.hpp>
#include <cxtream/core/utility/tuple.hpp>

#include <range/v3/view/all.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/shared.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <functional>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace cxtream {

/// Provides a bidirectional access from values to their indices in a vector.
template<typename T>
class index_mapper {
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

/// Tabular object with convenient data access methods.
///
/// By default, all fields are stored as std::string and they are
/// cast to the requested type on demand.
template<typename DataTable = std::vector<std::vector<std::string>>>
class dataframe {
public:
    dataframe() = default;

    /// Constructs the dataset from a vector of columns of the same type.
    template<typename T>
    dataframe(std::vector<std::vector<T>> columns, std::vector<std::string> header = {})
    {
        assert(header.empty() || header.size() == columns.size());
        for (std::size_t i = 0; i < columns.size(); ++i) {
            std::string col_name = header.empty() ? "" : std::move(header[i]);
            col_insert(std::move(columns[i]), std::move(col_name));
        }
    }

    /// Constructs the dataset from a tuple of columns of possibly different types.
    template<typename... Ts>
    dataframe(std::tuple<Ts...> columns, std::vector<std::string> header = {})
    {
        assert(header.empty() || header.size() == sizeof...(Ts));
        utility::tuple_for_each_with_index(
          [this, &header](auto&& column, auto index) {
              std::string col_name = header.empty() ? "" : std::move(header[index]);
              this->col_insert(std::move(column), std::move(col_name));
          },
          std::move(columns));
    }

    // insertion //

    /// Inserts a new column to the dataframe.
    template<typename Rng, typename ToStrFun = std::string (*)(ranges::range_value_t<Rng>)>
    std::size_t col_insert(Rng rng, std::string col_name = {},
                           std::function<std::string(ranges::range_value_t<Rng>)> cvt =
                             static_cast<ToStrFun>(utility::to_string))
    {
        assert(n_rows() == 0 || ranges::size(rng) == n_rows());
        assert((!header_.size() || col_name.size()) &&
               "The dataframe has a header, please provide the name of column.");
        if (col_name.size()) header_.insert(col_name);
        data_.emplace_back(rng | ranges::view::transform(cvt));
        return n_cols() - 1;
    }

    /// Inserts a new row to the dataframe.
    template<typename... Ts>
    std::size_t row_insert(std::tuple<Ts...> row_tuple,
                           std::tuple<std::function<std::string(Ts)>...> cvts = std::make_tuple(
                             static_cast<std::string (*)(Ts)>(utility::to_string)...))
    {
        assert(n_cols() == 0 || sizeof...(Ts) == n_cols());
        utility::tuple_for_each_with_index(
          [this, &cvts](auto&& field, auto index) {
              this->data_[index].emplace_back(std::get<index>(cvts)(field));
          },
          std::move(row_tuple));
        return n_rows() - 1;
    }

    // drop //

    /// Drop a column.
    void icol_drop(std::size_t col_index)
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

    /// Drop a column.
    void col_drop(const std::string& col_name)
    {
        assert(header_.size() && "Dataframe has no header, cannot drop by column name.");
        return icol_drop(header_.val2idx(col_name));
    }

    /// Drop a row.
    void row_drop(const std::size_t row_idx)
    {
        assert(row_idx < n_rows());
        for (auto& column : data_) {
            column.erase(column.begin() + row_idx);
        }
    }

    // raw column access //

    /// Return a raw view of a column.
    ///
    /// \returns A of range of std::string&.
    auto raw_icol(std::size_t col_index)
    {
        return raw_cols()[col_index] | ranges::view::all;
    }

    /// Return a raw view of a column.
    ///
    /// \returns A of range of const std::string&.
    auto raw_icol(std::size_t col_index) const
    {
        return raw_cols()[col_index] | ranges::view::all;
    }

    /// Return a raw view of a column.
    ///
    /// \returns A of range of std::string&.
    auto raw_col(const std::string& col_name)
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_icol(header_.val2idx(col_name));
    }

    /// Return a raw view of a column.
    ///
    /// \returns A of range of const std::string&.
    auto raw_col(const std::string& col_name) const
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_icol(header_.val2idx(col_name));
    }

    // typed column access //

    /// Return a typed view of a column.
    ///
    /// By default, this function does not provide a direct access to the stored data.
    /// Instead, each field is converted to the type T and a copy is returned.
    ///
    /// \returns A range of T.
    template<typename T>
    auto icol(std::size_t col_index,
              std::function<T(std::string)> cvt = utility::string_to<T>) const
    {
        return raw_icol(col_index) | ranges::view::transform(cvt);
    }

    /// Return a typed view of a column.
    ///
    /// By default, this function does not provide a direct access to the stored data.
    /// Instead, each field is converted to the type T and a copy is returned.
    ///
    /// \returns A range of T.
    template<typename T>
    auto col(const std::string& col_name,
             std::function<T(std::string)> cvt = utility::string_to<T>) const
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return icol<T>(header_.val2idx(col_name), std::move(cvt));
    }

    // raw multi column access //

    /// Return a raw view of multiple columns.
    ///
    /// \returns A range of ranges of std::string&.
    auto raw_cols()
    {
        return data_ | ranges::view::transform(ranges::view::all);
    }

    /// Return a raw view of multiple columns.
    ///
    /// \returns A range of ranges of const std::string&.
    auto raw_cols() const
    {
        return data_ | ranges::view::transform(ranges::view::all);
    }

    /// Return a raw view of multiple columns.
    ///
    /// \returns A range of ranges of std::string&.
    auto raw_icols(std::vector<std::size_t> col_indexes)
    {
        return raw_icols_impl(this, std::move(col_indexes));
    }

    /// Return a raw view of multiple columns.
    ///
    /// \returns A range of ranges of const std::string&.
    auto raw_icols(std::vector<std::size_t> col_indexes) const
    {
        return raw_icols_impl(this, std::move(col_indexes));
    }

    /// Return a raw view of multiple columns.
    ///
    /// \returns A range of ranges of std::string&.
    auto raw_cols(const std::vector<std::string>& col_names)
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_icols(colnames2idxs(col_names));
    }

    /// Return a raw view of multiple columns.
    ///
    /// \returns A range of ranges of const std::string&.
    auto raw_cols(const std::vector<std::string>& col_names) const
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_icols(colnames2idxs(col_names));
    }

    // typed multi column access //

    /// Return a typed view of multiple columns.
    ///
    /// \returns A tuple of ranges of Ts.
    template<typename... Ts>
    auto icols(std::vector<std::size_t> col_indexes,
               std::tuple<std::function<Ts(std::string)>...> cvts =
                 std::make_tuple(utility::string_to<Ts>...)) const
    {
        assert(sizeof...(Ts) == ranges::size(col_indexes));
        return utility::tuple_transform_with_index(
          [raw_cols = raw_icols(std::move(col_indexes))](auto&& cvt, auto i) {
              return raw_cols[i] | ranges::view::transform(std::move(cvt));
          },
          std::move(cvts));
    }

    /// Return a typed view of multiple columns.
    ///
    /// \returns A tuple of ranges of Ts.
    template<typename... Ts>
    auto cols(const std::vector<std::string>& col_names,
              std::tuple<std::function<Ts(std::string)>...> cvts =
                std::make_tuple(utility::string_to<Ts>...)) const
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return icols<Ts...>(colnames2idxs(col_names), std::move(cvts));
    }

    /// Return a raw view of multiple rows.
    ///
    /// \returns A range of ranges of std::string&.
    auto raw_rows()
    {
        return raw_rows_impl(this);
    }

    /// Return a raw view of multiple rows.
    ///
    /// \returns A range of ranges of const std::string&.
    auto raw_rows() const
    {
        return raw_rows_impl(this);
    }

    /// Return a raw view of multiple rows.
    ///
    /// \returns A range of ranges of std::string&.
    auto raw_irows(std::vector<std::size_t> col_indexes)
    {
        return raw_irows_impl(this, std::move(col_indexes));
    }

    /// Return a raw view of multiple rows.
    ///
    /// \returns A range of ranges of const std::string&.
    auto raw_irows(std::vector<std::size_t> col_indexes) const
    {
        return raw_irows_impl(this, std::move(col_indexes));
    }

    /// Return a raw view of multiple rows.
    ///
    /// \returns A range of ranges of std::string&.
    auto raw_rows(const std::vector<std::string>& col_names)
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_irows(colnames2idxs(col_names));
    }

    /// Return a raw view of multiple rows.
    ///
    /// \returns A range of ranges of const std::string&.
    auto raw_rows(const std::vector<std::string>& col_names) const
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return raw_irows(colnames2idxs(col_names));
    }

    // typed row access //

    /// Return a typed view of multiple rows.
    ///
    /// \returns A range of tuple of Ts.
    template<typename... Ts>
    auto irows(std::vector<std::size_t> col_indexes,
               std::tuple<std::function<Ts(std::string)>...> cvts =
                 std::make_tuple(utility::string_to<Ts>...)) const
    {
        return std::experimental::apply(
          ranges::view::zip,
          icols<Ts...>(std::move(col_indexes), std::move(cvts)));
    }

    /// Return a typed view of multiple rows.
    ///
    /// \returns A range of tuple of Ts.
    template<typename... Ts>
    auto rows(const std::vector<std::string>& col_names,
              std::tuple<std::function<Ts(std::string)>...> cvts =
                std::make_tuple(utility::string_to<Ts>...)) const
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return irows<Ts...>(colnames2idxs(col_names), std::move(cvts));
    }

    // typed indexed single column access //

    /// Return an indexed typed view of a single column.
    ///
    /// This function returns a hashmap indexed by a given key column with values of another column.
    ///
    /// Example:
    /// \code
    ///     std::unordered_map<int, double> mapper = df.index_icol<int, double>(0, 1);
    /// \endcode
    ///
    /// \param key_col_index Index of the column to be used as key.
    /// \param val_col_index Index of the column to be used as value.
    /// \returns A hashmap.
    template <typename IndexT, typename ColT>
    std::unordered_map<IndexT, ColT>
    index_icol(std::size_t key_col_index,
               std::size_t val_col_index,
               std::function<IndexT(std::string)> key_col_cvt = utility::string_to<IndexT>,
               std::function<ColT(std::string)> val_col_cvt = utility::string_to<ColT>) const
    {
        auto key_col = icol<IndexT>(key_col_index, std::move(key_col_cvt));
        auto val_col = icol<ColT>(val_col_index, std::move(val_col_cvt));
        return ranges::view::zip(key_col, val_col);
    }

    /// Return an indexed typed view of a single column.
    ///
    /// This function is the same as index_icol, but columns are selected by name.
    template<typename IndexT, typename ColT>
    std::unordered_map<IndexT, ColT>
    index_col(const std::string& key_col_name,
              const std::string& val_col_name,
              std::function<IndexT(std::string)> key_col_cvt = utility::string_to<IndexT>,
              std::function<ColT(std::string)> val_col_cvt = utility::string_to<ColT>) const
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return index_icol(header_.val2idx(key_col_name),
                          header_.val2idx(val_col_name),
                          std::move(key_col_cvt),
                          std::move(val_col_cvt));
    }

    // typed indexed multiple column access //

    /// Return an indexed typed view of multiple columns.
    ///
    /// This function is similar to index_icol, but value type is a tuple of Ts.
    template<typename IndexT, typename... Ts>
    std::unordered_map<IndexT, std::tuple<Ts...>>
    index_icols(std::size_t key_col_index,
                std::vector<std::size_t> val_col_indexes,
                std::function<IndexT(std::string)> key_col_cvt =
                  utility::string_to<IndexT>,
                std::tuple<std::function<Ts(std::string)>...> val_col_cvts =
                  std::make_tuple(utility::string_to<Ts>...)) const
    {
        auto key_col = icol<IndexT>(key_col_index, std::move(key_col_cvt));
        auto val_cols = irows<Ts...>(std::move(val_col_indexes), std::move(val_col_cvts));
        return ranges::view::zip(key_col, val_cols);
    }


    /// Return an indexed typed view of multiple columns.
    ///
    /// This function is similar to index_icols, columns are selected by name.
    template<typename IndexT, typename... Ts>
    std::unordered_map<IndexT, std::tuple<Ts...>>
    index_cols(const std::string& key_col_name,
               const std::vector<std::string>& val_col_names,
               std::function<IndexT(std::string)> key_col_cvt =
                 utility::string_to<IndexT>,
               std::tuple<std::function<Ts(std::string)>...> val_col_cvts =
                 std::make_tuple(utility::string_to<Ts>...)) const
    {
        assert(header_.size() && "Dataframe has no header, cannot index by column name.");
        return index_icols(header_.val2idx(key_col_name),
                           colnames2idxs(val_col_names),
                           std::move(key_col_cvt),
                           std::move(val_col_cvts));
    }

    // shape functions //

    /// Returns the number of columns.
    std::size_t n_cols() const
    {
        return data_.size();
    }

    /// Returns the number of rows (excluding header).
    std::size_t n_rows() const
    {
        if (n_cols() == 0) return 0;
        return data_.front().size();
    }

    /// Returns the names of columns.
    std::vector<std::string> header() const
    {
        return header_.values();
    }

private:
    template <typename This>
    static auto raw_irows_impl(This this_ptr, std::vector<std::size_t> col_indexes)
    {
        namespace view = ranges::view;
        return view::iota(0UL, this_ptr->n_rows())
          | view::transform([this_ptr, col_indexes=std::move(col_indexes)](std::size_t i) {
                return this_ptr->raw_icols(col_indexes)
                  // decltype(auto) to make sure a reference is returned
                  | view::transform([i](auto&& col) -> decltype(auto) {
                        return col[i];
                    });
            });
      }

      template<typename This>
      static auto raw_rows_impl(This this_ptr)
      {
        namespace view = ranges::view;
        return view::iota(0UL, this_ptr->n_rows())
          | view::transform([this_ptr](std::size_t i) {
                return view::iota(0UL, this_ptr->n_cols())
                  // decltype(auto) to make sure a reference is returned
                  | view::transform([this_ptr, i](std::size_t j) -> decltype(auto) {
                        return this_ptr->raw_cols()[j][i];
                    });
            });
      }

      template<typename... Ts>
      std::vector<std::size_t> colnames2idxs(const std::vector<std::string>& col_names) const
      {
        return col_names
          | ranges::view::transform([this](const std::string& name) {
                return this->header_.val2idx(name);
        });
      }

      template<typename This>
      static auto raw_icols_impl(This this_ptr, std::vector<std::size_t> col_indexes)
      {
        return std::move(col_indexes)
          | ranges::view::shared
          | ranges::view::transform([this_ptr](std::size_t idx) {
                return this_ptr->raw_cols()[idx];
            });
      }


      // data storage //

      DataTable data_;

      using header_t = index_mapper<std::string>;
      header_t header_;

};  // class dataframe

/// Pretty printing of dataframe to std::ostream.
template<typename DataTable>
std::ostream& operator<<(std::ostream& out, const dataframe<DataTable>& df)
{
    namespace view = ranges::view;
    // calculate the width of the columns using their longest field
    std::vector<std::size_t> col_widths =
        df.raw_cols()
      | view::transform([](auto&& col) {
            std::vector<std::size_t> elem_sizes = col | view::transform(&std::string::size);
            return ranges::max(elem_sizes) + 2;
        });

    auto header = df.header();
    if (header.size()) {
        // update col_widths using header widths
        col_widths = view::zip(col_widths, header)
          | view::transform([](auto&& tpl) {
                return std::max(std::get<0>(tpl), std::get<1>(tpl).size() + 2);
            });

        // print header
        for (std::size_t j = 0; j < header.size(); ++j) {
            out << std::setw(col_widths[j]) << header[j];
            if (j + 1 < header.size()) out << '|';
            else out << '\n';
        }

        // print header and data separator
        for (std::size_t j = 0; j < header.size(); ++j) {
            out << std::setw(col_widths[j]) << std::setfill('-');
            if (j + 1 < header.size()) out << '-' << '+';
            else out << '-' << '\n';
        }
        out << std::setfill(' ');
    }

    // print data
    for (std::size_t i = 0; i < df.n_rows(); ++i) {
        for (std::size_t j = 0; j < df.n_cols(); ++j) {
            out << std::setw(col_widths[j]) << df.raw_rows()[i][j];
            if (j + 1 < df.n_cols()) out << '|';
            else out << '\n';
        }
    }

    return out;
}

} // end namespace cxtream
#endif
