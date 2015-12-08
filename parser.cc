#include <functional>
#include <gsl.h>
#include <iostream>
#include <string>
#include <vector>

/////////////////////////////////////////////////////////////////////// TYPES //
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

/////////////////////////////////////////////////////////////////// THE MONAD //
template <typename C, typename A>
struct Parser : public Func<State<C, A>(Span<C>)> {
  using Func<State<C, A>(Span<C>)>::Func;

  // nice infix operator
  Parser<C, A> operator>>=(const Func<Parser<C, A>(C)>& r) {
    return bind<C, A>(*this, r);
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

/////////////////////////////////////////////////////////////////// FUNCTIONS //

template <typename C, typename A>
constexpr State<C, A> parse(const Parser<C, A>& f, Span<C> s) {
  return f(s);
}

template <typename C>
constexpr Parser<C, C> next() {
  return [](Span<C> s) -> State<C, C>{
    return (s.empty() ? State<C, C>()
                      : State<C, C>({ { s[0], s.subspan(1) } }));
  };
}

class Null {};

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

//////////////////////////////////////////////////////////////////////// SHOW //
std::ostream& operator<<(std::ostream& os, const Null) { return os << "()"; }

template <typename C, typename A>
std::ostream& operator<<(std::ostream& os, const State<C, A>& st) {
  os << '[';
  for (const auto& el : st) {
    os << "(" << el.first << ",\"";
    for (auto c : el.second) os << c;
    os << "\")";
    if (&el != &st.back()) os << ", ";
  }
  os << ']';
  return os;
}

//////////////////////////////////////////////////////////////////////// MAIN //
int main() {
  using Ch = const char;
  std::string input_string{ "abcd" };
  Span<Ch> input{ input_string };

  auto is_b = [](Ch c) { return c == 'b'; };

  auto result =
      parse(next<Ch>() >>= [&is_b](Ch x) { return satisfy<Ch>(is_b); }, input);

  std::cout << result << std::endl;
  return 0;
}
