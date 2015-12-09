#pragma once

#include <vector>
#include <functional>
#include <gsl.h>

template <typename T>
using Func = std::function<T>;

template <typename T>
using List = std::vector<T>;

template <typename A, typename B>
using Pair = std::pair<A, B>;

template <typename T>
using Span = gsl::span<T>;

template <typename C, typename A>
using State = List<Pair<A, Span<C>>>;

class Null {};

////////////////////////////////////////////////////////////////////////////////

template <typename C, typename A>
struct Parser : public Func<State<C, A>(Span<C>)> {
  using Func<State<C, A>(Span<C>)>::Func;

  constexpr Parser<C, A> operator>>=(const Func<Parser<C, A>(C)>& r) const {
    return bind<C, A>(*this, r);
  }

  constexpr Parser<C, A> operator|(const Parser<C, A>& r) const {
    return or <C, A>(*this, r);
  }

  template <typename R>
  constexpr Parser<C, R> operator>(const Parser<C, R>& r) const {
    return rstar<C, A, R>(*this, r);
  }

  template <typename R>
  constexpr Parser<C, A> operator<(const Parser<C, R>& r) const {
    return lstar<C, A, R>(*this, r);
  }
};

// pure x = Parser $ \ s -> [(x,s)]
template <typename C, typename A>
constexpr Parser<C, A> pure(const C& x) {
  return [x](Span<C> s) { return State<C, A>({ { x, s } }); };
}

// Parser f >>= g = Parser $ \ s -> do
//   (a,t) <- f s
//   let Parser h = g a
//   h t
template <typename C, typename A>
constexpr Parser<C, A> bind(const Parser<C, A>& f,
                            const Func<Parser<C, A>(C)>& g) {
  return [f, g](Span<C> s) -> State<C, A> {
    State<C, A> ret;
    // ats :: [(C,[A])]
    auto ats = f(s); // (a,t) <- f s
    for (const auto& at : ats) {
      auto h = g(at.first);  // let Parser h = g a
      auto x = h(at.second); // h t
      std::move(x.begin(), x.end(), std::back_inserter(ret));
    }
    return ret;
  };
}

template <typename C, typename A>
    constexpr Parser<C, A> or (const Parser<C, A>& f, const Parser<C, A>& g) {
  return [f, g](Span<C> s) -> State<C, A> {
    auto fval = f(s);
    auto gval = g(s);
    std::move(gval.begin(), gval.end(), std::back_inserter(fval));
    return fval;
  };
}

template <typename C, typename L, typename R>
constexpr Parser<C, L> lstar(const Parser<C, L>& f, const Parser<C, R>& g) {
  return [f, g](Span<C> s) -> State<C, L> {
    auto l = f(s);
    State<C, L> ret;
    for (auto& x : l) {
      for (auto& y : g(x.second)) {
        ret.emplace_back(std::move(x.first), y.second);
      }
    }
    return ret;
  };
}

template <typename C, typename L, typename R>
constexpr Parser<C, R> rstar(const Parser<C, L>& f, const Parser<C, R>& g) {
  return [f, g](Span<C> s) -> State<C, R> {
    auto l = f(s);
    State<C, R> ret;
    for (auto& x : l) {
      auto r = g(x.second);
      std::move(r.begin(), r.end(), std::back_inserter(ret));
    }
    return ret;
  };
}
