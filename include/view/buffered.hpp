/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef STREAM_BUFFERED_VIEW_HPP
#define STREAM_BUFFERED_VIEW_HPP

#include <deque>
#include <future>
#include <range/v3/core.hpp>
#include <range/v3/view/all.hpp>


namespace stream {
  

  template<typename Rng>
  struct buffered_view
    : ranges::view_facade<buffered_view<Rng>>
  {
    private:
      friend ranges::range_access;

      Rng rng_;

      std::size_t n_;
      std::launch policy_;

      struct cursor
      {
        private:
          buffered_view<Rng> *rng_ = nullptr;
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
          explicit cursor(buffered_view<Rng>& rng)
            : rng_{&rng},
              it_{ranges::begin(rng.rng_)},
              n_{rng.n_},
              policy_{rng.policy_}
          {
            fill_buffer();
          }
          auto read() const
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
      buffered_view() = default;
      buffered_view(Rng rng, std::size_t n, std::launch policy = std::launch::async)
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


  namespace view {


    template<typename Rng, CONCEPT_REQUIRES_(ranges::ForwardRange<Rng>())>
    buffered_view<ranges::view::all_t<Rng>> buffered_view(
      Rng&& rng, std::size_t n, std::launch policy = std::launch::async)
    {
      return {ranges::view::all(std::forward<Rng>(rng)), n, policy};
    }


  } //end namespace view
} //end namespace stream
#endif
