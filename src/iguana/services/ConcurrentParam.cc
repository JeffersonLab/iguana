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
  SingleThreadParam<T>::SingleThreadParam() : ConcurrentParam<T>("single")
  {
    this->m_needs_hashing = false;
  }

  template <typename T>
  MemoizedParam<T>::MemoizedParam() : ConcurrentParam<T>("memoize")
  {
    this->m_needs_hashing = true;
  }

  // ==================================================================================
  // Load methods
  // ==================================================================================

  template <typename T>
  T const SingleThreadParam<T>::Load(concurrent_key_t const key) const
  {
    return m_value;
  }

  template <typename T>
  T const MemoizedParam<T>::Load(concurrent_key_t const key) const
  {
    if(auto it{m_container.find(key)}; it != m_container.end())
      return it->second;
    throw std::runtime_error("MemoizedParam::Load failed to find the parameter");
  }

  // ==================================================================================
  // Save methods
  // ==================================================================================

  template <typename T>
  void SingleThreadParam<T>::Save(T const& value, concurrent_key_t const key)
  {
    this->m_empty = false;
    m_value = value;
  }

  template <typename T>
  void MemoizedParam<T>::Save(T const& value, concurrent_key_t const key)
  {
    std::lock_guard<std::mutex> const lock(this->m_mutex);
    this->m_empty = false;
    m_container.insert({key, value});
  }

  // ==================================================================================
  // HasKey methods
  // ==================================================================================

  template <typename T>
  bool SingleThreadParam<T>::HasKey(concurrent_key_t const key) const
  {
    throw std::runtime_error("do not call ConcurrentParam::HasKey when model is 'single'");
  }

  template <typename T>
  bool MemoizedParam<T>::HasKey(concurrent_key_t const key) const
  {
    return m_container.find(key) != m_container.end();
  }

  // ==================================================================================
  // GetSize() methods
  // ==================================================================================
  template <typename T>
  std::size_t SingleThreadParam<T>::GetSize() const
  {
    return 1;
  }

  template <typename T>
  std::size_t MemoizedParam<T>::GetSize() const
  {
    return m_container.size();
  }

  // ==================================================================================
  // template specializations
  // ==================================================================================

  template class ConcurrentParam<int>;
  template class ConcurrentParam<double>;
  template class ConcurrentParam<std::string>;
  template class ConcurrentParam<std::vector<int>>;
  template class ConcurrentParam<std::vector<double>>;
  template class ConcurrentParam<std::vector<std::string>>;

  template class SingleThreadParam<int>;
  template class SingleThreadParam<double>;
  template class SingleThreadParam<std::string>;
  template class SingleThreadParam<std::vector<int>>;
  template class SingleThreadParam<std::vector<double>>;
  template class SingleThreadParam<std::vector<std::string>>;

  template class MemoizedParam<int>;
  template class MemoizedParam<double>;
  template class MemoizedParam<std::string>;
  template class MemoizedParam<std::vector<int>>;
  template class MemoizedParam<std::vector<double>>;
  template class MemoizedParam<std::vector<std::string>>;

}
