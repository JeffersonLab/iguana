#include <iguana/algorithms/clas12/EventBuilderFilter.h>
#include <iguana/algorithms/clas12/LorentzTransformer.h>
#include <hipo4/reader.h>
#include <sstream>
#include <glib.h>

// Template function to read values of any C++ type from GKeyFile
template <typename T>
T readValue(GKeyFile *keyfile, const std::string &section, const std::string &key, T defaultValue) {
    GError *error = nullptr;

    // Get the raw string value from GKeyFile
    gchar *rawValue = g_key_file_get_string(keyfile, section.c_str(), key.c_str(), &error);

    if (error != nullptr) {
        g_error_free(error);
        return defaultValue;
    }

    // Convert the raw string value to the desired C++ type
    T value;
    std::istringstream(rawValue) >> value;

    // Free the raw string value
    g_free(rawValue);

    return value;
}

// Function to read an array of any C++ type from GKeyFile
template <typename T>
std::vector<T> readArray(GKeyFile *keyfile, const std::string &section, const std::string &key, const std::vector<T> &defaultValue) {
    GError *error = nullptr;

    // Get the raw string value from GKeyFile
    gchar *rawValue = g_key_file_get_string(keyfile, section.c_str(), key.c_str(), &error);

    if (error != nullptr) {
        g_error_free(error);
        return defaultValue;
    }

    // Convert the raw string value to a vector of the desired C++ type
    std::istringstream iss(rawValue);
    std::vector<T> value;
    std::string element;

    // Read the entire line as a string
    std::getline(iss, element);

    // Tokenize the string using ',' as the delimiter
    std::istringstream elementStream(element);
    while (std::getline(elementStream, element, ',')) {
        // Trim leading and trailing whitespaces
        element.erase(0, element.find_first_not_of(" \t"));
        element.erase(element.find_last_not_of(" \t") + 1);

        // Convert the element to the desired type
        T convertedElement;
        std::istringstream(element) >> convertedElement;

        value.push_back(convertedElement);
    }

    // Free the raw string value
    g_free(rawValue);

    return value;
}

int main(int argc, char **argv) {

  // parse arguments
  int argi = 1;
  const char* inFileName = argc > argi ? argv[argi++]            : "data.hipo";
  const int   numEvents  = argc > argi ? std::stoi(argv[argi++]) : 1;

  // read input file
  hipo::reader reader(inFileName);

  // Create a GKeyFile
  GKeyFile *keyfile;
  keyfile = g_key_file_new();

  // Load the INI file
  GError *error = nullptr;
  if (!g_key_file_load_from_file(keyfile, "/Users/tyson/iguana/examples/config_files/ex2.ini", G_KEY_FILE_NONE, &error))
  {
    g_printerr("Error loading INI file: %s\n", error->message);
    g_error_free(error);
    g_key_file_free(keyfile);
    return 1;
  }

  // Read values of different C++ types
  int intValue = readValue<int>(keyfile, "random", "int", 0);
  double pi = readValue<double>(keyfile, "random/floats", "pi", 0.0);
  std::string frameType = readValue<std::string>(keyfile, "frame", "type", "");

  // Read arrays of different C++ types
  std::vector<int> pids = readArray<int>(keyfile, "filter", "pids", {0});
  std::vector<double> doubleArray = readArray<double>(keyfile, "random/floats", "arr", {0.0});
  std::vector<std::string> banksArray = readArray<std::string>(keyfile, "banks", "names", {""});

  // Cleanup
  g_key_file_free(keyfile);

  std::cout << "Double Array: ";
  for (const auto &value : doubleArray)
  {
    std::cout << value << " ";
  }
  std::cout << std::endl;

  std::cout<<"PI "<<pi<<" int "<<intValue<<std::endl;

  // set banks
  hipo::banklist banks = reader.getBanks(banksArray);
  enum banks_enum { b_particle, b_calo }; // TODO: users shouldn't have to do this

  // create the algorithms
  iguana::clas12::EventBuilderFilter algo_eventbuilder_filter;
  iguana::clas12::LorentzTransformer algo_lorentz_transformer;

  // set log levels
  algo_eventbuilder_filter.SetOption("log", "debug");
  algo_lorentz_transformer.SetOption("log", "debug");

  // set algorithm options
  algo_eventbuilder_filter.SetOption<std::vector<int>>("pids", pids);
  algo_lorentz_transformer.SetOption("frame", frameType);

  std::vector<float> old_p;
  std::vector<float> new_p;

  // start the algorithms
  algo_eventbuilder_filter.Start();
  algo_lorentz_transformer.Start();

  // run the algorithm sequence on each event
  int iEvent = 0;
  while(reader.next(banks) && (numEvents==0 || iEvent++ < numEvents)) {

    // show the particle bank
    auto& particleBank = banks.at(b_particle);
    particleBank.show();

    // loop over bank rows
    for(int row=0; row<particleBank.getRows(); row++) {

      // check the PID with EventBuilderFilter
      auto pid = particleBank.getInt("pid", row);
      if(algo_eventbuilder_filter.Filter(pid)) {

        // if accepted PID, transform its momentum with LorentzTransformer
        auto [px, py, pz, e] = algo_lorentz_transformer.Transform(
            particleBank.getFloat("px", row),
            particleBank.getFloat("py", row),
            particleBank.getFloat("pz", row),
            0.0 // (ignoring the energy)
            );

        // then print the result
        fmt::print("Accepted PID {}:\n", pid);
        auto printMomentum = [] (auto v1, auto v2) { fmt::print("  {:>20}  {:>20}\n", v1, v2); };
        printMomentum("p_old", "p_new");
        printMomentum("--------", "--------");
        printMomentum(particleBank.getFloat("px", row), px);
        printMomentum(particleBank.getFloat("py", row), py);
        printMomentum(particleBank.getFloat("pz", row), pz);
        old_p={particleBank.getFloat("px", row),particleBank.getFloat("py", row),particleBank.getFloat("pz", row)};
        new_p={px,py,pz};

      }
    }
  }

  // writing out to file

  // Create a GKeyFile
  GKeyFile *keyfile_out = g_key_file_new();

  // Add sections and keys with values and comments
  g_key_file_set_string(keyfile_out, "frame", "#", "Can write a comment like this, not ideal though as preceded by '='...");
  g_key_file_set_string(keyfile_out, "frame", "##", "Using key '#' would overwrite previous comment.");
  g_key_file_set_string(keyfile_out, "frame", "description", "Mirror a particle three momentum # An inline comment");

  // Create a nested section under [frame]
  g_key_file_set_string(keyfile_out, "frame/old", "px", std::to_string(old_p[0]).c_str());
  g_key_file_set_string(keyfile_out, "frame/old", "py", std::to_string(old_p[1]).c_str());
  g_key_file_set_string(keyfile_out, "frame/old", "pz", std::to_string(old_p[2]).c_str());

  // Create a nested section under [frame]
  g_key_file_set_string(keyfile_out, "frame/new", "px", std::to_string(new_p[0]).c_str());
  g_key_file_set_string(keyfile_out, "frame/new", "py", std::to_string(new_p[1]).c_str());
  g_key_file_set_string(keyfile_out, "frame/new", "pz", std::to_string(new_p[2]).c_str());

  // Save the keyfile_out to a file
  GError *error_out = nullptr;
  if (!g_key_file_save_to_file(keyfile_out, "/Users/tyson/iguana/examples/config_files/output.ini", &error_out))
  {
    g_printerr("Error saving INI file: %s\n", error_out->message);
    g_error_free(error_out);
    g_key_file_free(keyfile_out);
    return 1;
  }

  // Cleanup
  g_key_file_free(keyfile_out);

  // stop the algorithms
  algo_eventbuilder_filter.Stop();
  algo_lorentz_transformer.Stop();
  return 0;
}
