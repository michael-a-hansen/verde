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

#ifndef VERDE_INCLUDE_VERDE_HPP_
#define VERDE_INCLUDE_VERDE_HPP_

#include "yaml-cpp/yaml.h"
#include <string>
#include <vector>
#include <map>

namespace verde {

class ParserHelper;

class SchemaNodeBase;

class SchemaTraverserBase {
public:
	virtual ~SchemaTraverserBase() {
	}
	virtual void visit(SchemaNodeBase&)=0;
};

class SyntaxValidator;

class SchemaNodeBase {
protected:
	const ParserHelper& node_factory_;
	const std::string name_;
	const std::string type_;
	std::string description_;
	std::vector<std::pair<std::string, std::string> > parent_names_and_types_;

public:
	SchemaNodeBase(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	virtual ~SchemaNodeBase() {
	}

	virtual bool accept(SyntaxValidator&)=0;

	inline const std::string get_name() const {
		return name_;
	}

	inline const std::string get_type() const {
		return type_;
	}

	class Builder {
	protected:
		const ParserHelper& node_factory_;
	public:
		Builder(const ParserHelper& node_factory);
		virtual std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const =0;
		virtual ~Builder() {
		}
	};
};

class SyntaxValidator: public SchemaTraverserBase {
protected:
	const YAML::Node& config_node_;
	const bool throw_on_fail_;
	std::string error_message_;
public:
	SyntaxValidator(const YAML::Node& config_node, const bool throw_on_fail =
			true);

	void visit(SchemaNodeBase&) override;

	inline const YAML::Node& get_config_node() const {
		return config_node_;
	}

	inline const bool get_throw_on_fail() const {
		return throw_on_fail_;
	}

	inline bool report_error(const std::logic_error& exception) {
		if (throw_on_fail_) {
			throw exception;
		} else {
			error_message_ = exception.what();
			return false;
		}
	}

	inline void set_error_message(const std::string& error_message) {
		error_message_ = error_message;
	}

	inline std::string get_error_message() const {
		return error_message_;
	}
};

class TypeCastValidationFailure: public std::logic_error {
public:
	TypeCastValidationFailure(const std::string& name, const std::string& type);
};

class InvalidScalarValueValidationFailure: public std::logic_error {
public:
	InvalidScalarValueValidationFailure(const std::string& name,
			const std::string& string_value,
			const std::string& valid_values_string);
};

class InvalidSchemaNodeFailure: public std::logic_error {
public:
	InvalidSchemaNodeFailure(const std::string& name, const std::string& type,
			std::string& failure_msg);
};

void check_schema_node_keys_validity(const std::vector<std::string>& valid_keys,
		const YAML::Node& yaml_node, const std::string& name,
		const std::string& type);

class MapSchemaNode: public SchemaNodeBase {
protected:
	bool has_required_ = false;
	bool has_optional_ = false;
	std::map<std::string, std::shared_ptr<SchemaNodeBase> > required_nodes_;
	std::map<std::string, std::shared_ptr<SchemaNodeBase> > optional_nodes_;
	std::string required_nodes_string_ = "required nodes: ";
	std::string optional_nodes_string_ = "optional nodes: ";
public:
	MapSchemaNode(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	bool accept(SyntaxValidator&);

	class Builder: public SchemaNodeBase::Builder {
	public:
		Builder(const ParserHelper& node_factory);
		std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const;
		virtual ~Builder() {
		}
	};

	class TypeValidationFailure: public std::logic_error {
	public:
		TypeValidationFailure(const std::string& name);
	};

	class MissingRequiredKeyFailure: public std::logic_error {
	public:
		MissingRequiredKeyFailure(const std::string& name,
				const std::string& missing_key);
	};

	class InvalidKeyFailure: public std::logic_error {
	public:
		InvalidKeyFailure(const std::string& name,
				const std::string& config_key,
				const std::string& required_keys_string,
				const std::string& optional_keys_string);
	};

	class EmptyMapFailure: public std::logic_error {
	public:
		EmptyMapFailure(const std::string& name);
	};
};

class VectorSchemaNode: public SchemaNodeBase {
protected:
	std::shared_ptr<SchemaNodeBase> element_node_;
	unsigned int minimum_length_ = 0;
	unsigned int maximum_length_ = 0;
	bool check_minimum_length_ = false;
	bool check_maximum_length_ = false;
public:
	VectorSchemaNode(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	bool accept(SyntaxValidator&);

	class Builder: public SchemaNodeBase::Builder {
	public:
		Builder(const ParserHelper& node_factory);
		std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const;
		virtual ~Builder() {
		}
	};

	class TypeValidationFailure: public std::logic_error {
	public:
		TypeValidationFailure(const std::string& name);
	};

	class LengthValidationFailure: public std::logic_error {
	public:
		LengthValidationFailure(const std::string& name,
				const std::string& length_str, const std::string& min_str,
				const std::string& max_str);
	};
};

class SelectorSchemaNode: public SchemaNodeBase {
protected:
	std::map<std::pair<std::string, std::string>,
			std::shared_ptr<SchemaNodeBase> > option_nodes_;
public:
	SelectorSchemaNode(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	bool accept(SyntaxValidator&);

	class Builder: public SchemaNodeBase::Builder {
	public:
		Builder(const ParserHelper& node_factory);
		std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const;
		virtual ~Builder() {
		}
	};

	class SelectorValidationFailure: public std::logic_error {
	public:
		SelectorValidationFailure(const std::string& name,
				const std::string& error_messages);
	};

	class MissingOptionsFailure: public std::logic_error {
	public:
		MissingOptionsFailure(const std::string& name);
	};
};

class StringSchemaNode: public SchemaNodeBase {
protected:
	bool has_default_ = false;
	std::string default_;
	bool has_valid_values_ = false;
	std::vector<std::string> valid_values_;
	std::string valid_values_string_ = "";

public:
	StringSchemaNode(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	bool accept(SyntaxValidator&);

	class Builder: public SchemaNodeBase::Builder {
	public:
		Builder(const ParserHelper& node_factory);
		std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const;
		virtual ~Builder() {
		}
	};
};

class DoubleSchemaNode: public SchemaNodeBase {
protected:
	bool has_default_ = false;
	double default_;
	bool has_valid_values_ = false;
	std::vector<double> valid_values_;
	std::string valid_values_string_ = "";

public:
	DoubleSchemaNode(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	bool accept(SyntaxValidator&);

	class Builder: public SchemaNodeBase::Builder {
	public:
		Builder(const ParserHelper& node_factory);
		std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const;
		virtual ~Builder() {
		}
	};
};

class FloatSchemaNode: public SchemaNodeBase {
protected:
	bool has_default_ = false;
	float default_;
	bool has_valid_values_ = false;
	std::vector<float> valid_values_;
	std::string valid_values_string_ = "";

public:
	FloatSchemaNode(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	bool accept(SyntaxValidator&);

	class Builder: public SchemaNodeBase::Builder {
	public:
		Builder(const ParserHelper& node_factory);
		std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const;
		virtual ~Builder() {
		}
	};
};

class BoolSchemaNode: public SchemaNodeBase {
protected:
	bool has_default_ = false;
	bool default_;
	std::vector<std::string> valid_strings_ = { "true", "false" };
	std::string valid_values_string_ = "true, false, ";

public:
	BoolSchemaNode(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	bool accept(SyntaxValidator&);

	class Builder: public SchemaNodeBase::Builder {
	public:
		Builder(const ParserHelper& node_factory);
		std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const;
		virtual ~Builder() {
		}
	};
};

class IntegerSchemaNode: public SchemaNodeBase {
protected:
	bool has_default_ = false;
	int default_;
	bool has_valid_values_ = false;
	std::vector<int> valid_values_;
	std::string valid_values_string_ = "";

public:
	IntegerSchemaNode(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	bool accept(SyntaxValidator&);

	class Builder: public SchemaNodeBase::Builder {
	public:
		Builder(const ParserHelper& node_factory);
		std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const;
		virtual ~Builder() {
		}
	};
};

class UnsignedIntegerSchemaNode: public SchemaNodeBase {
protected:
	bool has_default_ = false;
	unsigned int default_;
	bool has_valid_values_ = false;
	std::vector<unsigned int> valid_values_;
	std::string valid_values_string_ = "";

public:
	UnsignedIntegerSchemaNode(const ParserHelper& node_factory,
			const YAML::Node& yaml_node);

	bool accept(SyntaxValidator&);

	class Builder: public SchemaNodeBase::Builder {
	public:
		Builder(const ParserHelper& node_factory);
		std::shared_ptr<SchemaNodeBase> build(
				const YAML::Node& yaml_node) const;
		virtual ~Builder() {
		}
	};
};

class ParserHelper {
protected:
	std::map<std::string, std::shared_ptr<SchemaNodeBase::Builder> > builders_;
	std::map<std::string, YAML::Node> tags_;
	const YAML::Node schema_file_;
	std::shared_ptr<SchemaNodeBase> schema_;
	bool schema_is_set_ = false;

	void finalize_and_build_schema();

public:
	ParserHelper(const std::string& schema_file_name);

	void freeze_schema();

	void add_type(const std::string& type,
			std::shared_ptr<SchemaNodeBase::Builder> builder);

	std::shared_ptr<SchemaNodeBase> build_node(
			const YAML::Node& yaml_node) const;

	bool validate_configuration_file(const std::string& config_file_name);

	class FrozenSchemaFailure: public std::logic_error {
	public:
		FrozenSchemaFailure(const std::string& type);
	};

	class TypeAdditionFailure: public std::logic_error {
	public:
		TypeAdditionFailure(const std::string& type,
				const std::string& types_string);
	};

	class InvalidTagFailure: public std::logic_error {
	public:
		InvalidTagFailure(const std::string& tag,
				const std::string& tags_string);
	};

	class InvalidTypeFailure: public std::logic_error {
	public:
		InvalidTypeFailure(const std::string& type,
				const std::string& types_string);
	};
};

}

#endif /* VERDE_INCLUDE_VERDE_HPP_ */
