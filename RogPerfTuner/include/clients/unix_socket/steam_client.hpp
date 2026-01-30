#pragma once

#include "abstracts/clients/abstract_unix_socket_client.hpp"
#include "abstracts/singleton.hpp"
#include "models/steam/steam_game_details.hpp"

class SteamSocketClient : public AbstractUnixSocketClient, public Singleton<SteamSocketClient> {
  private:
	SteamSocketClient();
	friend class Singleton<SteamSocketClient>;

  public:
	/**
	 * @brief Registers a callback to be invoked when a game is launched.
	 *
	 * This function allows you to specify a callback that will be called with parameters
	 * when a game launch event is detected. The callback should match the signature
	 * expected by CallbackWithParams.
	 *
	 * @param callback The callback function to be executed on game launch.
	 */
	void onGameLaunch(CallbackWithParams&& callback);

	/**
	 * @brief Registers a callback to be invoked when a game stops.
	 *
	 * This function allows you to specify a callback that will be called with parameters
	 * when a game stop event is detected.
	 *
	 * @param callback A callable object (such as a lambda or function) that accepts the required parameters
	 *                 and will be executed when the game stop event occurs.
	 */
	void onGameStop(CallbackWithParams&& callback);

	/**
	 * @brief Retrieves detailed information for a list of Steam applications.
	 *
	 * Given a vector of Steam application IDs, this function queries and returns
	 * detailed information for each application as a vector of SteamGameDetails objects.
	 *
	 * @param appIds A vector containing the Steam application IDs to query.
	 * @return A vector of SteamGameDetails objects containing details for each requested application.
	 */
	const std::vector<SteamGameDetails> getAppsDetails(const std::vector<unsigned int>& appIds);

	/**
	 * @brief Sets the launch options for a specific Steam application.
	 *
	 * This function assigns custom launch options to the Steam application identified by the given app ID.
	 *
	 * @param appid The unique identifier of the Steam application.
	 * @param launchOpts The launch options to be set for the application.
	 */
	void setLaunchOptions(const int& appid, const std::string& launchOpts);

	/**
	 * @brief Retrieves the icon associated with a given Steam application ID.
	 *
	 * This function returns the icon (typically as a URL or file path) for the specified Steam app.
	 *
	 * @param appid The unique identifier of the Steam application.
	 * @return A string representing the icon for the specified app ID.
	 */
	std::string getIcon(const int& appid);
};