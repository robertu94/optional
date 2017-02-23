/*
A simple monad class for c++11
Copyright © 2017 Robert Underwood
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Robert Underwood ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Robert Underwood BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


The views and conclusions contained in the software and documentation
are those of the authors and should not be interpreted as representing
official policies, either expressedor implied, of Robert Underwood.
*/

#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

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
