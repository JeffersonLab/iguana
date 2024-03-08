/// @file AlgorithmBoilerplate.h
/// @brief Preprocessor macros to generate standardized algorithm boilerplate code

/// Generate an algorithm constructor
/// @param ALGO_NAME the name of the algorithm class
/// @param BASE_NAME the name of the base class
#define CONSTRUCT_IGUANA_ALGORITHM(ALGO_NAME, BASE_NAME) \
  ALGO_NAME(std::string name = "")                       \
      : BASE_NAME(name == "" ? GetClassName() : name)    \
  {                                                      \
    m_default_config_file = GetDefaultConfigFile();      \
    m_class_name          = GetClassName();              \
  }

/// Generate an algorithm destructor
/// @param ALGO_NAME the name of the algorithm class
#define DESTROY_IGUANA_ALGORITHM(ALGO_NAME) \
  ~ALGO_NAME() {}

/////////////////////////////////////////////////////////////////////////////////

/// Define the public members of an algorithm
/// @param ALGO_NAME the name of the algorithm class
/// @param ALGO_FULL_NAME the full name of this algorithm, used by `iguana::AlgorithmFactory`
/// @param BASE_NAME the name of the base class
#define IGUANA_ALGORITHM_PUBLIC_MEMBERS(ALGO_NAME, ALGO_FULL_NAME, BASE_NAME) \
  using BASE_NAME::Start;                                                     \
  static algo_t Creator() { return std::make_unique<ALGO_NAME>(); }           \
  static std::string GetClassName() { return #ALGO_FULL_NAME; }               \
  static std::string GetDefaultConfigFile()                                   \
  {                                                                           \
    return ConfigFileReader::ConvertAlgoNameToConfigName(#ALGO_FULL_NAME);    \
  }

/// Define the private members of an algorithm
#define IGUANA_ALGORITHM_PRIVATE_MEMBERS \
  static bool s_registered;

/////////////////////////////////////////////////////////////////////////////////

/// Implementation for definining the public and private members of an `Algorithm`-derived class
/// @param ALGO_NAME the name of the algorithm class
/// @param ALGO_FULL_NAME the full name of this algorithm, used by `iguana::AlgorithmFactory`
/// @param BASE_NAME the name of the base class, which may be `Algorithm` or one of its derived classes
#define DEFINE_IGUANA_ALGORITHM_IMPL(ALGO_NAME, ALGO_FULL_NAME, BASE_NAME) \
private:                                                                   \
  IGUANA_ALGORITHM_PRIVATE_MEMBERS                                         \
public:                                                                    \
  CONSTRUCT_IGUANA_ALGORITHM(ALGO_NAME, BASE_NAME)                         \
  DESTROY_IGUANA_ALGORITHM(ALGO_NAME)                                      \
  IGUANA_ALGORITHM_PUBLIC_MEMBERS(ALGO_NAME, ALGO_FULL_NAME, BASE_NAME)

/// Define the public and private members of an algorithm, along with its constructor and destructor; this
/// macro should be called in the `class` body in the algorithm's header file
/// @param ALGO_NAME the name of the algorithm class
/// @param ALGO_FULL_NAME the full name of this algorithm, used by `iguana::AlgorithmFactory`
#define DEFINE_IGUANA_ALGORITHM(ALGO_NAME, ALGO_FULL_NAME) \
  DEFINE_IGUANA_ALGORITHM_IMPL(ALGO_NAME, ALGO_FULL_NAME, Algorithm)

/// Define the public and private members of a validator, along with its constructor and destructor; this
/// macro should be called in the `class` body in the validator's header file
/// @param VDOR_NAME the name of the validator class
/// @param VDOR_FULL_NAME the full name of this validator, used by `iguana::AlgorithmFactory`
#define DEFINE_IGUANA_VALIDATOR(VDOR_NAME, VDOR_FULL_NAME) \
  DEFINE_IGUANA_ALGORITHM_IMPL(VDOR_NAME, VDOR_FULL_NAME, Validator)

/////////////////////////////////////////////////////////////////////////////////

/// Register an algorithm for the `iguana::AlgorithmFactory`; this macro should be called in the algorithm's implementation
/// @param ALGO_NAME the name of the algorithm class
#define REGISTER_IGUANA_ALGORITHM(ALGO_NAME) \
  bool ALGO_NAME::s_registered = AlgorithmFactory::Register(ALGO_NAME::GetClassName(), ALGO_NAME::Creator);

/// Register a validator for the `iguana::AlgorithmFactory`, similar to `REGISTER_IGUANA_ALGORITHM`
/// @param VDOR_NAME the name of the validator class
#define REGISTER_IGUANA_VALIDATOR(VDOR_NAME) \
  REGISTER_IGUANA_ALGORITHM(VDOR_NAME)
