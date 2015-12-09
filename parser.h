#pragma once
#include "parser_monad.h"

template <typename C, typename A>
constexpr State<C, A> parse(const Parser<C, A>& f, Span<C> s) {
  return f(s);
}

template <typename C>
constexpr Parser<C, C> next() {
  return [](Span<C> s) -> State<C, C> {
    return (s.empty() ? State<C, C>()
                      : State<C, C>({ { s[0], s.subspan(1) } }));
  };
}

template <typename C>
constexpr Parser<C, Null> eof() {
  return [](Span<C> s) {
    return (s.empty() ? State<C, Null>({ { Null(), {} } }) : State<C, Null>());
  };
}

template <typename C, typename A>
constexpr Parser<C, A> reject() {
  return [](Span<C> s) -> State<C, A> { return {}; };
}

//   do { x <- next ; if prop x then return x else reject }
// same as
//   next >>= \ x -> if prop x then return x else reject
template <typename C>
constexpr Parser<C, C> satisfy(const Func<bool(C)>& prop) {
  return next<C>() >>= [prop](C x) -> Parser<C, C> {
    return (prop(x) ? pure<C, C>(x) : reject<C, C>());
  };
}
