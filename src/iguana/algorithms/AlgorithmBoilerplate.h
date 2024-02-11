/// @file AlgorithmBoilerplate.h
/// @brief Preprocessor macros to generate standardized algorithm boilerplate code

/// Generate an algorithm constructor
/// @param ALGO_NAME the name of the algorithm class
#define CONSTRUCT_IGUANA_ALGORITHM(ALGO_NAME)                                    \
  ALGO_NAME(std::string name="") : Algorithm(name=="" ? GetClassName() : name) { \
    m_default_config_file = GetDefaultConfigFile();                              \
  }

/// Generate an algorithm destructor
/// @param ALGO_NAME the name of the algorithm class
#define DESTROY_IGUANA_ALGORITHM(ALGO_NAME) \
  ~ALGO_NAME() {}

/// Define the public members of an algorithm
/// @param ALGO_NAME the name of the algorithm class
/// @param ALGO_FULL_NAME the full name of this algorithm, used by `iguana::AlgorithmFactory`
#define IGUANA_ALGORITHM_PUBLIC_MEMBERS(ALGO_NAME, ALGO_FULL_NAME)  \
                                                                    \
  using Algorithm::Start;                                           \
  static algo_t Creator() { return std::make_unique<ALGO_NAME>(); } \
  static std::string GetClassName() { return #ALGO_FULL_NAME; }     \
  static std::string GetDefaultConfigFile() {                       \
    return ConfigFileReader::ConvertAlgoNameToConfigName(#ALGO_FULL_NAME, "yaml"); \
  }

/// Define the private members of an algorithm
#define IGUANA_ALGORITHM_PRIVATE_MEMBERS \
  static bool s_registered;

/// Define the public and private members of an algorithm, along with its constructor and destructor; this
/// macro should be called in the `class` body in the algorithm's header file
/// @param ALGO_NAME the name of the algorithm class
/// @param ALGO_FULL_NAME the full name of this algorithm, used by `iguana::AlgorithmFactory`
#define DEFINE_IGUANA_ALGORITHM(ALGO_NAME, ALGO_FULL_NAME) \
  private:                                                 \
    IGUANA_ALGORITHM_PRIVATE_MEMBERS                       \
  public:                                                  \
    CONSTRUCT_IGUANA_ALGORITHM(ALGO_NAME)                  \
    DESTROY_IGUANA_ALGORITHM(ALGO_NAME)                    \
    IGUANA_ALGORITHM_PUBLIC_MEMBERS(ALGO_NAME, ALGO_FULL_NAME)

/// Register an algorithm for the `iguana::AlgorithmFactory`; this macro should be called in the algorithm's implementation
/// @param ALGO_NAME the name of the algorithm class
#define REGISTER_IGUANA_ALGORITHM(ALGO_NAME) \
  bool ALGO_NAME::s_registered = AlgorithmFactory::Register(ALGO_NAME::GetClassName(), ALGO_NAME::Creator);
