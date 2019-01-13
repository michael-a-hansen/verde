#include "verde.hpp"

namespace verde {

ParserHelper::FrozenSchemaFailure::FrozenSchemaFailure(const std::string& type) :
		std::logic_error(
				"verde parser helper failure: cannot add type \"" + type
						+ "\" to schema as it has been frozen already. "
						+ "Either freeze_schema() has been explicitly called or"
								"the schema has been used and implicitly frozen already.") {
}

ParserHelper::TypeAdditionFailure::TypeAdditionFailure(const std::string& type,
		const std::string& types_string) :
		std::logic_error(
				"verde parser helper failure: cannot add type \"" + type
						+ "\" to schema as it has already exists.\n  - types already added: "
						+ types_string + '\n') {
}

ParserHelper::InvalidTagFailure::InvalidTagFailure(const std::string& tag,
		const std::string& tags_string) :
		std::logic_error(
				"verde schema construction failure: invalid tag \"" + tag
						+ "\" identified in the schema.\n  - valid tags: "
						+ tags_string + '\n') {
}

ParserHelper::InvalidTypeFailure::InvalidTypeFailure(const std::string& type,
		const std::string& types_string) :
		std::logic_error(
				"verde schema construction failure: invalid type \"" + type
						+ "\" identified in the schema.\n  - valid types: "
						+ types_string + '\n') {
}

InvalidSchemaNodeFailure::InvalidSchemaNodeFailure(const std::string& name,
		const std::string& type, std::string& failure_msg) :
		std::logic_error(
				"verde schema construction failure: problem constructing node \""
						+ name + "\" of type \"" + type + "\". Description: "
						+ failure_msg + '\n') {
}

TypeCastValidationFailure::TypeCastValidationFailure(const std::string& name,
		const std::string& type) :
		std::logic_error(
				"verde syntax validation failure: unable to cast \"" + name
						+ "\" node to type: \"" + type + "\"") {
}

InvalidScalarValueValidationFailure::InvalidScalarValueValidationFailure(
		const std::string& name, const std::string& string_value,
		const std::string& valid_values_string) :
		std::logic_error(
				"verde syntax validation failure: node \"" + name
						+ "\" given invalid value: \"" + string_value
						+ "\"\n  - valid values: " + valid_values_string) {
}

MapSchemaNode::TypeValidationFailure::TypeValidationFailure(
		const std::string& name) :
		std::logic_error(
				"verde syntax validation failure: map node \"" + name
						+ "\" was not given a map") {
}

MapSchemaNode::MissingRequiredKeyFailure::MissingRequiredKeyFailure(
		const std::string& name, const std::string& missing_key) :
		std::logic_error(
				"verde syntax validation failure: required key \"" + missing_key
						+ "\" was not given in map node \"" + name + "\"") {
}

MapSchemaNode::InvalidKeyFailure::InvalidKeyFailure(const std::string& name,
		const std::string& config_key, const std::string& required_keys_string,
		const std::string& optional_keys_string) :
		std::logic_error(
				"verde syntax validation failure: key \"" + config_key
						+ "\" given in map node \"" + name
						+ "\" is not valid.\n  - " + required_keys_string
						+ "\n  - " + optional_keys_string) {
}

MapSchemaNode::EmptyMapFailure::EmptyMapFailure(const std::string& name) :
		std::logic_error(
				"verde syntax validation failure: map \"" + name
						+ "\" is defined in the schema without required or optional entries.") {
}

VectorSchemaNode::TypeValidationFailure::TypeValidationFailure(
		const std::string& name) :
		std::logic_error(
				"verde syntax validation failure: vector node \"" + name
						+ "\" was not given a list") {
}

VectorSchemaNode::LengthValidationFailure::LengthValidationFailure(
		const std::string& name, const std::string& length_str,
		const std::string& min_str, const std::string& max_str) :
		std::logic_error(
				"verde syntax validation failure: vector node \"" + name
						+ "\" has invalid number of elements: " + length_str
						+ '\n' + "  - minimum length: " + min_str + '\n'
						+ "  - maximum length: " + max_str + '\n') {
}

SelectorSchemaNode::SelectorValidationFailure::SelectorValidationFailure(
		const std::string& name, const std::string& error_messages) :
		std::logic_error(
				"verde syntax validation failure: selector node \"" + name
						+ "\" failed to identify any valid options. Errors:\n"
						+ error_messages) {
}

SelectorSchemaNode::MissingOptionsFailure::MissingOptionsFailure(
		const std::string& name) :
		std::logic_error(
				"verde syntax validation failure: selector \"" + name
						+ "\" is defined in the schema with no options given.") {
}

}

