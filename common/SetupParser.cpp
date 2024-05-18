#include "SetupParser.h"

std::ifstream setup_filepath((std::string)PROJECT_PATH + "/common/setup.json");
nlohmann::json SetupParser::setup_file_data = nlohmann::json::parse(setup_filepath);

std::string SetupParser::getValue(std::string key) {
	return setup_file_data[key];
};