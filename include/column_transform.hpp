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
#include <boost/type_traits/has_left_shift.hpp>

#include <utility/tuple.hpp>
#include <range/v3/view/transform.hpp>
#include <experimental/tuple>


#define STREAM_DEFINE_COLUMN(col_name, col_type) \
struct col_name { \
  col_type value; \
  static constexpr const char* name = #col_name; \
  col_name() = default; \
  col_name(col_type v) \
    : value{std::move(v)} {} \
}; \
template<typename = boost::has_left_shift<std::ostream&, const col_type&>::type> \
std::ostream& operator<<(std::ostream& out, const col_name& c) \
{ \
  using utility::operator<<; \
  return out << std::make_tuple(std::cref(c.name), std::cref(c.value)); \
}


namespace stream {

  using namespace ranges;


  /* helpers */


  template<typename... Columns>
  struct from_t {};

  template<typename... Columns>
  auto from = from_t<Columns...>{};

  template<typename... Columns>
  struct to_t {};

  template<typename... Columns>
  auto to = to_t<Columns...>{};


  struct identity_t
  {
    template<typename T>
    constexpr T&& operator()(T&& val) const noexcept
    {
      return std::forward<T>(val);
    }
  };

  auto identity = identity_t{};


  /* column_transform */


  template<typename... FromTypes, typename... ToTypes,
           typename Fun, typename Projection = identity_t>
  constexpr auto partial_transform(from_t<FromTypes...>, to_t<ToTypes...>,
                         Fun fun, Projection proj = Projection{})
  {
    return view::transform([fun=std::move(fun), proj=std::move(proj)](auto&& source) {
      // build the view for the transformer; 1) slice 2) project
      const auto slice_view =
        utility::tuple_transform(proj, utility::tuple_type_view<FromTypes...>(source));
      // process the transformer's result and convert it to the requested types
      std::tuple<ToTypes...> result{std::experimental::apply(fun, slice_view)};
      // replace the corresponding fields
      return utility::tuple_cat_unique(std::move(result), std::forward<decltype(source)>(source));
    });
  }


  template<typename... FromColumns, typename... ToColumns, typename Fun>
  constexpr auto column_transform(from_t<FromColumns...> f, to_t<ToColumns...> t, Fun fun)
  {
    return partial_transform(f, t, std::move(fun), [](auto& column) {
      return std::ref(column.value);
    });
  }


  /* column_drop */


  template<typename Column>
  constexpr auto column_drop_fn()
  {
    return view::transform([](auto&& source) {
      return utility::tuple_remove<Column>(std::forward<decltype(source)>(source));
    });
  }

  template<typename Column>
  auto column_drop = column_drop_fn<Column>();


  /* column_create */


  template<typename Column>
  constexpr auto column_create_fn()
  {
    return view::transform([](auto&& source){
        return std::tuple<Column>{std::forward<decltype(source)>(source)};
    });
  }

  template<typename Column>
  auto column_create = column_create_fn<Column>();


} // end namespace stream
#endif
