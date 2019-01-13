/*
 * demo.cpp
 *
 *  Created on: Nov 4, 2018
 *      Author: mike
 */

#include <iostream>
#include "yaml-cpp/yaml.h"
#include "verde.hpp"

int main() {
	verde::ParserHelper parser_helper("schema.yaml");
	try{
		const bool success = parser_helper.validate_configuration_file("config.yaml");
	}
	catch(const std::logic_error& e){
		std::cout << e.what() << '\n';
		return -1;
	}
}

