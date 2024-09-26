#pragma once

#include "GlobalParam.h"

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
      virtual ~ConcurrentParam() = default;

      /// @brief access a stored value
      /// @param key the access key
      /// @returns the stored value
      virtual T const Load(concurrent_key_t const key) const = 0;

      /// @brief modify a value
      /// @param value the value
      /// @param key the access key
      virtual void Save(T const& value, concurrent_key_t const key) = 0;

      /// @param key the key
      /// @returns if key `key` is used
      virtual bool HasKey(concurrent_key_t const key) const = 0;

      /// @brief whether or not hashing is needed to use this parameter
      /// @returns true if hashing is needed
      bool NeedsHashing() const { return m_needs_hashing; }

      /// @returns the size of the internal data storage container
      virtual std::size_t GetSize() const = 0;

      /// @returns true if no value has been saved
      bool IsEmpty() const { return m_empty; }

    protected:

      /// whether this `ConcurrentParam` needs hashing for calling `::Load` or `::Save`
      bool m_needs_hashing;

      /// mutex for this `ConcurrentParam`
      std::mutex m_mutex;

      /// whether this `ConcurrentParam` has something saved
      bool m_empty{true};

  };

  // ==================================================================================
  // SingleThreadParam
  // ==================================================================================

  /// @brief a parameter that is _not_ thread safe;
  /// used when `GlobalConcurrencyModel == "single"`
  template <typename T>
  class SingleThreadParam : public ConcurrentParam<T> {

    public:
      SingleThreadParam();
      ~SingleThreadParam() override = default;
      T const Load(concurrent_key_t const key) const override;
      void Save(T const& value, concurrent_key_t const key) override;
      bool HasKey(concurrent_key_t const key) const override;
      std::size_t GetSize() const override;

    private:

      /// the stored value; it is not `std::atomic` since `std::string` is one of
      /// the specializations (for `T`), which is not trivially copyable
      T m_value;

  };

  // ==================================================================================
  // MemoizedParam
  // ==================================================================================

  /// @brief a `ConcurrentParam` that uses memoization for thread safety;
  /// used when `GlobalConcurrencyModel == "memoize"`
  template <typename T>
  class MemoizedParam : public ConcurrentParam<T> {

    /// hash table container for memoization
    using container_t = std::unordered_map<concurrent_key_t, T>;

    public:
      MemoizedParam();
      ~MemoizedParam() override = default;
      T const Load(concurrent_key_t const key) const override;
      void Save(T const& value, concurrent_key_t const key) override;
      bool HasKey(concurrent_key_t const key) const override;
      std::size_t GetSize() const override;

    private:

      /// the hash table for stored (memoized) values
      container_t m_container;

  };

  // ==================================================================================
  // ConcurrentParamFactory
  // ==================================================================================

  /// @brief factory to create the appropriate `ConcurrentParam`-derived class instance for the current `GlobalConcurrencyModel`
  class ConcurrentParamFactory {

    public:
      ConcurrentParamFactory() = delete;

      /// @brief create a new `ConcurrentParam`-derived class instance
      /// @warning if `GlobalConcurrencyModel` is not set, the model `"memoize"` will be chosen,
      /// since it is thread safe and does not assume anything about the user's implementation
      /// @returns a pointer to the new instance
      template <typename T>
      static std::unique_ptr<ConcurrentParam<T>> Create() {

        if(GlobalConcurrencyModel() == "none")
          GlobalConcurrencyModel = "memoize"; // the safest default, but not the fastest for single-threaded users

        if(GlobalConcurrencyModel() == "single")
          return std::make_unique<SingleThreadParam<T>>();
        else if(GlobalConcurrencyModel() == "memoize")
          return std::make_unique<MemoizedParam<T>>();

        throw std::runtime_error("unknown GlobalConcurrencyModel '" + GlobalConcurrencyModel() + "'; valid options are 'single' or 'memoize'");
      }

  };

}
