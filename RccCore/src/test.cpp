/*#include <QApplication>
#include <unistd.h>
#include "httplib.h"

#include "../include/clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../include/servers/http/http_server.hpp"
#include "../include/services/application_service.hpp"
#include "../include/services/hardware_service.hpp"
#include "../include/services/open_rgb_service.hpp"
#include "../include/services/profile_service.hpp"
#include "../include/services/steam_service.hpp"
#include "../include/configuration/configuration.hpp"
#include "../include/gui/tray_icon.hpp"
#include "../include/gui/password_dialog.hpp"
#include "../include/gui/toaster.hpp"
#include "../include/logger/logger.hpp"
#include "../include/shell/shell.hpp"
#include "../include/translator/translator.hpp"
#include "../include/utils/constants.hpp"

void getApplicablePIDs(const pid_t parentId, std::set<pid_t> &pids)
{
	for (auto &entry : std::filesystem::directory_iterator("/proc"))
	{
		std::string filename = entry.path().filename();
		if (entry.is_directory() && std::all_of(filename.begin(), filename.end(), ::isdigit))
		{
			auto pid = std::stoi(filename);
			if (pids.find(pid) == pids.end())
			{
				std::ifstream statFile("/proc/" + filename + "/stat");
				if (statFile.is_open())
				{
					int pid_read, ppid;
					std::string comm, state;
					statFile >> pid_read >> comm >> state >> ppid;

					if (ppid == parentId)
					{
						pids.insert(pid);
						getApplicablePIDs(pid, pids);
					}
				}
			}
		}
	}
}

int main(int, char **)
{
	pid_t pid = 1508283;

	std::set<pid_t> pids;
	pids.insert(pid);

	auto t0 = std::chrono::high_resolution_clock::now();
	getApplicablePIDs(pid, pids);
	auto t1 = std::chrono::high_resolution_clock::now();

	for (auto p : pids)
	{
		std::cout << p << std::endl;
	}

	std::cout << std::endl
			  << std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()) << std::endl;
}*/
