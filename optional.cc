#include "optional.hpp"

void withInts() {
  std::function<Optional<int>(int *)> f = [](int *v) {
    if (v == nullptr) {
      return Optional<int>(-1);
    } else if (*v == 2) {
      return Optional<int>();
    } else {
      return Optional<int>(*v + 1);
    }
  };
  int *one = new int(1);
  int *two = new int(2);
  int *three = new int(3);

  std::cout << std::boolalpha;
  std::cout << (Optional<int>{2}.Bind(f) == f(two)) << std::endl;
  std::cout << (Optional<int>{1}.Bind(f) == f(one)) << std::endl;
  std::cout << (Optional<int>().Bind(f) == f(nullptr)) << std::endl;

  delete one;
  delete two;
  delete three;
}

void withStrings() {
  std::function<const int *(const int * const)> f = [](const int * const x) {
    if (x)
      return (*x % 2 == 0) ? nullptr : x;
    else
      return static_cast<const int * const>(nullptr);
  };

  std::function<std::string(const int * const)> g = [](const int * const x) {
    return (x) ? std::to_string(*x) : std::string("null");
  };

  std::function<std::string(const int * const)> h = [f, g](const int * const x) {
    return g(f(x));
  };

  std::function<void(std::string &)> print = [](std::string &s) {
    std::cout << s << std::endl;
  };

  Optional<int> two{2};
  Optional<int> three{3};

  two.Bind(f).Bind(g).Bind(print);
  three.Bind(f).Bind(g).Bind(print);

  two.Bind(f).Bind(g).Bind(print);
  three.Bind(f).Bind(g).Bind(print);

  two.Bind(h).Bind(print);
  three.Bind(h).Bind(print);
}
int main() {
  withInts();
  withStrings();
}
