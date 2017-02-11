/// \file
// Stream prototype library
//
//  Copyright Filip Matzner 2017
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0.
//  (see http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef COLUMN_TRANSFORM_HPP
#define COLUMN_TRANSFORM_HPP

#include <type_traits>
#include <utility>
#include <ostream>

#include <utility/tuple.hpp>
#include <range/v3/view/transform.hpp>
#include <experimental/tuple>


#define STREAM_DEFINE_COLUMN(col_name, col_type) \
struct col_name { \
  col_type value; \
  static constexpr const char* name = #col_name; \
  col_name() = default; \
  col_name(col_type v) noexcept \
    : value{std::move(v)} {} \
  bool operator==(const col_name& rhs) const noexcept \
  { return value == rhs.value; } \
}; \
std::ostream& operator<<(std::ostream& out, const col_name& c) \
{ return out << std::make_tuple(c.name, c.value); }


namespace stream {

  using namespace ranges;


  template<typename... Columns>
  struct from {};


  template<typename... Columns>
  struct to {};


  struct identity
  {
    template<typename T>
    T&& operator()(T&& val) const noexcept
    {
      return std::forward<T>(val);
    }
  };


  template<typename... FromTypes, typename... ToTypes,
           typename Fun, typename Projection = identity>
  auto partial_transform(from<FromTypes...>, to<ToTypes...>,
                         Fun fun, Projection proj = Projection{})
  {
    return view::transform([fun=std::move(fun), proj=std::move(proj)](auto source){
      // select the tuple types to be processed
      auto slice = utility::tuple_type_view<FromTypes...>(source);
      // project them
      const auto proj_slice = utility::tuple_transform(std::move(proj), std::move(slice));
      // process the result and convert to the requested types
      std::tuple<ToTypes...> result{std::experimental::apply(fun, proj_slice)};
      // replace the returned types
      return utility::tuple_cat_unique(std::move(result), std::move(source));
    });
  }


  template<typename... FromColumns, typename... ToColumns, typename Fun>
  auto column_transform(from<FromColumns...> f, to<ToColumns...> t, Fun fun)
  {
    return partial_transform(f, t, std::move(fun), [](auto& column) -> auto& {
      return column.value;
    });
  }

} // end namespace stream
#endif
