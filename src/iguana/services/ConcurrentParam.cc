#include "ConcurrentParam.h"

namespace iguana {

  // ==================================================================================
  // Constructors
  // ==================================================================================

  template <typename T>
  ConcurrentParam<T>::ConcurrentParam(std::string const& model)
  {
    if(GlobalConcurrencyModel() != model)
      throw std::runtime_error(
          "attempted to construct a ConcurrentParam with model '" +
          model + "', but GlobalConcurrencyModel is '" + GlobalConcurrencyModel() + "'");
  }

  template <typename T>
  UnsafeParam<T>::UnsafeParam() : ConcurrentParam<T>("unsafe")
  {
    this->m_needs_hashing = false;
  }

  template <typename T>
  MemoizedParam<T>::MemoizedParam() : ConcurrentParam<T>("memoize")
  {
    this->m_needs_hashing = true;
  }

  template <typename T>
  ThreadPoolParam<T>::ThreadPoolParam() : ConcurrentParam<T>("threadpool")
  {
    this->m_needs_hashing = false;
  }

  // ==================================================================================
  // Load methods
  // ==================================================================================

  template <typename T>
  T const UnsafeParam<T>::Load(concurrent_key_t const key) const
  {
    return m_value;
  }

  template <typename T>
  T const MemoizedParam<T>::Load(concurrent_key_t const key) const
  {
    typename container_t::const_accessor acc;
    if(m_container.find(acc, key))
      return acc->second;
    throw std::runtime_error("MemoizedParam::Load failed to find the parameter");
  }

  template <typename T>
  T const ThreadPoolParam<T>::Load(concurrent_key_t const key) const
  {
    throw std::runtime_error("TODO: 'threadpool' model not yet implemented");
  }

  // ==================================================================================
  // Save methods
  // ==================================================================================

  template <typename T>
  void UnsafeParam<T>::Save(T const& value, concurrent_key_t const key)
  {
    m_value = value;
  }

  template <typename T>
  void MemoizedParam<T>::Save(T const& value, concurrent_key_t const key)
  {
    typename container_t::accessor acc;
    m_container.insert(acc, key);
    acc->second = value;
  }

  template <typename T>
  void ThreadPoolParam<T>::Save(T const& value, concurrent_key_t const key)
  {
    throw std::runtime_error("TODO: 'threadpool' model not yet implemented");
  }

  // ==================================================================================
  // HasKey methods
  // ==================================================================================

  template <typename T>
  bool UnsafeParam<T>::HasKey(concurrent_key_t const key) const
  {
    throw std::runtime_error("do not call ConcurrentParam::HasKey when model is 'none'");
  }

  template <typename T>
  bool MemoizedParam<T>::HasKey(concurrent_key_t const key) const
  {
    typename container_t::const_accessor acc;
    return m_container.find(acc, key);
  }

  template <typename T>
  bool ThreadPoolParam<T>::HasKey(concurrent_key_t const key) const
  {
    throw std::runtime_error("TODO: 'threadpool' model not yet implemented");
  }

}
