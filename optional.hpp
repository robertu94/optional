#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <typeinfo>

class OptionalBase {
  // for type safety
public:
  OptionalBase() = default;
  virtual ~OptionalBase() = default;
};

template <class T> class Optional : public OptionalBase {
public:
  Optional<T>() : OptionalBase(), managed(nullptr) {}

  Optional<T>(T v) : OptionalBase(), managed(new T(v)) {}

  Optional<T>(T* v) : OptionalBase(), managed((v)? new T(*v) : nullptr) {}

  Optional<T>(const Optional<T> &v)
      : OptionalBase(), managed((v.managed) ? new T(*v.managed) : nullptr) {}

  ~Optional<T>() { delete managed; }
  Optional<T> &operator=(const Optional &rhs) {
    if (&rhs == this)
      return *this;
    delete managed;
    managed = ((rhs.managed) ? (new T(*rhs.managed)) : (nullptr));
    return *this;
  }

  // comparable
  bool operator==(const Optional<T> &rhs) {
    if (managed == nullptr && rhs.managed == nullptr)
      return true; // empty == empty
    else if (managed == nullptr || rhs.managed == nullptr)
      return false; // empty != full
    else
      return *managed == *rhs.managed;
  }

  bool operator<(const Optional<T> &rhs) {
    if (managed == nullptr && rhs.managed == nullptr)
      return false; // empty == empty
    else if (managed == nullptr || rhs.managed == nullptr)
      return managed == nullptr; // empty < full
    else
      return *managed < *rhs.managed;
  }

  /*
 * Args must be {T*,T&, or T}
 * Ret must be {V,Optional<V>,}
 */
  // Args= *T Ret= *V
  template <class Ret, class Args>
  Optional<typename std::remove_pointer<Ret>::type> Bind(
      std::function<Ret(Args)> f,
      typename std::enable_if<
          std::is_pointer<Ret>{} && std::is_pointer<Args>{} &&
          std::is_convertible<T, typename std::remove_pointer<Args>::type>{}>::type * =
          0) const {
    return f(managed);
  }

  // Args= T Ret= *V
  template <class Ret, class Args>
  Optional<typename std::remove_pointer<Ret>::type>
  Bind(std::function<Ret(Args)> f,
       typename std::enable_if<std::is_pointer<Ret>{} &&
                               std::is_object<Args>{} &&
                               std::is_convertible<T, Args>{}>::type * = 0) const {
    if (managed)
      return f(*managed);
    return nullptr;
  }

  // Args= &T Ret = *V
  template <class Ret, class Args>
  Optional<typename std::remove_pointer<Ret>::type>
  Bind(std::function<Ret(Args)> f,
       typename std::enable_if<
           std::is_pointer<Ret>{} && std::is_reference<Args>{} &&
           std::is_convertible<T, typename std::remove_reference<Args>::type>{}>::type
           * = 0) const {
    if (managed)
      return f(*managed);
    return nullptr;
  }

  // Args= *T Ret= V
  template <class Ret, class Args>
  Optional<Ret>
  Bind(std::function<Ret(Args)> f,
       typename std::enable_if<
           std::is_object<Ret>{} && !std::is_pointer<Ret>{} &&
           std::is_pointer<Args>{} &&
           std::is_convertible<T, typename std::remove_pointer<Args>::type>{} &&
           !std::is_base_of<OptionalBase, Ret>{}>::type * = 0) const {
    return f(managed);
  }

  // Args= T Ret= V
  template <class Ret, class Args>
  Optional<Ret>
  Bind(std::function<Ret(Args)> f,
       typename std::enable_if<
           std::is_object<Ret>{} && !std::is_pointer<Ret>{} &&
           std::is_object<Args>{} && std::is_convertible<T, Args>{} &&
           !std::is_base_of<OptionalBase, Ret>{}>::type * = 0) const {
    if (managed)
      return f(*managed);
    return nullptr;
  }

  // Args= &T Ret = V
  template <class Ret, class Args>
  Optional<Ret>
  Bind(std::function<Ret(Args)> f,
       typename std::enable_if<
           std::is_object<Ret>{} && !std::is_pointer<Ret>{} &&
           std::is_reference<Args>{} &&
           std::is_convertible<T, typename std::remove_reference<Args>::type>{} &&
           !std::is_base_of<OptionalBase, Ret>{}>::type * = 0) const {
    if (managed)
      return f(*managed);
    return nullptr;
  }

  // Args= *T Ret= Optional<V>
  template <class Ret, class Args>
  Ret Bind(std::function<Ret(Args)> &f,
           typename std::enable_if<
               std::is_object<Ret>{} && std::is_pointer<Args>{} &&
               std::is_convertible<T, typename std::remove_pointer<Args>::type>{} &&
               std::is_base_of<OptionalBase, Ret>{}>::type * = 0) const {
    return f(managed);
  }

  // Args= T Ret= Optional<V>
  template <class Ret, class Args>
  Ret Bind(
      std::function<Ret(Args)> f,
      typename std::enable_if<std::is_object<Ret>{} && std::is_object<Args>{} &&
                              std::is_convertible<T, Args>{} &&
                              std::is_base_of<OptionalBase, Ret>{}>::type * =
          0) const {
    if (managed)
      return f(*managed);
    return nullptr;
  }

  // Args= &T Ret = Optional<V>
  template <class Ret, class Args>
  Ret Bind(std::function<Ret(Args)> f,
           typename std::enable_if<
               std::is_object<Ret>{} && std::is_reference<Args>{} &&
               std::is_convertible<T, typename std::remove_reference<Args>::type>{} &&
               std::is_base_of<OptionalBase, Ret>{}>::type * = 0) const {
    if (managed)
      return f(*managed);
    return nullptr;
  }

  // Args= *T Ret= void
  template <class Ret, class Args>
  Optional<T> Bind(
      std::function<Ret(Args)> &f,
      typename std::enable_if<
          std::is_void<Ret>{} && std::is_pointer<Args>{} &&
          std::is_convertible<T, typename std::remove_pointer<Args>::type>{}>::type * =
          0) const {
    if (managed)
      f(managed);
    return *this;
  }

  // Args= T Ret= void
  template <class Ret, class Args>
  Optional<T>
  Bind(std::function<Ret(Args)> f,
       typename std::enable_if<std::is_void<Ret>{} && std::is_object<Args>{} &&
                               std::is_convertible<T, Args>{}>::type * = 0) const {
    if (managed)
      f(*managed);
    return *this;
  }

  // Args= &T Ret = void
  template <class Ret, class Args>
  Optional<T>
  Bind(std::function<Ret(Args)> f,
       typename std::enable_if<
           std::is_void<Ret>{} && std::is_reference<Args>{} &&
           std::is_convertible<T, typename std::remove_reference<Args>::type>{}>::type
           * = 0) const {
    if (managed)
      f(*managed);
    return *this;
  }

  //allow raw access to the type
  T* get() const {return managed;}
  
private:
  T *managed;
};
