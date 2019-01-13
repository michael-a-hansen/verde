#include "verde.hpp"
#include <iostream> // todo: get rid of this

namespace verde {

SyntaxValidator::SyntaxValidator(const YAML::Node& config_node) :
		config_node_(config_node) {
}

void SyntaxValidator::visit(SchemaNodeBase& node) {
	node.accept(*this);
}

void MapSchemaNode::accept(SyntaxValidator& v) {
	const YAML::Node& config_node = v.get_config_node();

	if (config_node.Type() == YAML::NodeType::Map) {
		std::vector<std::string> config_keys;
		for (const auto& keyval : config_node) {
			config_keys.push_back(keyval.first.as<std::string>());
		}

		for (const auto& reqd_nn : required_nodes_) {
			const std::string key = reqd_nn.first;
			if (std::find(config_keys.begin(), config_keys.end(), key)
					== config_keys.end()) {
				throw MissingRequiredKeyFailure(get_name(), key);
			}
		}

		for (const auto& key : config_keys) {
			const bool is_required = required_nodes_.find(key)
					!= required_nodes_.end();
			const bool is_optional = optional_nodes_.find(key)
					!= optional_nodes_.end();

			if (not is_required and not is_optional) {
				throw InvalidKeyFailure(get_name(), key, required_nodes_string_,
						optional_nodes_string_);
			}
			if (has_required_ and not has_optional_) {
				if (not is_required) {
					throw InvalidKeyFailure(get_name(), key,
							required_nodes_string_, optional_nodes_string_);
				}
			}
			if (not has_required_ and has_optional_) {
				if (not is_optional) {
					throw InvalidKeyFailure(get_name(), key,
							required_nodes_string_, optional_nodes_string_);
				}
			}

			if (is_required) {
				SyntaxValidator e(config_node[key]);
				required_nodes_.at(key)->accept(e);
			} else {
				SyntaxValidator e(config_node[key]);
				optional_nodes_.at(key)->accept(e);
			}
		}
	} else {
		throw TypeValidationFailure(get_name());
	}
}

void VectorSchemaNode::accept(SyntaxValidator& v) {
	const YAML::Node& config_node = v.get_config_node();

	if (config_node.Type() == YAML::NodeType::Sequence) {
		const unsigned int length = config_node.size();
		bool bad_length = false;
		std::string min_str = "unspecified";
		std::string max_str = "unspecified";
		if (check_minimum_length_) {
			bad_length = bad_length or length < minimum_length_;
			min_str = std::to_string(minimum_length_);
		}
		if (check_maximum_length_) {
			bad_length = bad_length or length > maximum_length_;
			max_str = std::to_string(maximum_length_);
		}
		if (bad_length) {
			throw LengthValidationFailure(get_name(), std::to_string(length),
					min_str, max_str);
		}

		for (const auto& element : config_node) {
			SyntaxValidator e(element);
			element_node_->accept(e);
		}
	} else {
		throw TypeValidationFailure(get_name());
	}
}

void SelectorSchemaNode::accept(SyntaxValidator& v) {
	const YAML::Node& config_node = v.get_config_node();

	std::string error_messages = "";

	for (const auto& option : option_nodes_) {
		const std::string name = option.first.first;
		const std::string type = option.first.second;

		try {
			SyntaxValidator o(config_node);
			option.second->accept(o);
			return;
		} catch (const std::logic_error& e) {
			error_messages += "\n- option (name: " + name + ", type: " + type
					+ "): " + e.what() + "\n";
		}
	}
	throw SelectorValidationFailure(get_name(), error_messages);
}

void StringSchemaNode::accept(SyntaxValidator& v) {
	using MyType = std::string;
	MyType value;

	// check that yaml-cpp can cast to the right type
	try {
		value = v.get_config_node().as<MyType>();
	} catch (...) {
		throw TypeCastValidationFailure(get_name(), "std::string");
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			throw InvalidScalarValueValidationFailure(get_name(),
					v.get_config_node().as<std::string>(),
					valid_values_string_);
		}
	}
}

void DoubleSchemaNode::accept(SyntaxValidator& v) {
	using MyType = double;
	MyType value;

	// check that yaml-cpp can cast to the right type
	try {
		value = v.get_config_node().as<MyType>();
	} catch (...) {
		throw TypeCastValidationFailure(get_name(), "double");
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			throw InvalidScalarValueValidationFailure(get_name(),
					v.get_config_node().as<std::string>(),
					valid_values_string_);
		}
	}
}

void FloatSchemaNode::accept(SyntaxValidator& v) {
	using MyType = float;
	MyType value;

	// check that yaml-cpp can cast to the right type
	try {
		value = v.get_config_node().as<MyType>();
	} catch (...) {
		throw TypeCastValidationFailure(get_name(), "float");
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			throw InvalidScalarValueValidationFailure(get_name(),
					v.get_config_node().as<std::string>(),
					valid_values_string_);
		}
	}
}

void BoolSchemaNode::accept(SyntaxValidator& v) {
	using MyType = bool;
	MyType value;

	// check that yaml-cpp can cast to the right type
	try {
		value = v.get_config_node().as<MyType>();
	} catch (...) {
		throw TypeCastValidationFailure(get_name(), "bool");
	}

	// check that the provided value is valid
	const std::string string_value = v.get_config_node().as<std::string>();
	if (std::find(valid_strings_.begin(), valid_strings_.end(), string_value)
			== valid_strings_.end()) {
		throw InvalidScalarValueValidationFailure(get_name(),
				v.get_config_node().as<std::string>(), valid_values_string_);
	}
}

void IntegerSchemaNode::accept(SyntaxValidator& v) {
	using MyType = int;
	MyType value;

	// check that yaml-cpp can cast to the right type
	try {
		value = v.get_config_node().as<MyType>();
	} catch (...) {
		throw TypeCastValidationFailure(get_name(), "int");
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			throw InvalidScalarValueValidationFailure(get_name(),
					v.get_config_node().as<std::string>(),
					valid_values_string_);
		}
	}
}

void UnsignedIntegerSchemaNode::accept(SyntaxValidator& v) {
	using MyType = unsigned int;
	MyType value;

	// check that yaml-cpp can cast to the right type
	try {
		value = v.get_config_node().as<MyType>();
	} catch (...) {
		throw TypeCastValidationFailure(get_name(), "unsigned int");
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			throw InvalidScalarValueValidationFailure(get_name(),
					v.get_config_node().as<std::string>(),
					valid_values_string_);
		}
	}
}

}
