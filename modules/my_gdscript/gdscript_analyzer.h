/**************************************************************************/
/*  my_gdscript_analyzer.h                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "my_gdscript_cache.h"
#include "my_gdscript_parser.h"

#include "core/object/object.h"
#include "core/object/ref_counted.h"

class MyGDScriptAnalyzer {
	MyGDScriptParser *parser = nullptr;

	template <typename Fn>
	class Finally {
		Fn fn;

	public:
		Finally(Fn p_fn) :
				fn(p_fn) {}
		~Finally() {
			fn();
		}
	};

	const MyGDScriptParser::EnumNode *current_enum = nullptr;
	MyGDScriptParser::LambdaNode *current_lambda = nullptr;
	List<MyGDScriptParser::LambdaNode *> pending_body_resolution_lambdas;
	HashMap<const MyGDScriptParser::ClassNode *, Ref<MyGDScriptParserRef>> external_class_parser_cache;
	bool static_context = false;

	// Tests for detecting invalid overloading of script members
	static _FORCE_INLINE_ bool has_member_name_conflict_in_script_class(const StringName &p_name, const MyGDScriptParser::ClassNode *p_current_class_node, const MyGDScriptParser::Node *p_member);
	static _FORCE_INLINE_ bool has_member_name_conflict_in_native_type(const StringName &p_name, const StringName &p_native_type_string);
	Error check_native_member_name_conflict(const StringName &p_member_name, const MyGDScriptParser::Node *p_member_node, const StringName &p_native_type_string);
	Error check_class_member_name_conflict(const MyGDScriptParser::ClassNode *p_class_node, const StringName &p_member_name, const MyGDScriptParser::Node *p_member_node);

	void get_class_node_current_scope_classes(MyGDScriptParser::ClassNode *p_node, List<MyGDScriptParser::ClassNode *> *p_list, MyGDScriptParser::Node *p_source);

	Error resolve_class_inheritance(MyGDScriptParser::ClassNode *p_class, const MyGDScriptParser::Node *p_source = nullptr);
	Error resolve_class_inheritance(MyGDScriptParser::ClassNode *p_class, bool p_recursive);
	MyGDScriptParser::DataType resolve_datatype(MyGDScriptParser::TypeNode *p_type);

	void decide_suite_type(MyGDScriptParser::Node *p_suite, MyGDScriptParser::Node *p_statement);

	void resolve_annotation(MyGDScriptParser::AnnotationNode *p_annotation);
	void resolve_class_member(MyGDScriptParser::ClassNode *p_class, const StringName &p_name, const MyGDScriptParser::Node *p_source = nullptr);
	void resolve_class_member(MyGDScriptParser::ClassNode *p_class, int p_index, const MyGDScriptParser::Node *p_source = nullptr);
	void resolve_class_interface(MyGDScriptParser::ClassNode *p_class, const MyGDScriptParser::Node *p_source = nullptr);
	void resolve_class_interface(MyGDScriptParser::ClassNode *p_class, bool p_recursive);
	void resolve_class_body(MyGDScriptParser::ClassNode *p_class, const MyGDScriptParser::Node *p_source = nullptr);
	void resolve_class_body(MyGDScriptParser::ClassNode *p_class, bool p_recursive);
	void resolve_function_signature(MyGDScriptParser::FunctionNode *p_function, const MyGDScriptParser::Node *p_source = nullptr, bool p_is_lambda = false);
	void resolve_function_body(MyGDScriptParser::FunctionNode *p_function, bool p_is_lambda = false);
	void resolve_node(MyGDScriptParser::Node *p_node, bool p_is_root = true);
	void resolve_suite(MyGDScriptParser::SuiteNode *p_suite);
	void resolve_assignable(MyGDScriptParser::AssignableNode *p_assignable, const char *p_kind);
	void resolve_variable(MyGDScriptParser::VariableNode *p_variable, bool p_is_local);
	void resolve_constant(MyGDScriptParser::ConstantNode *p_constant, bool p_is_local);
	void resolve_parameter(MyGDScriptParser::ParameterNode *p_parameter);
	void resolve_if(MyGDScriptParser::IfNode *p_if);
	void resolve_for(MyGDScriptParser::ForNode *p_for);
	void resolve_while(MyGDScriptParser::WhileNode *p_while);
	void resolve_assert(MyGDScriptParser::AssertNode *p_assert);
	void resolve_match(MyGDScriptParser::MatchNode *p_match);
	void resolve_match_branch(MyGDScriptParser::MatchBranchNode *p_match_branch, MyGDScriptParser::ExpressionNode *p_match_test);
	void resolve_match_pattern(MyGDScriptParser::PatternNode *p_match_pattern, MyGDScriptParser::ExpressionNode *p_match_test);
	void resolve_return(MyGDScriptParser::ReturnNode *p_return);

	// Reduction functions.
	void reduce_expression(MyGDScriptParser::ExpressionNode *p_expression, bool p_is_root = false);
	void reduce_array(MyGDScriptParser::ArrayNode *p_array);
	void reduce_assignment(MyGDScriptParser::AssignmentNode *p_assignment);
	void reduce_await(MyGDScriptParser::AwaitNode *p_await);
	void reduce_binary_op(MyGDScriptParser::BinaryOpNode *p_binary_op);
	void reduce_call(MyGDScriptParser::CallNode *p_call, bool p_is_await = false, bool p_is_root = false);
	void reduce_cast(MyGDScriptParser::CastNode *p_cast);
	void reduce_dictionary(MyGDScriptParser::DictionaryNode *p_dictionary);
	void reduce_get_node(MyGDScriptParser::GetNodeNode *p_get_node);
	void reduce_identifier(MyGDScriptParser::IdentifierNode *p_identifier, bool can_be_builtin = false);
	void reduce_identifier_from_base(MyGDScriptParser::IdentifierNode *p_identifier, MyGDScriptParser::DataType *p_base = nullptr);
	void reduce_lambda(MyGDScriptParser::LambdaNode *p_lambda);
	void reduce_literal(MyGDScriptParser::LiteralNode *p_literal);
	void reduce_preload(MyGDScriptParser::PreloadNode *p_preload);
	void reduce_self(MyGDScriptParser::SelfNode *p_self);
	void reduce_subscript(MyGDScriptParser::SubscriptNode *p_subscript, bool p_can_be_pseudo_type = false);
	void reduce_ternary_op(MyGDScriptParser::TernaryOpNode *p_ternary_op, bool p_is_root = false);
	void reduce_type_test(MyGDScriptParser::TypeTestNode *p_type_test);
	void reduce_unary_op(MyGDScriptParser::UnaryOpNode *p_unary_op);

	Variant make_expression_reduced_value(MyGDScriptParser::ExpressionNode *p_expression, bool &is_reduced);
	Variant make_array_reduced_value(MyGDScriptParser::ArrayNode *p_array, bool &is_reduced);
	Variant make_dictionary_reduced_value(MyGDScriptParser::DictionaryNode *p_dictionary, bool &is_reduced);
	Variant make_subscript_reduced_value(MyGDScriptParser::SubscriptNode *p_subscript, bool &is_reduced);
	Variant make_call_reduced_value(MyGDScriptParser::CallNode *p_call, bool &is_reduced);

	// Helpers.
	Array make_array_from_element_datatype(const MyGDScriptParser::DataType &p_element_datatype, const MyGDScriptParser::Node *p_source_node = nullptr);
	Dictionary make_dictionary_from_element_datatype(const MyGDScriptParser::DataType &p_key_element_datatype, const MyGDScriptParser::DataType &p_value_element_datatype, const MyGDScriptParser::Node *p_source_node = nullptr);
	MyGDScriptParser::DataType type_from_variant(const Variant &p_value, const MyGDScriptParser::Node *p_source);
	MyGDScriptParser::DataType type_from_property(const PropertyInfo &p_property, bool p_is_arg = false, bool p_is_readonly = false) const;
	MyGDScriptParser::DataType make_global_class_meta_type(const StringName &p_class_name, const MyGDScriptParser::Node *p_source);
	bool get_function_signature(MyGDScriptParser::Node *p_source, bool p_is_constructor, MyGDScriptParser::DataType base_type, const StringName &p_function, MyGDScriptParser::DataType &r_return_type, List<MyGDScriptParser::DataType> &r_par_types, int &r_default_arg_count, BitField<MethodFlags> &r_method_flags, StringName *r_native_class = nullptr);
	bool function_signature_from_info(const MethodInfo &p_info, MyGDScriptParser::DataType &r_return_type, List<MyGDScriptParser::DataType> &r_par_types, int &r_default_arg_count, BitField<MethodFlags> &r_method_flags);
	void validate_call_arg(const List<MyGDScriptParser::DataType> &p_par_types, int p_default_args_count, bool p_is_vararg, const MyGDScriptParser::CallNode *p_call);
	void validate_call_arg(const MethodInfo &p_method, const MyGDScriptParser::CallNode *p_call);
	MyGDScriptParser::DataType get_operation_type(Variant::Operator p_operation, const MyGDScriptParser::DataType &p_a, const MyGDScriptParser::DataType &p_b, bool &r_valid, const MyGDScriptParser::Node *p_source);
	MyGDScriptParser::DataType get_operation_type(Variant::Operator p_operation, const MyGDScriptParser::DataType &p_a, bool &r_valid, const MyGDScriptParser::Node *p_source);
	void update_const_expression_builtin_type(MyGDScriptParser::ExpressionNode *p_expression, const MyGDScriptParser::DataType &p_type, const char *p_usage, bool p_is_cast = false);
	void update_array_literal_element_type(MyGDScriptParser::ArrayNode *p_array, const MyGDScriptParser::DataType &p_element_type);
	void update_dictionary_literal_element_type(MyGDScriptParser::DictionaryNode *p_dictionary, const MyGDScriptParser::DataType &p_key_element_type, const MyGDScriptParser::DataType &p_value_element_type);
	bool is_type_compatible(const MyGDScriptParser::DataType &p_target, const MyGDScriptParser::DataType &p_source, bool p_allow_implicit_conversion = false, const MyGDScriptParser::Node *p_source_node = nullptr);
	void push_error(const String &p_message, const MyGDScriptParser::Node *p_origin = nullptr);
	void mark_node_unsafe(const MyGDScriptParser::Node *p_node);
	void downgrade_node_type_source(MyGDScriptParser::Node *p_node);
	void mark_lambda_use_self();
	void resolve_pending_lambda_bodies();
	bool class_exists(const StringName &p_class) const;
	void reduce_identifier_from_base_set_class(MyGDScriptParser::IdentifierNode *p_identifier, MyGDScriptParser::DataType p_identifier_datatype);
	Ref<MyGDScriptParserRef> ensure_cached_external_parser_for_class(const MyGDScriptParser::ClassNode *p_class, const MyGDScriptParser::ClassNode *p_from_class, const char *p_context, const MyGDScriptParser::Node *p_source);
	Ref<MyGDScriptParserRef> find_cached_external_parser_for_class(const MyGDScriptParser::ClassNode *p_class, const Ref<MyGDScriptParserRef> &p_dependant_parser);
	Ref<MyGDScriptParserRef> find_cached_external_parser_for_class(const MyGDScriptParser::ClassNode *p_class, MyGDScriptParser *p_dependant_parser);
	Ref<MyGDScript> get_depended_shallow_script(const String &p_path, Error &r_error);
#ifdef DEBUG_ENABLED
	void is_shadowing(MyGDScriptParser::IdentifierNode *p_identifier, const String &p_context, const bool p_in_local_scope);
#endif

public:
	Error resolve_inheritance();
	Error resolve_interface();
	Error resolve_body();
	Error resolve_dependencies();
	Error analyze();

	Variant make_variable_default_value(MyGDScriptParser::VariableNode *p_variable);

	static bool check_type_compatibility(const MyGDScriptParser::DataType &p_target, const MyGDScriptParser::DataType &p_source, bool p_allow_implicit_conversion = false, const MyGDScriptParser::Node *p_source_node = nullptr);
	static MyGDScriptParser::DataType type_from_metatype(const MyGDScriptParser::DataType &p_meta_type);

	MyGDScriptAnalyzer(MyGDScriptParser *p_parser);
};
