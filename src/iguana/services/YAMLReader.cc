#include "YAMLReader.h"

namespace iguana
{
    void YAMLReader::LoadFiles() {
      for(const auto& file : m_files) {
        try {
          m_configs.push_front(YAML::LoadFile(file));
        }
        catch (const YAML::Exception &e) {
          m_log->Error("YAML Exception: {}", e.what());
        }
        catch (const std::exception &e) {
          m_log->Error("Exception: {}", e.what());
        }
      }
    }

    template <typename T>
    T YAMLReader::readValue(const std::string &key, T defaultValue, const YAML::Node &node)
    {
      for(const auto& config : m_configs) {
        try
        {
            const YAML::Node &targetNode = node.IsNull() ? config : node;

            if (targetNode[key])
            {
                return targetNode[key].as<T>();
            }
        }
        catch (const YAML::Exception &e)
        {
            // Handle YAML parsing errors
            m_log->Error("YAML Exception: {}", e.what());
            return defaultValue;
        }
        catch (const std::exception &e)
        {
            // Handle other exceptions (e.g., conversion errors)
            m_log->Error("Exception: {}", e.what());
            return defaultValue;
        }
      }
      // not found in any config
      return defaultValue;
    }

    // Explicit instantiation for double
    template double YAMLReader::readValue<double>(const std::string &, const double, const YAML::Node &);
    // Explicit instantiation for int
    template int YAMLReader::readValue<int>(const std::string &, const int, const YAML::Node &);
    // Explicit instantiation for std::string
    template std::string YAMLReader::readValue<std::string>(const std::string &, const std::string, const YAML::Node &);

    template <typename T>
    std::vector<T> YAMLReader::readArray(const std::string &key, const std::vector<T> &defaultValue, const YAML::Node &node)
    {
      for(const auto& config : m_configs) {
        try
        {
            const YAML::Node &targetNode = node.IsNull() ? config : node;

            if (targetNode[key])
            {
                std::vector<T> value;
                const YAML::Node &arrayNode = targetNode[key];
                for (const auto &element : arrayNode)
                {
                    value.push_back(element.as<T>());
                }
                return value;
            }
        }
        catch (const YAML::Exception &e)
        {
            // Handle YAML parsing errors
            m_log->Error("YAML Exception: {}", e.what());
            return defaultValue;
        }
        catch (const std::exception &e)
        {
            // Handle other exceptions (e.g., conversion errors)
            m_log->Error("Exception: {}", e.what());
            return defaultValue;
        }
      }
      // not found in any config
      return defaultValue;
    }

    // Explicit instantiation for double
    template std::vector<double> YAMLReader::readArray<double>(const std::string &, const std::vector<double> &, const YAML::Node &);
    // Explicit instantiation for int
    template std::vector<int> YAMLReader::readArray<int>(const std::string &, const std::vector<int> &, const YAML::Node &);
    // Explicit instantiation for std::string
    template std::vector<std::string> YAMLReader::readArray<std::string>(const std::string &, const std::vector<std::string> &, const YAML::Node &);

    template <typename T>
    T YAMLReader::findKeyAtRunAndPID(
        const std::string &cutKey,
        const std::string &runkey,
        const std::string &pidkey,
        const std::string &key,
        const int         runnb,
        const int         pid,
        const T           defaultValue
        )
    {
      for(const auto& config : m_configs) {
        // Accessing the whole sequence of maps
        const YAML::Node &cutsNode = config[cutKey];
        if (cutsNode.IsSequence())
        {
            for (const auto &runNode : cutsNode)
            {

                std::vector<int> runs = readArray<int>(runkey, {}, runNode);
                if (runs.size() == 2 && runs[0] <= runnb && runs[1] >= runnb)
                {
                    if (runNode[pidkey].IsDefined())
                    {
                        const YAML::Node &pidNode = runNode[pidkey];
                        return readValue<T>(std::to_string(pid), defaultValue, pidNode);
                    }
                    else
                    {
                        return readValue<T>(key, defaultValue, runNode);
                    }
                }
            }
        }
      }
      // not found in any config
      return defaultValue;
    }

    // Explicit instantiation for double
    template double YAMLReader::findKeyAtRunAndPID<double>(const std::string &,const std::string &, const std::string &, const std::string &, int, int, const double);
    // Explicit instantiation for int
    template int YAMLReader::findKeyAtRunAndPID<int>(const std::string &,const std::string &, const std::string &, const std::string &, int, int, const int);
    // Explicit instantiation for std::string
    template std::string YAMLReader::findKeyAtRunAndPID<std::string>(const std::string &,const std::string &, const std::string &, const std::string &, int, int, const std::string);

    template <typename T>
    std::vector<T> YAMLReader::findKeyAtRunAndPIDVector(
        const std::string &cutKey,
        const std::string &runkey,
        const std::string &pidkey,
        const std::string &key,
        int runnb,
        int pid,
        const std::vector<T> &defaultValue
        )
    {
      for(const auto& config : m_configs) {
        // Accessing the whole sequence of maps

        const YAML::Node &cutsNode = config[cutKey];
        if (cutsNode.IsSequence())
        {
            for (const auto &runNode : cutsNode)
            {

                std::vector<int> runs = readArray<int>(runkey, {}, runNode);
                if (runs.size() == 2 && runs[0] <= runnb && runs[1] >= runnb)
                {
                    if (runNode[pidkey].IsDefined())
                    {
                        const YAML::Node &pidNode = runNode[pidkey];
                        return readArray<T>(std::to_string(pid), defaultValue, pidNode);
                    }
                    else
                    {
                        return readArray<T>(key, defaultValue, runNode);
                    }
                }
            }
        }
      }
      // not found in any config
      return defaultValue;
    }

    // Explicit instantiation for double
    template std::vector<double> YAMLReader::findKeyAtRunAndPIDVector<double>(const std::string &,const std::string &, const std::string &, const std::string &, int, int, const std::vector<double> &);
    // Explicit instantiation for int
    template std::vector<int> YAMLReader::findKeyAtRunAndPIDVector<int>(const std::string &,const std::string &, const std::string &, const std::string &, int, int, const std::vector<int> &);
    // Explicit instantiation for std::string
    template std::vector<std::string> YAMLReader::findKeyAtRunAndPIDVector<std::string>(const std::string &,const std::string &, const std::string &, const std::string &, int, int, const std::vector<std::string> &);
}
