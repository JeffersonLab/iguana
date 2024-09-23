#include <cassert>
#include <iguana/algorithms/clas12/ZVertexFilter/Algorithm.h>

/// @begin_doc_example{cpp}
/// @file iguana_ex_cpp_config_files.cc
/// @brief Example showing how to control algorithm configuration.
///
/// Examples include:
/// - hard-coding a configuration setting override
/// - using a configuration file
/// - using a directory of configuration files
///
/// @par Usage
/// ```bash
/// iguana_ex_cpp_config_files [CONFIG_FILE_DIRECTORY]
///
///   CONFIG_FILE_DIRECTORY    a custom directory with config files
///                            (default = an example directory)
/// ```
/// @end_doc_example

/// main function
int main(int argc, char** argv)
{

  // parse arguments
  std::string configDir;
  if(argc > 1)
    configDir = std::string(argv[1]);
  else
    configDir = iguana::ConfigFileReader::GetConfigInstallationPrefix() + "/examples";
  fmt::print("Using top-level configuration directory {}\n", configDir);

  // loop over multiple examples how to use configuration files and set options
  for(int example = 1; example <= 6; example++) {
    fmt::print("\n" + iguana::Logger::Header(fmt::format("CONFIG EXAMPLE {}", example)) + "\n");

    // start the algorithm; it will be destroyed at the end of each `example` iteration
    auto algo = std::make_unique<iguana::clas12::ZVertexFilter>();
    algo->SetLogLevel("debug");

    switch(example) {

    case 1:
      {
        // Use the default configuration, from `../src/iguana/algorithms/clas12/ZVertexFilter.yaml`
        algo->Start();
        auto key = algo->PrepareEvent(4800, 0); // sets the run number and loads the cuts
        assert((algo->GetRunNum(key) == 4800)); // pass the key into the 'Get*' methods
        assert((algo->GetZcuts(key).at(0) == -13.0));
        assert((algo->GetZcuts(key).at(1) == 12.0));
        break;
      }

    case 2:
      {
        // Use `SetZcuts` to set the cuts
        // - note that this will OVERRIDE any value used in any configuration file
        // - only use `SetZcuts` if for any reason you want to hard-code a specific value; usage
        //   of configuration files is preferred in general
        algo->Start();
        iguana::concurrent_key_t const key = 0; // need the same key in `SetZcuts` and `GetZcuts`
        algo->SetZcuts(-5.0, 3.0, key);
        assert((algo->GetZcuts(key).at(0) == -5.0));
        assert((algo->GetZcuts(key).at(1) == 3.0));
        break;
      }

    case 3:
      {
        // Use a specific configuration file
        algo->SetConfigFile(configDir + "/my_z_vertex_cuts.yaml");
        algo->Start();
        auto key = algo->PrepareEvent(5500, 0);
        assert((algo->GetZcuts(key).at(0) == -0.8));
        assert((algo->GetZcuts(key).at(1) == 0.7));
        break;
      }

    case 4:
      {
        // Use the same specific configuration file, but don't set a run number;
        // note also the usage of `SetConfigDirectory`, as another example how to set a specific configuration file
        algo->SetConfigDirectory(configDir);
        algo->SetConfigFile("my_z_vertex_cuts.yaml");
        algo->Start();
        auto key = algo->PrepareEvent(0, 0); // run number "0" means "no run number"
        assert((algo->GetZcuts(key).at(0) == -1.5));
        assert((algo->GetZcuts(key).at(1) == 1.3));
        break;
      }

    case 5:
      {
        // Use a custom directory of configuration files; if a configuration file within
        // has the same path and name as the default (`ZVertexFilter.yaml`), it will be used instead of the default.
        // This is designed such that if you copy the full installed configuration directory to a new location, you
        // may use that directory instead of the default, and modify any configuration file within.
        algo->SetConfigDirectory(configDir + "/my_config_directory");
        algo->Start();
        auto key = algo->PrepareEvent(0, 0); // run number "0" means "no run number"
        assert((algo->GetZcuts(key).at(0) == -15.0));
        assert((algo->GetZcuts(key).at(1) == 15.0));
        break;
      }

    case 6:
      {
        // Use a single, combined configuration file; each algorithm's options are in a separate section
        algo->SetConfigDirectory(configDir);
        algo->SetConfigFile("my_combined_config_file.yaml");
        algo->Start();
        auto key = algo->PrepareEvent(0, 0); // run number "0" means "no run number"
        assert((algo->GetZcuts(key).at(0) == -33.0));
        assert((algo->GetZcuts(key).at(1) == 11.0));
        break;
      }

    default:
      {
        fmt::print(stderr, "ERROR: unknown example number '{}'\n", example);
        return 1;
      }
    }

    algo->Stop();
  }

  return 0;
}
