#include <cstdlib>
#include <fstream>
#include <iostream>

#include "Sensor.h"
#include "json.hpp"

void usage(char** argv) {
  std::cout << "Application: " << argv[0]
            << " requires the path to the config file." << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    usage(argv);
    exit(EXIT_FAILURE);
  }

  std::ifstream config_file(argv[1]);

  if (!config_file.good()) {
    std::cout << "File: " << argv[1] << " cannot be accessed." << std::endl;
    exit(EXIT_FAILURE);
  }

  nlohmann::json config_json;
  config_file >> config_json;

  for (auto& [key, value] : config_json["sensors"].items()) {
    std::cout << key << " : " << value << "\n";
  }

  return 1;
}