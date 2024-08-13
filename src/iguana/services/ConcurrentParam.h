#pragma once

#include "GlobalParam.h"

#include <oneapi/tbb/concurrent_hash_map.h>
#include <oneapi/tbb/concurrent_vector.h>

namespace iguana {

  /// concurrent hash key type
  using concurrent_key_t = std::size_t;

  // ==================================================================================
  // ConcurrentParam
  // ==================================================================================

  /// @brief abstract base class for concurrently mutable configuration parameters
  template <typename T>
  class ConcurrentParam {

    public:

      /// @param model the concurrency model this instance must be; throws a runtime exception if it is not
      ConcurrentParam(std::string const& model);
      ~ConcurrentParam() {}

      /// @brief access a stored value
      /// @param key the access key
      /// @returns the stored value
      virtual T const Load(concurrent_key_t const key = 0) const = 0;

      /// @brief modify a value
      /// @param key the access key
      /// @param value the value
      virtual void Save(T const& value, concurrent_key_t const key = 0) = 0;

      /// @param key the key
      /// @returns if key `key` is used
      virtual bool HasKey(concurrent_key_t const key) const = 0;

  };

  // ==================================================================================
  // UnsafeParam
  // ==================================================================================

  /// @brief a parameter that is _not_ thread safe
  template <typename T>
  class UnsafeParam : public ConcurrentParam<T> {

    public:
      UnsafeParam() : ConcurrentParam<T>("unsafe") {};
      ~UnsafeParam() {}
      T const Load(concurrent_key_t const key = 0) const override;
      void Save(T const& value, concurrent_key_t const key = 0) override;
      bool HasKey(concurrent_key_t const key) const override;

    private:
      T m_value;
  };

  // ==================================================================================
  // MemoizedParam
  // ==================================================================================

  /// @brief a `ConcurrentParam` that uses memoization for thread safety
  template <typename T>
  class MemoizedParam : public ConcurrentParam<T> {

    /// hash table container for memoization
    using container_t = oneapi::tbb::concurrent_hash_map<concurrent_key_t, T>;

    public:
      MemoizedParam() : ConcurrentParam<T>("memoize") {};
      ~MemoizedParam() {}
      T const Load(concurrent_key_t const key = 0) const override;
      void Save(T const& value, concurrent_key_t const key = 0) override;
      bool HasKey(concurrent_key_t const key) const override;

    private:
      container_t m_container;

  };

  // ==================================================================================
  // ThreadPoolParam
  // ==================================================================================

  /// @brief a `ConcurrentParam` that uses unique thread-pool indices for thread safety
  template <typename T>
  class ThreadPoolParam : public ConcurrentParam<T> {

    /// hash table container for memoization
    using container_t = oneapi::tbb::concurrent_vector<concurrent_key_t, T>;

    public:
      ThreadPoolParam() : ConcurrentParam<T>("threadpool") {};
      ~ThreadPoolParam() {}
      T const Load(concurrent_key_t const key = 0) const override;
      void Save(T const& value, concurrent_key_t const key = 0) override;
      bool HasKey(concurrent_key_t const key) const override;

    private:
      container_t m_container;

  };

  // ==================================================================================
  // ConcurrentParamFactory
  // ==================================================================================

  class ConcurrentParamFactory {

    public:
      ConcurrentParamFactory() = delete;

      template <typename T>
      static std::unique_ptr<ConcurrentParam<T>> Create() {

        if(GlobalConcurrencyModel() == "none") {
          printf("WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING\n"); // FIXME: use `Logger`
          GlobalConcurrencyModel = "unsafe";
        }

        if(GlobalConcurrencyModel() == "unsafe")
          return std::make_unique<UnsafeParam<T>>();
        else if(GlobalConcurrencyModel() == "memoize")
          return std::make_unique<MemoizedParam<T>>();
        else if(GlobalConcurrencyModel() == "threadpool")
          return std::make_unique<ThreadPoolParam<T>>();

        throw std::runtime_error("unknown GlobalConcurrencyModel '" + GlobalConcurrencyModel() + "'");
      }

  };

}
