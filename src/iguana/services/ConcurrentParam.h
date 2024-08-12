#pragma once

#include <oneapi/tbb/concurrent_hash_map.h>
#include <oneapi/tbb/concurrent_vector.h>

namespace iguana {

  /// concurrent hash key type
  using concurrent_key_t = std::size_t;

  /// @brief wrapper for concurrently mutable configuration parameters
  template <typename T>
  class ConcurrentParam {

    /// hash table container for memoization
    using memo_t = oneapi::tbb::concurrent_hash_map<concurrent_key_t, T>;
    /// vector container for thread pools
    using vector_t = oneapi::tbb::concurrent_vector<T>;

    private:
      enum model_t {
        none,
        memoize,
        threadpool
      };
      std::unordered_map<model_t, std::string> const m_model_names = {
        {none, "none"},
        {memoize, "memoize"},
        {threadpool, "threadpool"}};

      model_t m_model;

      T m_value;
      memo_t m_memo;
      vector_t m_vector;

    public:

      /// @param model the concurrent storage model
      ConcurrentParam(model_t model);

      /// @param model the concurrent storage model
      ConcurrentParam(std::string const& model);

      ~ConcurrentParam() {}

      /// @brief access a stored value
      /// @param key the access key
      /// @returns the stored value
      T const Load(concurrent_key_t const key) const;

      /// @brief modify a value
      /// @param key the access key
      /// @param value the value
      void Save(concurrent_key_t const key, T const& value);

      /// @param key the key
      /// @returns if key `key` is used
      bool HasKey(concurrent_key_t const key) const;

  };

  // template specializations
  template class ConcurrentParam<int>;
  template class ConcurrentParam<double>;
  template class ConcurrentParam<std::string>;
  template class ConcurrentParam<std::vector<int>>;
  template class ConcurrentParam<std::vector<double>>;
  template class ConcurrentParam<std::vector<std::string>>;

}
