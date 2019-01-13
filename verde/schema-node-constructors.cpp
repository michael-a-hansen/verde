#include "verde.hpp"
#include <sstream>
#include <iomanip>
#include <limits>

namespace verde {

SchemaNodeBase::SchemaNodeBase(const ParserHelper& node_factory,
		const YAML::Node& yaml_node) :
		node_factory_(node_factory), name_(yaml_node["name"].as<std::string>()), type_(
				yaml_node["type"].as<std::string>()) {
	if (yaml_node["description"]) {
		description_ = yaml_node["description"].as<std::string>();
	}
}

SchemaNodeBase::Builder::Builder(const ParserHelper& node_factory) :
		node_factory_(node_factory) {
}

void check_schema_node_keys_validity(const std::vector<std::string>& valid_keys,
		const YAML::Node& yaml_node, const std::string& name,
		const std::string& type) {
	for (const auto& keyval : yaml_node) {
		const std::string& key = keyval.first.as<std::string>();
		if (std::find(valid_keys.begin(), valid_keys.end(), key)
				== valid_keys.end()) {
			std::string msg = "Invalid key \"" + key
					+ "\" identified.\n  - valid keys: ";
			for (const auto& vk : valid_keys) {
				msg += vk + ", ";
			}
			throw InvalidSchemaNodeFailure(name, type, msg);
		}
	}
}

MapSchemaNode::MapSchemaNode(const ParserHelper& node_factory,
		const YAML::Node& yaml_node) :
		SchemaNodeBase(node_factory, yaml_node) {

	std::vector<std::string> valid_keys = { "name", "type", "description",
			"required-entries", "optional-entries" };
	check_schema_node_keys_validity(valid_keys, yaml_node, get_name(),
			get_type());

	has_required_ = yaml_node["required-entries"];
	has_optional_ = yaml_node["optional-entries"];

	if (has_required_) {
		for (const YAML::Node& node : yaml_node["required-entries"]) {
			const std::string key = node["name"].as<std::string>();
			required_nodes_[key] = node_factory_.build_node(node);
			required_nodes_string_ += key + ", ";
		}
	}

	if (has_optional_) {
		for (const YAML::Node& node : yaml_node["optional-entries"]) {
			const std::string key = node["name"].as<std::string>();
			optional_nodes_[key] = node_factory_.build_node(node);
			optional_nodes_string_ += key + ", ";
		}
	}

	if (not has_optional_ and not has_required_) {
		throw EmptyMapFailure(get_name());
	}
}

MapSchemaNode::Builder::Builder(const ParserHelper& node_factory) :
		SchemaNodeBase::Builder(node_factory) {
}

std::shared_ptr<SchemaNodeBase> MapSchemaNode::Builder::build(
		const YAML::Node& yaml_node) const {
	return std::make_shared<MapSchemaNode>(node_factory_, yaml_node);
}

VectorSchemaNode::VectorSchemaNode(const ParserHelper& node_factory,
		const YAML::Node& yaml_node) :
		SchemaNodeBase(node_factory, yaml_node) {
	std::vector<std::string> valid_keys = { "name", "type", "description",
			"elements", "minimum-length", "maximum-length" };
	check_schema_node_keys_validity(valid_keys, yaml_node, get_name(),
			get_type());

	element_node_ = node_factory_.build_node(yaml_node["elements"]);

	if(yaml_node["minimum-length"]){
		minimum_length_ = yaml_node["minimum-length"].as<unsigned int>();
		check_minimum_length_ = true;
	}

	if(yaml_node["maximum-length"]){
		maximum_length_ = yaml_node["maximum-length"].as<unsigned int>();
		check_maximum_length_ = true;
	}
}

VectorSchemaNode::Builder::Builder(const ParserHelper& node_factory) :
		SchemaNodeBase::Builder(node_factory) {
}

std::shared_ptr<SchemaNodeBase> VectorSchemaNode::Builder::build(
		const YAML::Node& yaml_node) const {
	return std::make_shared<VectorSchemaNode>(node_factory_, yaml_node);
}

SelectorSchemaNode::SelectorSchemaNode(const ParserHelper& node_factory,
		const YAML::Node& yaml_node) :
		SchemaNodeBase(node_factory, yaml_node) {
	std::vector<std::string> valid_keys = { "name", "type", "description",
			"options" };
	check_schema_node_keys_validity(valid_keys, yaml_node, get_name(),
			get_type());

	if (yaml_node["options"]) {
		for (const YAML::Node& option_node : yaml_node["options"]) {
			const std::string option_name =
					option_node["name"].as<std::string>();
			const std::string option_type =
					option_node["type"].as<std::string>();

			const auto pair = std::make_pair(option_name, option_type);
			option_nodes_[pair] = node_factory_.build_node(option_node);
		}
	} else {
		throw MissingOptionsFailure(get_name());
	}
}

SelectorSchemaNode::Builder::Builder(const ParserHelper& node_factory) :
		SchemaNodeBase::Builder(node_factory) {
}

std::shared_ptr<SchemaNodeBase> SelectorSchemaNode::Builder::build(
		const YAML::Node& yaml_node) const {
	return std::make_shared<SelectorSchemaNode>(node_factory_, yaml_node);
}

template<typename T>
std::string full_precision_string(const T& value) {
	std::stringstream stream;
	stream << std::scientific
			<< std::setprecision(std::numeric_limits<T>::digits10 + 1) << value;
	return stream.str();
}

StringSchemaNode::StringSchemaNode(const ParserHelper& node_factory,
		const YAML::Node& yaml_node) :
		SchemaNodeBase(node_factory, yaml_node) {
	std::vector<std::string> valid_keys = { "name", "type", "description",
			"default", "values" };
	check_schema_node_keys_validity(valid_keys, yaml_node, get_name(),
			get_type());
	if (yaml_node["default"]) {
		has_default_ = true;
		default_ = yaml_node["default"].as<std::string>();
	}
	if (yaml_node["values"]) {
		has_valid_values_ = true;
		for (const auto& value_node : yaml_node["values"]) {
			const std::string& value_node_str = value_node.as<std::string>();
			valid_values_string_ += value_node_str + ", ";
			valid_values_.push_back(value_node_str);
		}
	}
}

StringSchemaNode::Builder::Builder(const ParserHelper& node_factory) :
		SchemaNodeBase::Builder(node_factory) {
}

std::shared_ptr<SchemaNodeBase> StringSchemaNode::Builder::build(
		const YAML::Node& yaml_node) const {
	return std::make_shared<StringSchemaNode>(node_factory_, yaml_node);
}

DoubleSchemaNode::DoubleSchemaNode(const ParserHelper& node_factory,
		const YAML::Node& yaml_node) :
		SchemaNodeBase(node_factory, yaml_node) {
	std::vector<std::string> valid_keys = { "name", "type", "description",
			"default", "values" };
	check_schema_node_keys_validity(valid_keys, yaml_node, get_name(),
			get_type());
	if (yaml_node["default"]) {
		has_default_ = true;
		default_ = yaml_node["default"].as<double>();
	}
	if (yaml_node["values"]) {
		has_valid_values_ = true;
		for (const auto& value_node : yaml_node["values"]) {
			const std::string& value_node_str = value_node.as<std::string>();
			valid_values_string_ += value_node_str + ", ";
			valid_values_.push_back(value_node.as<double>());
		}
	}
}

DoubleSchemaNode::Builder::Builder(const ParserHelper& node_factory) :
		SchemaNodeBase::Builder(node_factory) {
}

std::shared_ptr<SchemaNodeBase> DoubleSchemaNode::Builder::build(
		const YAML::Node& yaml_node) const {
	return std::make_shared<DoubleSchemaNode>(node_factory_, yaml_node);
}

FloatSchemaNode::FloatSchemaNode(const ParserHelper& node_factory,
		const YAML::Node& yaml_node) :
		SchemaNodeBase(node_factory, yaml_node) {
	std::vector<std::string> valid_keys = { "name", "type", "description",
			"default", "values" };
	check_schema_node_keys_validity(valid_keys, yaml_node, get_name(),
			get_type());
	if (yaml_node["default"]) {
		has_default_ = true;
		default_ = yaml_node["default"].as<float>();
	}
	if (yaml_node["values"]) {
		has_valid_values_ = true;
		for (const auto& value_node : yaml_node["values"]) {
			const std::string& value_node_str = value_node.as<std::string>();
			valid_values_string_ += value_node_str + ", ";
			valid_values_.push_back(value_node.as<float>());
		}
	}
}

FloatSchemaNode::Builder::Builder(const ParserHelper& node_factory) :
		SchemaNodeBase::Builder(node_factory) {
}

std::shared_ptr<SchemaNodeBase> FloatSchemaNode::Builder::build(
		const YAML::Node& yaml_node) const {
	return std::make_shared<FloatSchemaNode>(node_factory_, yaml_node);
}

BoolSchemaNode::BoolSchemaNode(const ParserHelper& node_factory,
		const YAML::Node& yaml_node) :
		SchemaNodeBase(node_factory, yaml_node) {
	std::vector<std::string> valid_keys = { "name", "type", "description",
			"default" };
	check_schema_node_keys_validity(valid_keys, yaml_node, get_name(),
			get_type());
	if (yaml_node["default"]) {
		has_default_ = true;
		default_ = yaml_node["default"].as<float>();
	}
}

BoolSchemaNode::Builder::Builder(const ParserHelper& node_factory) :
		SchemaNodeBase::Builder(node_factory) {
}

std::shared_ptr<SchemaNodeBase> BoolSchemaNode::Builder::build(
		const YAML::Node& yaml_node) const {
	return std::make_shared<BoolSchemaNode>(node_factory_, yaml_node);
}

IntegerSchemaNode::IntegerSchemaNode(const ParserHelper& node_factory,
		const YAML::Node& yaml_node) :
		SchemaNodeBase(node_factory, yaml_node) {
	std::vector<std::string> valid_keys = { "name", "type", "description",
			"default", "values" };
	check_schema_node_keys_validity(valid_keys, yaml_node, get_name(),
			get_type());
	if (yaml_node["default"]) {
		has_default_ = true;
		default_ = yaml_node["default"].as<int>();
	}
	if (yaml_node["values"]) {
		has_valid_values_ = true;
		for (const auto& value_node : yaml_node["values"]) {
			const std::string& value_node_str = value_node.as<std::string>();
			valid_values_string_ += value_node_str + ", ";
			valid_values_.push_back(value_node.as<int>());
		}
	}
}

IntegerSchemaNode::Builder::Builder(const ParserHelper& node_factory) :
		SchemaNodeBase::Builder(node_factory) {
}

std::shared_ptr<SchemaNodeBase> IntegerSchemaNode::Builder::build(
		const YAML::Node& yaml_node) const {
	return std::make_shared<IntegerSchemaNode>(node_factory_, yaml_node);
}

UnsignedIntegerSchemaNode::UnsignedIntegerSchemaNode(
		const ParserHelper& node_factory, const YAML::Node& yaml_node) :
		SchemaNodeBase(node_factory, yaml_node) {
	std::vector<std::string> valid_keys = { "name", "type", "description",
			"default", "values" };
	check_schema_node_keys_validity(valid_keys, yaml_node, get_name(),
			get_type());
	if (yaml_node["default"]) {
		has_default_ = true;
		default_ = yaml_node["default"].as<unsigned int>();
	}
	if (yaml_node["values"]) {
		has_valid_values_ = true;
		for (const auto& value_node : yaml_node["values"]) {
			const std::string& value_node_str = value_node.as<std::string>();
			valid_values_string_ += value_node_str + ", ";
			valid_values_.push_back(value_node.as<unsigned int>());
		}
	}
}

UnsignedIntegerSchemaNode::Builder::Builder(const ParserHelper& node_factory) :
		SchemaNodeBase::Builder(node_factory) {
}

std::shared_ptr<SchemaNodeBase> UnsignedIntegerSchemaNode::Builder::build(
		const YAML::Node& yaml_node) const {
	return std::make_shared<UnsignedIntegerSchemaNode>(node_factory_, yaml_node);
}

}
