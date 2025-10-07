#pragma once

#include "../../../../models/others/loggable.hpp"
#include "../asus_base_client.hpp"

class ArmouryBaseClient : public AsusBaseClient, Loggable {
  public:
	ArmouryBaseClient(std::string object_path_sufix, bool required = false);

	/**
	 * @brief Retrieves the minimum value supported or configured by the client.
	 *
	 * @return int The minimum value.
	 */
	int getMinValue();

	/**
	 * @brief Retrieves the maximum value supported or allowed.
	 *
	 * @return int The maximum value.
	 */
	int getMaxValue();

	/**
	 * @brief Retrieves the current value from the client.
	 *
	 * @return int The current value.
	 */
	int getCurrentValue();

	/**
	 * @brief Sets the current value to the specified integer.
	 *
	 * This method updates the current value with the provided integer parameter.
	 *
	 * @param value The new value to set.
	 */
	void setCurrentValue(int value);
};