/*********************************************************
 *  cxtream library
 *
 *  Copyright (c) 2017, Filip Matzner
 *
 *  This file is distributed under the MIT License.
 *  See the accompanying file LICENSE.txt for the complete
 *  license agreement.
 *********************************************************/

#ifndef CXTREAM_CORE_BUFFER_VIEW_HPP
#define CXTREAM_CORE_BUFFER_VIEW_HPP

#include <deque>
#include <future>
#include <climits>
#include <range/v3/core.hpp>
#include <range/v3/view/all.hpp>

namespace cxtream {
  

  template<typename Rng>
  struct buffer_view
    : ranges::view_facade<buffer_view<Rng>>
  {
    private:
      friend ranges::range_access;

      Rng rng_;

      std::size_t n_;
      std::launch policy_;

      struct cursor
      {
        private:
          buffer_view<Rng> *rng_ = nullptr;
          ranges::range_iterator_t<Rng> it_ = {};

          std::size_t n_;
          std::launch policy_;

          std::deque<
            std::shared_future<
              ranges::range_value_t<Rng>>> buffer_;

          void pop_buffer()
          {
            if (!buffer_.empty()) {
              buffer_.pop_front();
            }
          }
          void fill_buffer()
          {
            while (it_ != ranges::end(rng_->rng_) && buffer_.size() < n_) {
              buffer_.emplace_back(std::async(policy_, [it=it_](){
                return *it;
              }));
              ++it_;
            }
          }

        public:
          cursor() = default;
          explicit cursor(buffer_view<Rng>& rng)
            : rng_{&rng},
              it_{ranges::begin(rng.rng_)},
              n_{rng.n_},
              policy_{rng.policy_}
          {
            fill_buffer();
          }
          decltype(auto) read() const
          {
            return buffer_.front().get();
          }
          bool equal(ranges::default_sentinel) const
          {
            return buffer_.empty() && it_ == ranges::end(rng_->rng_);
          }
          bool equal(const cursor& that) const
          {
            assert(rng_ == that.rng_);
            return n_ == that.n_ && it_ == that.it_;
          }
          void next()
          {
            pop_buffer();
            fill_buffer();
          }
      };

      cursor begin_cursor()
      {
        return cursor{*this};
      }

    public:
      buffer_view() = default;
      buffer_view(Rng rng, std::size_t n, std::launch policy)
        : rng_{rng}, n_{n}, policy_{policy}
      {}

      CONCEPT_REQUIRES(ranges::SizedRange<Rng const>())
      constexpr ranges::range_size_t<Rng> size() const
      {
        return ranges::size(rng_);
      }

      CONCEPT_REQUIRES(ranges::SizedRange<Rng>())
      constexpr ranges::range_size_t<Rng> size()
      {
        return ranges::size(rng_);
      }
  };


  class buffer_fn {

    private:

      friend ranges::view::view_access;

      static auto bind(
        buffer_fn buffer,
        std::size_t n = std::numeric_limits<std::size_t>::max(),
        std::launch policy = std::launch::async)
      {
        return ranges::make_pipeable(std::bind(buffer, std::placeholders::_1, n, policy));
      }

    public:

      template<typename Rng, CONCEPT_REQUIRES_(ranges::ForwardRange<Rng>())>
      buffer_view<ranges::view::all_t<Rng>> operator()(
        Rng&& rng,
        std::size_t n = std::numeric_limits<std::size_t>::max(),
        std::launch policy = std::launch::async) const
      {
        return {ranges::view::all(std::forward<Rng>(rng)), n, policy};
      }

  };

  ranges::view::view<buffer_fn> buffer{};

} //end namespace stream
#endif
