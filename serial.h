/**
  * Copyright 2022-2023 ShunzDai
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
#pragma once

#include <string.h>
#include <assert.h>
#include <utility>
#include <string_view>
#include <string>
#include <array>

namespace serial {

using ibuf_t = std::basic_string_view<uint8_t>;
using obuf_t = std::basic_string<uint8_t>;

// ref https://stackoverflow.com/questions/15887144/stdremove-const-with-const-references 
template<typename arg_t>
struct remove_operator {
  using type = typename std::remove_const<typename std::remove_reference<arg_t>::type>::type;
};

template<template <typename ... arg_t> typename container_t, typename first_t, typename ... rest_t>
struct is_container {
  static const bool value = false;
};

template<template <typename ... arg_t> typename container_t, typename first_t, typename ... rest_t>
struct is_container<container_t, container_t<first_t, rest_t ...>> {
  static const bool value = true;
};

template<typename first_t, typename ... rest_t>
using is_tuple = is_container<std::tuple, first_t, rest_t ...>;

template<typename first_t, typename ... rest_t>
using is_pair = is_container<std::pair, first_t, rest_t ...>;

template<typename first_t, typename ... rest_t>
using is_string = is_container<std::basic_string, first_t, rest_t ...>;

template<typename first_t, typename ... rest_t>
using is_string_view = is_container<std::basic_string_view, first_t, rest_t ...>;

template <typename arg_t>
constexpr obuf_t pack_one(const arg_t &arg);
template <typename arg_t>
constexpr arg_t unpack_one(ibuf_t &ibuf);

template <typename ... arg_t>
constexpr void pod_check(const arg_t & ... arg) {
  if constexpr (sizeof...(arg_t)) {
    constexpr auto check = [](auto arg) {
      static_assert(std::is_pod<decltype(arg)>::value);
    };
    (check(arg), ...);
  }
}

template <typename ... arg_t, size_t ... seq>
constexpr obuf_t pack_tuple(const std::tuple<arg_t ...> &arg, std::index_sequence<seq ...>) {
  if constexpr (sizeof...(arg_t))
    return (pack_one(std::get<seq>(arg)) + ...);
  else
    return {};
}

template <typename arg_t>
constexpr obuf_t pack_one(const arg_t &arg) {
  using base_t = typename remove_operator<arg_t>::type;
  if constexpr (is_string<arg_t>::value || is_string_view<arg_t>::value) {
    return pack_one(arg.size()) + obuf_t((const uint8_t *)arg.data(), arg.size());
  }
  else if constexpr (is_tuple<arg_t>::value) {
    return pack_tuple(arg, std::make_index_sequence<std::tuple_size<arg_t>::value> {});
  }
  else if constexpr (is_pair<arg_t>::value) {
    return pack_one(arg.first) + pack_one(arg.second);
  }
  else if constexpr (std::is_pod<base_t>::value) {
    return obuf_t((uint8_t *)&arg, sizeof(arg_t));
  }
  else {
    static_assert(!std::is_same<arg_t, arg_t>::value, "unknown arg type");
  }
}

template <typename ... arg_t>
constexpr obuf_t pack(const arg_t & ... args) {
  if constexpr (sizeof...(arg_t))
    return (pack_one(args) + ...);
  else
    return {};
}

template <typename arg_t, size_t ... seq>
constexpr arg_t unpack_tuple(ibuf_t &ibuf, std::index_sequence<seq ...>) {
  if constexpr (std::tuple_size<arg_t>::value)
    return {unpack_one<typename std::tuple_element<seq, arg_t>::type>(ibuf) ...};
  else
    return {};
}

template <typename arg_t>
constexpr arg_t unpack_one(ibuf_t &ibuf) {
  using base_t = typename remove_operator<arg_t>::type;
  if constexpr (is_string_view<base_t>::value) {
    static_assert(!std::is_reference<arg_t>::value, "returning reference to local temporary object");
    auto size = unpack_one<size_t>(ibuf);
    auto data = (typename arg_t::value_type *)ibuf.data();
    ibuf.remove_prefix(size);
    return {data, size};
  }
  else if constexpr (std::is_pointer<base_t>::value) {
    return (arg_t)ibuf.data();
  }
  else if constexpr (std::is_pod<base_t>::value) {
    base_t *p = (base_t *)ibuf.data();
    ibuf.remove_prefix(sizeof(arg_t));
    return *p;
  }
  else if constexpr (is_tuple<base_t>::value) {
    static_assert(!std::is_reference<arg_t>::value, "returning reference to local temporary object");
    return unpack_tuple<base_t>(ibuf, std::make_index_sequence<std::tuple_size<base_t>::value> {});
  }
  else if constexpr (is_pair<base_t>::value) {
    static_assert(!std::is_reference<arg_t>::value, "returning reference to local temporary object");
    return {unpack_one<typename base_t::first_type>(ibuf), unpack_one<typename base_t::second_type>(ibuf)};
  }
  else {
    static_assert(!std::is_same<arg_t, arg_t>::value, "unknown arg type");
  }
}

template <typename ... arg_t>
constexpr std::tuple<arg_t ...> unpack(ibuf_t &&buf) {
  if constexpr (sizeof...(arg_t))
    return {unpack_one<arg_t>(buf) ...};
  else
    return {};
}

template <typename ... arg_t>
constexpr std::tuple<arg_t ...> unpack(obuf_t &&buf) {
  if constexpr (sizeof...(arg_t))
    return {unpack_one<arg_t>(buf) ...};
  else
    return {};
}

} /* namespace serial */
