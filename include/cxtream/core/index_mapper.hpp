/****************************************************************************
 *  cxtream library
 *  Copyright (c) 2017, Cognexa Solutions s.r.o.
 *  Author(s) Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete license agreement.
 ****************************************************************************/

#ifndef CXTREAM_CORE_INDEX_MAPPER_HPP
#define CXTREAM_CORE_INDEX_MAPPER_HPP

#include <cxtream/core/utility/string.hpp>
#include <cxtream/core/utility/tuple.hpp>

#include <range/v3/view/transform.hpp>

#include <unordered_map>
#include <vector>

namespace cxtream {

/// Provides a bidirectional access from values to their indices in an std::vector.
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

    /// Returns the index of the given value. Throws std::out_of_range if the value does not exist.
    std::size_t index_for(const T& val) const
    {
        return val2idx_.at(val);
    }

    /// Returns the index of the given value or a default value if it does not exist.
    std::size_t index_for(const T& val, std::size_t defval) const
    {
        auto pos = val2idx_.find(val);
        if (pos == val2idx_.end()) return defval;
        return pos->second;
    }

    /// Returns the value at the given index.
    const T& at(const std::size_t& idx) const
    {
        return idx2val_.at(idx);
    }

    /// Returns the indexes of the given values. Throws std::out_of_range if any value does not exist.
    std::vector<std::size_t> index_for(const std::vector<T>& vals) const
    {
        return vals
          | ranges::view::transform([this](const T& val) {
                return this->index_for(val);
            });
    }

    /// Returns the indexes of the given values or a default value if they do not exist.
    std::vector<std::size_t> index_for(const std::vector<T>& vals, std::size_t defval) const
    {
        return vals
          | ranges::view::transform([this, defval](const T& val) {
                return this->index_for(val, defval);
            });
    }

    /// Returns the values at the given indexes.
    std::vector<T> at(const std::vector<std::size_t>& idxs) const
    {
        return idxs
          | ranges::view::transform([this](std::size_t idx) {
                return this->at(idx);
            });
    }

    /// Checks whether the mapper contains the given value.
    bool contains(const T& val) const
    {
        return val2idx_.count(val);
    }

    /// Inserts a value into the mapper with index size()-1.
    std::size_t insert(T val)
    {
        assert(!contains(val) && "Index mapper cannot insert already contained value.");
        val2idx_[val] = idx2val_.size();
        idx2val_.emplace_back(std::move(val));
        return idx2val_.size() - 1;
    }

    /// Returns the size of the mapper.
    std::size_t size() const
    {
        return val2idx_.size();
    }

    /// Returns all the contained values.
    const std::vector<T>& values() const
    {
        return idx2val_;
    }

private:
    std::unordered_map<T, std::size_t> val2idx_;
    std::vector<T> idx2val_;
};

}  // end namespace cxtream
#endif
