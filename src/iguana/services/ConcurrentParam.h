#pragma once

#include <oneapi/tbb/concurrent_hash_map.h>
#include <oneapi/tbb/concurrent_vector.h>

namespace iguana {

  using concurrent_key_t = std::size_t;

  template <class T>
  class ConcurrentParam {

    using memo_t = oneapi::tbb::concurrent_hash_map<concurrent_key_t, T>;
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

      ConcurrentParam(model_t model) : m_model(model) {};
      ConcurrentParam(std::string const& model) {
        for(auto const& [model_i, model_name] : m_model_names) {
          if(model == model_name) {
            m_model = model_i;
            return;
          }
        }
        throw std::runtime_error("concurrency model '" + model + "' is unrecognized");
      }
      ~ConcurrentParam() {}

      T const Load(concurrent_key_t const key) const {
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

      void Save(concurrent_key_t const key, T const& value) {
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

      bool HasKey(concurrent_key_t const key) const {
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

  };
}
