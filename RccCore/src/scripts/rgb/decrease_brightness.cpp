#include <iostream>

#include "../../../include/utils/constants.hpp"
#include "httplib.h"

int main() {
	httplib::Client cli("localhost", Constants::HTTP_PORT);

	if (auto res = cli.Get(Constants::URL_DEC_BRIGHT)) {
		if (res->status == 200) {
			std::cout << res->body << std::endl;
		} else {
			std::cout << "HTTP code: " << res->status << std::endl;
		}
	} else {
		throw new std::runtime_error("Couldn't connect to server");
	}
}