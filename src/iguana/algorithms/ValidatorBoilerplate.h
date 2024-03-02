/// @file ValidatorBoilerplate.h
/// @brief Preprocessor macros to generate standardized validator boilerplate code

/// Define the public and private members of an validator, along with its constructor and destructor; this
/// macro should be called in the `class` body in the validator's header file
/// @param VALIDATOR_NAME the name of the validator class
/// @param VALIDATOR_FULL_NAME the full name of this validator, used by `iguana::ValidatorFactory`
#define DEFINE_IGUANA_VALIDATOR(VALIDATOR_NAME, VALIDATOR_FULL_NAME)          \
private:                                                                      \
  static bool s_registered;                                                   \
                                                                              \
public:                                                                       \
  VALIDATOR_NAME(std::string name = "")                                       \
      : Validator(name == "" ? GetClassName() : name)                         \
  {}                                                                          \
  ~VALIDATOR_NAME() {}                                                        \
  static validator_t Creator() { return std::make_unique<VALIDATOR_NAME>(); } \
  static std::string GetClassName() { return #VALIDATOR_FULL_NAME; }

/// Register an validator for the `iguana::ValidatorFactory`; this macro should be called in the validator's implementation
/// @param VALIDATOR_NAME the name of the validator class
#define REGISTER_IGUANA_VALIDATOR(VALIDATOR_NAME) \
  bool VALIDATOR_NAME::s_registered = ValidatorFactory::Register(VALIDATOR_NAME::GetClassName(), VALIDATOR_NAME::Creator);
