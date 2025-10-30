#include "httplib.h"

const std::string DOMAIN	  = "api.counterapi.dev";
const std::string PROJECT_URL = "/v2/emilio-pulido-gils-team-1479/ropgerftu";
const std::string TOKEN		  = "ut_B44NTa3SW6lTAmxIbaGo2yMLpqg9FU4uqk40YCHO";

inline void enroll() {
	httplib::SSLClient cli(DOMAIN);
	auto res = cli.Get(PROJECT_URL + "/up", {{"Authorization", std::string("Bearer ") + TOKEN}});
	if (res && res->status == 200) {
		std::cout << "Successful enrollment" << std::endl;
		return;
	}
	std::cout << "Enrollment failed, skipping" << std::endl;
}

inline void unenroll() {
	httplib::SSLClient cli(DOMAIN);
	auto res = cli.Get(PROJECT_URL + "/down", {{"Authorization", std::string("Bearer ") + TOKEN}});
	if (res && res->status == 200) {
		std::cout << "Successful unrollment" << std::endl;
		return;
	}
	std::cout << "Unenrollment failed, skipping" << std::endl;
}