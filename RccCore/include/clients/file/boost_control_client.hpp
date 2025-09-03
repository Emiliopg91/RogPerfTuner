#include "abstract/abstract_file_client.hpp"
#include "RccCommons.hpp"

class BoostControlClient : public AbstractFileClient
{
public:
    static BoostControlClient &getInstance()
    {
        static bool initialized = false;
        static std::string path;
        static std::string on;
        static std::string off;

        if (!initialized)
        {
            std::vector<std::map<std::string, std::string>> BOOST_CONTROLS = {
                {{"path", "/sys/devices/system/cpu/intel_pstate/no_turbo"},
                 {"on", "0"},
                 {"off", "1"}},
                {{"path", "/sys/devices/system/cpu/cpufreq/boost"},
                 {"on", "1"},
                 {"off", "0"}}};

            for (auto file_desc : BOOST_CONTROLS)
            {
                if (FileUtils::exists(file_desc.at("path")))
                {
                    path = file_desc.at("path");
                    on = file_desc.at("on");
                    off = file_desc.at("off");
                }
            }

            initialized = true;
        }

        static BoostControlClient instance(path, on, off);
        return instance;
    }

    void set_boost(bool &enabled)
    {
        write(enabled ? on : off);
    }

private:
    std::string on;
    std::string off;

    BoostControlClient(const std::string &path, const std::string &on, const std::string &off) : AbstractFileClient(path, "CPUInfoClient", true), on(on), off(off)
    {
    }
};