#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>

#include "RccCommons.hpp"

#include "../../../../include/clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../../../../include/clients/shell/asusctl_client.hpp"

OpenRgbClient::OpenRgbClient()
{
    logger.info("Initializing OpenRgbClient");
    logger.add_tab();

    if (AsusCtlClient::getInstance().available())
    {
        AsusCtlClient::getInstance().turnOffAura();
    }
    startOpenRgbProcess();
    startOpenRgbClient();
    getAvailableDevices();

    loadCompatibleDevices();
    logger.rem_tab();
}

void OpenRgbClient::loadCompatibleDevices()
{
    logger.info("Loading supported devices");

    std::ifstream file(Constants::ORGB_RULES_FILE);
    if (!file.is_open())
    {
        logger.error("Couldn't load " + Constants::ORGB_RULES_FILE);
        throw std::runtime_error("Couldn't load " + Constants::ORGB_RULES_FILE);
    }

    std::string line;

    // Raw string literal con delimitador custom para evitar conflicto con comillas
    std::regex regex(R"delimiter(SUBSYSTEMS==".*?", ATTRS\{idVendor\}=="([0-9a-fA-F]+)", ATTRS\{idProduct\}=="([0-9a-fA-F]+)".*?TAG\+="([a-zA-Z0-9_]+)")delimiter");

    while (std::getline(file, line))
    {
        // eliminar ', TAG+="uaccess"' si existe
        size_t pos = line.find(", TAG+=\"uaccess\"");
        if (pos != std::string::npos)
        {
            line.erase(pos, 15);
        }

        std::smatch match;
        if (std::regex_search(line, match, regex))
        {
            std::string vendor_id = match[1];
            std::string product_id = match[2];
            std::string device_name = match[3];

            std::replace(device_name.begin(), device_name.end(), '_', ' ');

            compatibleDevices.emplace_back(UsbIdentifier{vendor_id, product_id, device_name});
            compatibleDeviceNames[vendor_id + ":" + product_id] = device_name;
        }
    }

    logger.debug("Detected support for " + std::to_string(compatibleDevices.size()) + " devices");
}

std::vector<UsbIdentifier> OpenRgbClient::getCompatibleDevices()
{
    return compatibleDevices;
}

void OpenRgbClient::startOpenRgbProcess()
{
    logger.info("Starting OpenRGB server");
    logger.add_tab();
    port = NetUtils::getRandomFreePort();
    runnerThread = std::thread(&OpenRgbClient::runner, this);
    while (NetUtils::isPortFree(port))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    logger.info("OpenRgb server ready");
    logger.rem_tab();
}

void OpenRgbClient::runner()
{
    std::string hostArg = "--server-host";
    std::string hostVal = "localhost";
    std::string portArg = "--server-port";
    std::string portVal = std::to_string(port);

    // Vector de strings para mantener la memoria válida
    std::vector<std::string> argsStr = {hostArg, hostVal, portArg, portVal};

    // Vector de punteros a char terminado en nullptr
    std::vector<char *> argv;
    argv.push_back((char *)Constants::ORGB_PATH.c_str()); // argv[0] normalmente es el nombre del binario
    for (auto &s : argsStr)
        argv.push_back((char *)s.c_str());
    argv.push_back(nullptr); // execve requiere nullptr al final

    auto env = Shell::getInstance().copyEnviron();
    std::string ld = "LD_LIBRARY_PATH=\"\"";
    env.push_back(ld.data());
    int exit_code = Shell::getInstance().run_process(Constants::ORGB_PATH.c_str(), argv.data(), env.data(), Constants::LOG_ORGB_FILE);
    logger.info("Command finished with exit code " + std::to_string(exit_code));
}

void OpenRgbClient::startOpenRgbClient()
{
    logger.info("Connecting to server");
    logger.add_tab();
    client.connect("localhost", port);
    logger.info("Connected");
    logger.rem_tab();
}

void OpenRgbClient::getAvailableDevices()
{
    logger.info("Getting available devices");
    logger.add_tab();

    auto devList = client.requestDeviceList();
    for (auto &dev : devList.devices)
    {
        const orgb::Mode *directMode = dev.findMode("Direct");
        if (directMode)
        {
            logger.info(dev.name);
            client.changeMode(dev, *directMode);
            client.setDeviceColor(dev, orgb::Color::Black);
        }
    }

    logger.rem_tab();
}