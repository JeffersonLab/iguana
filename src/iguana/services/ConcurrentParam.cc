#include "ConcurrentParam.h"

namespace iguana {

  template <typename T>
  ConcurrentParam<T>::ConcurrentParam(model_t model)
    : m_model(model)
  {
  }

  template <typename T>
  ConcurrentParam<T>::ConcurrentParam(std::string const& model) {
    for(auto const& [model_i, model_name] : m_model_names) {
      if(model == model_name) {
        m_model = model_i;
        return;
      }
    }
    throw std::runtime_error("concurrency model '" + model + "' is unrecognized");
  }

  template <typename T>
  T const ConcurrentParam<T>::Load(concurrent_key_t const key) const {
    switch(m_model) {
      case none:
        return m_value;
      case memoize:
        {
          typename memo_t::const_accessor acc;
          if(m_memo.find(acc, key))
            return acc->second;
          break;
        }
      case threadpool:
        throw std::runtime_error("TODO: 'threadpool' model not yet implemented");
    }
    throw std::runtime_error("ConcurrentParam::HasKey failed");
  }

  template <typename T>
  void ConcurrentParam<T>::Save(concurrent_key_t const key, T const& value) {
    switch(m_model) {
      case none:
        m_value = value;
      case memoize:
        {
          typename memo_t::accessor acc;
          m_memo.insert(acc, key);
          acc->second = value;
          break;
        }
      case threadpool:
        throw std::runtime_error("TODO: 'threadpool' model not yet implemented");
    }
  }

  template <typename T>
  bool ConcurrentParam<T>::HasKey(concurrent_key_t const key) const {
    switch(m_model) {
      case none:
        throw std::runtime_error("do not call ConcurrentParam::HasKey when model is 'none'");
      case memoize:
        {
          typename memo_t::const_accessor acc;
          return m_memo.find(acc, key);
          break;
        }
      case threadpool:
        throw std::runtime_error("TODO: 'threadpool' model not yet implemented");
    }
    throw std::runtime_error("ConcurrentParam::HasKey failed");
  }

}
