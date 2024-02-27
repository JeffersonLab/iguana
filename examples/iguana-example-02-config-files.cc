#include <iguana/algorithms/clas12/ZVertexFilter.h>
#include <cassert>

int main(int argc, char** argv)
{

  // parse arguments
  std::string configDir;
  if(argc > 1)
    configDir = std::string(argv[1]);
  else
    configDir = iguana::ConfigFileReader::DirName(argv[0]) + "/../etc/iguana/examples";
  fmt::print("Using top-level configuration directory {}\n", configDir);

  // loop over multiple examples how to use configuration files and set options
  for(int example=1; example<=6; example++) {
    fmt::print("\n" + iguana::Logger::Header(fmt::format("CONFIG EXAMPLE {}", example)) + "\n");

    // start the algorithm; it will be destroyed at the end of each `example` iteration
    auto algo = std::make_unique<iguana::clas12::ZVertexFilter>();
    algo->SetLogLevel("debug");

    switch(example) {

      case 1:
        // Use the default configuration, from `../src/iguana/algorithms/clas12/ZVertexFilter.yaml`
        algo->SetOption("runnum", 4800);
        algo->Start();
        assert((algo->GetRunNum() == 4800));
        assert((algo->GetZcutLower() == -13.0));
        assert((algo->GetZcutUpper() == 12.0));
        break;

      case 2:
        // Use `SetOption` to set the cuts
        // - note that this will OVERRIDE any value used in any configuration file
        // - only use `SetOption` if for any reason you want to hard-code a specific value; usage
        //   of configuration files is preferred in general
        algo->SetOption<std::vector<double>>("zcuts", {-5.0, 3.0});
        algo->Start();
        assert((algo->GetZcutLower() == -5.0));
        assert((algo->GetZcutUpper() == 3.0));
        break;

      case 3:
        // Use a specific configuration file
        algo->SetConfigFile(configDir + "/my_z_vertex_cuts.yaml");
        algo->SetOption("runnum", 5500);
        algo->Start();
        assert((algo->GetZcutLower() == -8.0));
        assert((algo->GetZcutUpper() == 7.0));
        break;

      case 4:
        // Use the same specific configuration file, but don't set a run number;
        // note also the usage of `SetConfigDirectory`, as another example how to set a specific configuration file
        algo->SetConfigDirectory(configDir);
        algo->SetConfigFile("my_z_vertex_cuts.yaml");
        algo->Start();
        assert((algo->GetZcutLower() == -15.0));
        assert((algo->GetZcutUpper() == 15.0));
        break;

      case 5:
        // Use a custom directory of configuration files; if a configuration file within
        // has the same path and name as the default (`ZVertexFilter.yaml`), it will be used instead of the default.
        // This is designed such that if you copy the full installed configuration directory to a new location, you
        // may use that directory instead of the default, and modify any configuration file within.
        algo->SetConfigDirectory(configDir + "/my_config_directory");
        algo->Start();
        assert((algo->GetZcutLower() == -1.5));
        assert((algo->GetZcutUpper() == 1.3));
        break;

      case 6:
        // Use a single, combined configuration file; each algorithm's options are in a separate section
        algo->SetConfigDirectory(configDir);
        algo->SetConfigFile("my_combined_config_file.yaml");
        algo->Start();
        assert((algo->GetZcutLower() == -33.0));
        assert((algo->GetZcutUpper() == 11.0));
        break;

      default:
        fmt::print(stderr, "ERROR: unknown example number '{}'\n", example);
        return 1;
    }

    algo->Stop();
  }

  return 0;
}
