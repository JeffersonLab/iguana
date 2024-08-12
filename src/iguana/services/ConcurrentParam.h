#pragma once

#include <oneapi/tbb/concurrent_hash_map.h>
#include <oneapi/tbb/concurrent_vector.h>

namespace iguana {

  /// concurrent hash key type
  using concurrent_key_t = std::size_t;

  /// concurrency model types
  enum ConcurrencyModel {
    none,
    memoize,
    threadpool
  };


  /// @brief wrapper for concurrently mutable configuration parameters
  template <typename T>
  class ConcurrentParam {

    /// hash table container for memoization
    using memo_t = oneapi::tbb::concurrent_hash_map<concurrent_key_t, T>;
    /// vector container for thread pools
    using vector_t = oneapi::tbb::concurrent_vector<T>;

    public:

      /// @param model the concurrent storage model
      ConcurrentParam(ConcurrencyModel model);

      /// @param model the concurrent storage model
      ConcurrentParam(std::string const& model);

      ~ConcurrentParam() {}

      /// @returns the concurrency model
      ConcurrencyModel GetModel() const;

      /// @brief access a stored value
      /// @param key the access key
      /// @returns the stored value
      T const Load(concurrent_key_t const key = 0) const;

      /// @brief modify a value
      /// @param key the access key
      /// @param value the value
      void Save(T const& value, concurrent_key_t const key = 0);

      /// @param key the key
      /// @returns if key `key` is used
      bool HasKey(concurrent_key_t const key) const;

    private:
      std::unordered_map<ConcurrencyModel, std::string> const m_model_names = {
        {none, "none"},
        {memoize, "memoize"},
        {threadpool, "threadpool"}};

      ConcurrencyModel m_model;

      T m_value;
      memo_t m_memo;
      vector_t m_vector;

  };

  // template specializations
  template class ConcurrentParam<int>;
  template class ConcurrentParam<double>;
  template class ConcurrentParam<std::string>;
  template class ConcurrentParam<std::vector<int>>;
  template class ConcurrentParam<std::vector<double>>;
  template class ConcurrentParam<std::vector<std::string>>;

}
