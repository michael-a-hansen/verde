#include "verde.hpp"

namespace verde {

ParserHelper::ParserHelper(const std::string& schema_file_name) :
		schema_file_(YAML::LoadFile(schema_file_name)) {
	add_type("map", std::make_shared<MapSchemaNode::Builder>(*this));
	add_type("vector", std::make_shared<VectorSchemaNode::Builder>(*this));
	add_type("selector", std::make_shared<SelectorSchemaNode::Builder>(*this));
	add_type("string", std::make_shared<StringSchemaNode::Builder>(*this));
	add_type("double", std::make_shared<DoubleSchemaNode::Builder>(*this));
	add_type("float", std::make_shared<FloatSchemaNode::Builder>(*this));
	add_type("bool", std::make_shared<BoolSchemaNode::Builder>(*this));
	add_type("integer", std::make_shared<IntegerSchemaNode::Builder>(*this));
	add_type("unsigned-integer",
			std::make_shared<IntegerSchemaNode::Builder>(*this));
}

void ParserHelper::add_type(const std::string& type,
		std::shared_ptr<SchemaNodeBase::Builder> builder) {
	if (!schema_is_set_) {
		if (builders_.find(type) == builders_.end()) {
			builders_[type] = builder;
		} else {
			std::string types = "";
			for (const auto& type_builder : builders_) {
				types += type_builder.first + ", ";
			}
			throw TypeAdditionFailure(type, types);
		}
	} else {
		throw FrozenSchemaFailure(type);
	}
}

void ParserHelper::finalize_and_build_schema() {
	const YAML::Node& tags_node = schema_file_["tags"];
	for (const YAML::Node& tag_node : tags_node) {
		const std::string key = tag_node["name"].as<std::string>();
		tags_[key] = tag_node;
	}
	schema_ = build_node(schema_file_["schema"]);
	schema_is_set_ = true;
}

void ParserHelper::freeze_schema() {
	finalize_and_build_schema();
}

bool ParserHelper::validate_configuration_file(
		const std::string& config_file_name) {
	if (!schema_is_set_) {
		freeze_schema();
	}

	const YAML::Node config_file_node = YAML::LoadFile(config_file_name);

	try {
		SyntaxValidator v(config_file_node);
		v.visit(*schema_);
		return true;
	} catch (const std::logic_error& e) {
		throw e;
		return false;
	}
}

std::shared_ptr<SchemaNodeBase> ParserHelper::build_node(
		const YAML::Node& yaml_node) const {
	const std::string node_type = yaml_node["type"].as<std::string>();

	if (node_type == "tag") {
		const std::string tag_name = yaml_node["tag"].as<std::string>();
		if(tags_.find(tag_name) != tags_.end()){

			const YAML::Node& tag_node = tags_.at(tag_name);
			const std::string tag_type = tag_node["type"].as<std::string>();

			YAML::Node renamed_tag_node(tag_node);
			renamed_tag_node["name"] = yaml_node["name"].as<std::string>();

			if (builders_.find(tag_type) == builders_.end()) {
				std::string types_string = "";
				for(const auto& name_node : builders_){
					types_string += name_node.first + ", ";
				}
				throw InvalidTagFailure(tag_type, types_string);
			} else {
				return builders_.at(tag_type)->build(renamed_tag_node);
			}
		}
		else{
			std::string tags_string = "";
			for(const auto& name_node : tags_){
				tags_string += name_node.first + ", ";
			}
			throw InvalidTagFailure(tag_name, tags_string);
		}
	} else {

		if (builders_.find(node_type) == builders_.end()) {
			std::string types_string = "";
			for(const auto& name_node : builders_){
				types_string += name_node.first + ", ";
			}
			throw InvalidTagFailure(node_type, types_string);
		} else {
			return builders_.at(node_type)->build(yaml_node);
		}
	}
}

}
