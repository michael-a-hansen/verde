/*
 * Copyright (c) 2018-2019 Michael Alan Hansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include "verde.hpp"

namespace verde {

SyntaxValidator::SyntaxValidator(const YAML::Node& config_node,
		const bool throw_on_fail) :
		config_node_(config_node), throw_on_fail_(throw_on_fail) {
}

void SyntaxValidator::visit(SchemaNodeBase& node) {
	node.accept(*this);
}

bool MapSchemaNode::accept(SyntaxValidator& v) {
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
				return v.report_error(
						MissingRequiredKeyFailure(get_name(), key));
			}
		}

		for (const auto& key : config_keys) {
			const bool is_required = required_nodes_.find(key)
					!= required_nodes_.end();
			const bool is_optional = optional_nodes_.find(key)
					!= optional_nodes_.end();

			if (not is_required and not is_optional) {
				return v.report_error(
						InvalidKeyFailure(get_name(), key,
								required_nodes_string_, optional_nodes_string_));
			}
			if (has_required_ and not has_optional_) {
				if (not is_required) {
					return v.report_error(
							InvalidKeyFailure(get_name(), key,
									required_nodes_string_,
									optional_nodes_string_));
				}
			}
			if (not has_required_ and has_optional_) {
				if (not is_optional) {
					return v.report_error(
							InvalidKeyFailure(get_name(), key,
									required_nodes_string_,
									optional_nodes_string_));
				}
			}

			if (is_required) {
				SyntaxValidator e(config_node[key], v.get_throw_on_fail());
				if (not required_nodes_.at(key)->accept(e)) {
					v.set_error_message(e.get_error_message());
					return false;
				}
			} else {
				SyntaxValidator e(config_node[key], v.get_throw_on_fail());
				if (not optional_nodes_.at(key)->accept(e)) {
					v.set_error_message(e.get_error_message());
					return false;
				}
			}
		}
	} else {
		return v.report_error(TypeValidationFailure(get_name()));
	}
	return true;
}

bool VectorSchemaNode::accept(SyntaxValidator& v) {
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
			return v.report_error(
					LengthValidationFailure(get_name(), std::to_string(length),
							min_str, max_str));
		}

		for (const auto& element : config_node) {
			SyntaxValidator e(element, v.get_throw_on_fail());
			if (not element_node_->accept(e)) {
				v.set_error_message(e.get_error_message());
				return false;
			}
		}
	} else {
		return v.report_error(TypeValidationFailure(get_name()));
	}
	return true;
}

bool SelectorSchemaNode::accept(SyntaxValidator& v) {
	const YAML::Node& config_node = v.get_config_node();

	std::string error_messages = "";

	for (const auto& option : option_nodes_) {
		const std::string name = option.first.first;
		const std::string type = option.first.second;

		{
			SyntaxValidator local_validator(config_node, false); // this validator does not throw on failure
			const bool accepted = option.second->accept(local_validator);
			if (accepted) {
				return true;
			} else {
				error_messages += "\n- option (name: " + name + ", type: "
						+ type + "): " + local_validator.get_error_message()
						+ "\n";
			}
		}
	}
	return v.report_error(SelectorValidationFailure(get_name(), error_messages));
}

bool StringSchemaNode::accept(SyntaxValidator& v) {
	using MyType = std::string;
	MyType value;

	// check that yaml-cpp can cast to the right type
	if (not YAML::convert<MyType>::decode(v.get_config_node(), value)) {
		return v.report_error(
				TypeCastValidationFailure(get_name(), "std::string"));
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			return v.report_error(
					InvalidScalarValueValidationFailure(get_name(),
							v.get_config_node().as<std::string>(),
							valid_values_string_));
		}
	}
	return true;
}

bool DoubleSchemaNode::accept(SyntaxValidator& v) {
	using MyType = double;
	MyType value;

	// check that yaml-cpp can cast to the right type
	if (not YAML::convert<MyType>::decode(v.get_config_node(), value)) {
		return v.report_error(TypeCastValidationFailure(get_name(), "double"));
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			return v.report_error(
					InvalidScalarValueValidationFailure(get_name(),
							v.get_config_node().as<std::string>(),
							valid_values_string_));
		}
	}
	return true;
}

bool FloatSchemaNode::accept(SyntaxValidator& v) {
	using MyType = float;
	MyType value;

	// check that yaml-cpp can cast to the right type
	if (not YAML::convert<MyType>::decode(v.get_config_node(), value)) {
		return v.report_error(TypeCastValidationFailure(get_name(), "float"));
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			return v.report_error(
					InvalidScalarValueValidationFailure(get_name(),
							v.get_config_node().as<std::string>(),
							valid_values_string_));
		}
	}
	return true;
}

bool BoolSchemaNode::accept(SyntaxValidator& v) {
	using MyType = bool;
	MyType value;

	// check that yaml-cpp can cast to the right type
	if (not YAML::convert<MyType>::decode(v.get_config_node(), value)) {
		return v.report_error(TypeCastValidationFailure(get_name(), "bool"));
	}

	// check that the provided value is valid
	const std::string string_value = v.get_config_node().as<std::string>();
	if (std::find(valid_strings_.begin(), valid_strings_.end(), string_value)
			== valid_strings_.end()) {
		return v.report_error(
				InvalidScalarValueValidationFailure(get_name(),
						v.get_config_node().as<std::string>(),
						valid_values_string_));
	}
	return true;
}

bool IntegerSchemaNode::accept(SyntaxValidator& v) {
	using MyType = int;
	MyType value;

	// check that yaml-cpp can cast to the right type
	if (not YAML::convert<MyType>::decode(v.get_config_node(), value)) {
		return v.report_error(TypeCastValidationFailure(get_name(), "int"));
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			return v.report_error(
					InvalidScalarValueValidationFailure(get_name(),
							v.get_config_node().as<std::string>(),
							valid_values_string_));
		}
	}
	return true;
}

bool UnsignedIntegerSchemaNode::accept(SyntaxValidator& v) {
	using MyType = unsigned int;
	MyType value;

	// check that yaml-cpp can cast to the right type
	if (not YAML::convert<MyType>::decode(v.get_config_node(), value)) {
		return v.report_error(
				TypeCastValidationFailure(get_name(), "unsigned int"));
	}

	// check that the provided value is valid
	if (has_valid_values_) {
		if (std::find(valid_values_.begin(), valid_values_.end(), value)
				== valid_values_.end()) {
			return v.report_error(
					InvalidScalarValueValidationFailure(get_name(),
							v.get_config_node().as<std::string>(),
							valid_values_string_));
		}
	}
	return true;
}

}
