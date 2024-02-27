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
  algo->SetConfigDirectory("src/iguana/tests"); // must be relative to build directory
  algo->SetConfigFile(fmt::format("test_{}.yaml", test_num));
  algo->Start();

  switch(test_num) {

  case 1: {
    // test `GetOptionScalar`
    assert((algo->GetOptionScalar<int>("scalar_int") == 1));
    assert((algo->GetOptionScalar<double>("scalar_double") == 2.5));
    assert((algo->GetOptionScalar<std::string>("scalar_string") == "lizard"));
    // test `GetOptionVector`
    assert((algo->GetOptionVector<int>("vector_int") == std::vector<int>{1, 2, 3}));
    assert((algo->GetOptionVector<double>("vector_double") == std::vector<double>{1.5, 2.5}));
    assert((algo->GetOptionVector<std::string>("vector_string") == std::vector<std::string>{"spider", "bat", "chameleon", "spider"}));
    // test `GetOptionSet`
    auto s = algo->GetOptionSet<std::string>("vector_string");
    assert((s.size() == 3));
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

  case 2: {
    assert((algo->GetOptionScalar<double>("t1s1", {"tree1", "scalar1"}) == 1.5));
    assert((algo->GetOptionScalar<double>("t1s2", {"tree1", "scalar2"}) == 2.5));
    assert((algo->GetOptionScalar<double>("t2t1s1", {"tree2", "tree1", "scalar1"}) == 3.5));
    assert((algo->GetOptionVector<std::string>("t2t2t3v1", {"tree2", "tree2", "tree3", "vector1"}) == std::vector<std::string>{"gecko", "snake"}));
    assert((algo->GetOptionVector<int>("t2v2", {"tree2", "vector2"}) == std::vector<int>{3, -4, 5}));
    assert((algo->GetOptionVector<std::string>("vector1", {"vector1"}) == std::vector<std::string>{"bee"}));
    assert((algo->GetOptionVector<std::string>("vector1") == std::vector<std::string>{"bee"}));
    // options are immutable; if cached once, they cannot be changed:
    assert((algo->GetOptionVector<std::string>("vector1", {"tree2", "tree2", "tree3", "vector1"}) == std::vector<std::string>{"bee"})); // != {"gecko", "snake"}
    assert((algo->GetOptionVector<std::string>("t2t2t3v1") == std::vector<std::string>{"gecko", "snake"}));
    break;
  }

  case 3: {
    // test InRange tree1
    assert((algo->GetOptionScalar<int>("t1a", {"tree1", algo->GetConfig()->InRange("test_range", 1), "val"}) == 3));
    assert((algo->GetOptionScalar<int>("t1b", {"tree1", algo->GetConfig()->InRange("test_range", 3), "val"}) == 3));
    assert((algo->GetOptionScalar<int>("t1c", {"tree1", algo->GetConfig()->InRange("test_range", 5), "val"}) == 3)); // at a border
    assert((algo->GetOptionScalar<int>("t1d", {"tree1", algo->GetConfig()->InRange("test_range", 6), "val"}) == 4));
    assert((algo->GetOptionScalar<int>("t1e", {"tree1", algo->GetConfig()->InRange("test_range", 10), "val"}) == 4));
    assert((algo->GetOptionScalar<int>("t1f", {"tree1", algo->GetConfig()->InRange("test_range", 11), "val"}) == 0)); // default fallback
    assert((algo->GetOptionScalar<int>("t1g", {"tree1", algo->GetConfig()->InRange("test_range", 10.1), "val"}) == 0)); // wrong type
    assert((algo->GetOptionScalar<int>("t1h", {"tree1", algo->GetConfig()->InRange("test_range", 3.7), "val"}) == 3)); // wrong type
    // test InRange tree2
    assert((algo->GetOptionScalar<std::string>("t2a", {"tree2", algo->GetConfig()->InRange("test_range", 1.9), "subtree", "lizard"}) == "iguana"));
    assert((algo->GetOptionScalar<int>("t2b", {"tree2", algo->GetConfig()->InRange("test_range", 1.9), "subtree", "number"}) == 7));
    assert((algo->GetOptionScalar<int>("t2c", {"tree2", algo->GetConfig()->InRange("test_range", 3.0), "subtree", algo->GetConfig()->InRange("sub_range", 1), "val"}) == 7));
    assert((algo->GetOptionScalar<int>("t2d", {"tree2", algo->GetConfig()->InRange("test_range", 3.0), "subtree", algo->GetConfig()->InRange("sub_range", 8), "val"}) == 8));
    assert((algo->GetOptionScalar<int>("t2e", {"tree2", algo->GetConfig()->InRange("test_range", 3.5), "subtree", algo->GetConfig()->InRange("sub_range", 11), "val"}) == 1));
    assert((algo->GetOptionScalar<int>("t2f", {"tree2", algo->GetConfig()->InRange("test_range", 4.0), "subtree"}) == 10));
    // test InRange tree3
    assert((algo->GetOptionScalar<int>("t3a", {"tree3", algo->GetConfig()->InRange("test_range", 3), "val"}) == 3));
    try {
      assert((algo->GetOptionScalar<int>("t3b", {"tree3", algo->GetConfig()->InRange("test_range", 11), "val"}) == 0));
      fmt::print(stderr, "ERROR: accessing a missing default value for `InRange` did not throw exception\n");
      return 1;
    }
    catch(const std::exception& ex) {
      fmt::print("SUCCESS: accessing a missing default value for `InRange` threw expected exception\n");
    }
    break;
  }

  default:
    fmt::print(stderr, "ERROR: unknown test number '{}'", test_num);
    return 1;
  }
  return 0;
}
