#include <nlohmann/json.hpp>
#include <string>
#include <fstream>


class SetupParser {
public:
	static nlohmann::json setup_file_data;
	static std::string getValue(std::string);
};