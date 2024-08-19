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

      /// @param model the concurrency model this instance must be
      /// @see `ConcurrentParamFactory`, the preferred instantiation method
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

      /// @brief whether or not hashing is needed to use this parameter
      /// @returns true if hashing is needed
      bool NeedsHashing() const { return m_needs_hashing; }

    protected:

      /// whether this `ConcurrentParam` needs hashing for calling `::Load` or `::Save`
      bool m_needs_hashing;

  };

  // ==================================================================================
  // UnsafeParam
  // ==================================================================================

  /// @brief a parameter that is _not_ thread safe
  template <typename T>
  class UnsafeParam : public ConcurrentParam<T> {

    public:
      UnsafeParam();
      ~UnsafeParam() {}
      T const Load(concurrent_key_t const key = 0) const override;
      void Save(T const& value, concurrent_key_t const key = 0) override;
      bool HasKey(concurrent_key_t const key) const override;

    private:
      T m_value; // FIXME: consider std::atomic instead
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
      MemoizedParam();
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
    using container_t = oneapi::tbb::concurrent_vector<T>;

    public:
      ThreadPoolParam();
      ~ThreadPoolParam() {}
      T const Load(concurrent_key_t const key = 0) const override;
      void Save(T const& value, concurrent_key_t const key = 0) override;
      bool HasKey(concurrent_key_t const key) const override;

    private:
      container_t m_container;

  };

  // ==================================================================================
  // template specializations
  // ==================================================================================

  template class ConcurrentParam<int>;
  template class ConcurrentParam<double>;
  template class ConcurrentParam<std::string>;
  template class ConcurrentParam<std::vector<int>>;
  template class ConcurrentParam<std::vector<double>>;
  template class ConcurrentParam<std::vector<std::string>>;

  template class UnsafeParam<int>;
  template class UnsafeParam<double>;
  template class UnsafeParam<std::string>;
  template class UnsafeParam<std::vector<int>>;
  template class UnsafeParam<std::vector<double>>;
  template class UnsafeParam<std::vector<std::string>>;

  template class MemoizedParam<int>;
  template class MemoizedParam<double>;
  template class MemoizedParam<std::string>;
  template class MemoizedParam<std::vector<int>>;
  template class MemoizedParam<std::vector<double>>;
  template class MemoizedParam<std::vector<std::string>>;

  template class ThreadPoolParam<int>;
  template class ThreadPoolParam<double>;
  template class ThreadPoolParam<std::string>;
  template class ThreadPoolParam<std::vector<int>>;
  template class ThreadPoolParam<std::vector<double>>;
  template class ThreadPoolParam<std::vector<std::string>>;

  // ==================================================================================
  // ConcurrentParamFactory
  // ==================================================================================

  /// @brief factory to create the appropriate `ConcurrentParam`-derived class instance for the current `GlobalConcurrencyModel`
  class ConcurrentParamFactory {

    public:
      ConcurrentParamFactory() = delete;

      /// @brief create a new `ConcurrentParam`-derived class instance
      /// @returns a pointer to the new instance
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
