// test configuration

#include <cassert>
#include <iguana/algorithms/Algorithm.h>

inline int TestConfig(int test_num, bool verbose)
{
  if(test_num == 0) {
    fmt::print(stderr, "ERROR: need a test number\n");
    return 1;
  }
  auto algo = iguana::AlgorithmFactory::Create("example::ExampleAlgorithm");
  algo->SetOption("log", verbose ? "debug" : "info");
  algo->SetOption("config_file", fmt::format("src/iguana/tests/test_{}.yaml", test_num)); // must be relative to build directory
  algo->Start();

  switch(test_num) {

    case 1:
    {
      // test `GetOptionScalar`
      assert((algo->GetOptionScalar<int>("scalar_int") == 1));
      assert((algo->GetOptionScalar<double>("scalar_double") == 2.5));
      assert((algo->GetOptionScalar<std::string>("scalar_string") == "lizard"));
      // test `GetOptionVector`
      assert((algo->GetOptionVector<int>("vector_int") == std::vector<int>{1, 2, 3}));
      assert((algo->GetOptionVector<double>("vector_double") == std::vector<double>{1.5, 2.5}));
      assert((algo->GetOptionVector<std::string>("vector_string") == std::vector<std::string>{"bat", "chameleon", "spider"}));
      // test `GetOptionSet`
      auto s = algo->GetOptionSet<std::string>("vector_string");
      assert((s.find("spider") != s.end()));
      assert((s.find("bee") == s.end()));
      // test empty access - expect exceptions, so just catch them and do nothing
      try {
        algo->GetOptionScalar<int>("scalar_empty");
        fmt::print(stderr, "ERROR: accessing 'scalar_empty' did not throw exception\n");
        return 1;
      }
      catch(const std::exception& ex) {
        fmt::print("SUCCESS: accessing 'scalar_empty' threw an expected exception\n");
      }
      try {
        algo->GetOptionVector<int>("vector_empty");
        fmt::print(stderr, "ERROR: accessing 'vector_empty' did not throw exception\n");
        return 1;
      }
      catch(const std::exception& ex) {
        fmt::print("SUCCESS: accessing 'vector_empty' threw an expected exception\n");
      }
      try {
        algo->GetOptionSet<int>("vector_empty");
        fmt::print(stderr, "ERROR: accessing 'vector_empty' as a `set` did not throw exception\n");
        return 1;
      }
      catch(const std::exception& ex) {
        fmt::print("SUCCESS: accessing 'vector_empty' as a `set` threw an expected exception\n");
      }
      // test access to a key that does not exist
      try {
        algo->GetOptionScalar<int>("non_existent_scalar");
        fmt::print(stderr, "ERROR: accessing 'non_existent_scalar' did not throw exception\n");
        return 1;
      }
      catch(const std::exception& ex) {
        fmt::print("SUCCESS: accessing 'non_existent_scalar' threw an expected exception\n");
      }
      try {
        algo->GetOptionVector<int>("non_existent_vector");
        fmt::print(stderr, "ERROR: accessing 'non_existent_vector' did not throw exception\n");
        return 1;
      }
      catch(const std::exception& ex) {
        fmt::print("SUCCESS: accessing 'non_existent_vector' threw an expected exception\n");
      }
      try {
        algo->GetOptionSet<int>("non_existent_vector");
        fmt::print(stderr, "ERROR: accessing 'non_existent_vector' as a `set` did not throw exception\n");
        return 1;
      }
      catch(const std::exception& ex) {
        fmt::print("SUCCESS: accessing 'non_existent_vector' as a `set` threw an expected exception\n");
      }
      break;
    }

    default:
      fmt::print(stderr, "ERROR: unknown test number '{}'", test_num);
      return 1;
  }
  return 0;
}
