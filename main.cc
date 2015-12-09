#include "parser.h"
#include <iostream>
#include <string>

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

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
  if (argc < 2) return 1;
  using Ch = const char;
  auto input = Span<Ch>(argv[1], strlen(argv[1]));

  auto is_b = [](Ch c) { return c == 'b'; };

  auto result = parse(next<Ch>() < satisfy<Ch>(is_b), input);

  std::cout << result << std::endl;
  return 0;
}
