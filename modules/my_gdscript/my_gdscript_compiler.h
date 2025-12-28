/**************************************************************************/
/*  my_gdscript_compiler.h                                                   */
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

#include "my_gdscript.h"
#include "my_gdscript_codegen.h"
#include "my_gdscript_function.h"
#include "my_gdscript_parser.h"

#include "core/templates/hash_set.h"

class MyGDScriptCompiler {
	const MyGDScriptParser *parser = nullptr;
	HashSet<MyGDScript *> parsed_classes;
	HashSet<MyGDScript *> parsing_classes;
	MyGDScript *main_script = nullptr;

	struct FunctionLambdaInfo {
		MyGDScriptFunction *function = nullptr;
		MyGDScriptFunction *parent = nullptr;
		MyGDScript *script = nullptr;
		StringName name;
		int line = 0;
		int index = 0;
		int depth = 0;
		//uint64_t code_hash;
		//int code_size;
		int capture_count = 0;
		bool use_self = false;
		int arg_count = 0;
		int default_arg_count = 0;
		//Vector<MyGDScriptDataType> argument_types;
		//MyGDScriptDataType return_type;
		Vector<FunctionLambdaInfo> sublambdas;
	};

	struct ScriptLambdaInfo {
		Vector<FunctionLambdaInfo> implicit_initializer_info;
		Vector<FunctionLambdaInfo> implicit_ready_info;
		Vector<FunctionLambdaInfo> static_initializer_info;
		HashMap<StringName, Vector<FunctionLambdaInfo>> member_function_infos;
		Vector<FunctionLambdaInfo> other_function_infos;
		HashMap<StringName, ScriptLambdaInfo> subclass_info;
	};

	struct CodeGen {
		MyGDScript *script = nullptr;
		const MyGDScriptParser::ClassNode *class_node = nullptr;
		const MyGDScriptParser::FunctionNode *function_node = nullptr;
		StringName function_name;
		MyGDScriptCodeGenerator *generator = nullptr;
		HashMap<StringName, MyGDScriptCodeGenerator::Address> parameters;
		HashMap<StringName, MyGDScriptCodeGenerator::Address> locals;
		List<HashMap<StringName, MyGDScriptCodeGenerator::Address>> locals_stack;
		bool is_static = false;

		MyGDScriptCodeGenerator::Address add_local(const StringName &p_name, const MyGDScriptDataType &p_type) {
			uint32_t addr = generator->add_local(p_name, p_type);
			locals[p_name] = MyGDScriptCodeGenerator::Address(MyGDScriptCodeGenerator::Address::LOCAL_VARIABLE, addr, p_type);
			return locals[p_name];
		}

		MyGDScriptCodeGenerator::Address add_local_constant(const StringName &p_name, const Variant &p_value) {
			uint32_t addr = generator->add_local_constant(p_name, p_value);
			locals[p_name] = MyGDScriptCodeGenerator::Address(MyGDScriptCodeGenerator::Address::CONSTANT, addr);
			return locals[p_name];
		}

		MyGDScriptCodeGenerator::Address add_temporary(const MyGDScriptDataType &p_type = MyGDScriptDataType()) {
			uint32_t addr = generator->add_temporary(p_type);
			return MyGDScriptCodeGenerator::Address(MyGDScriptCodeGenerator::Address::TEMPORARY, addr, p_type);
		}

		MyGDScriptCodeGenerator::Address add_constant(const Variant &p_constant) {
			MyGDScriptDataType type;
			type.has_type = true;
			type.kind = MyGDScriptDataType::BUILTIN;
			type.builtin_type = p_constant.get_type();
			if (type.builtin_type == Variant::OBJECT) {
				Object *obj = p_constant;
				if (obj) {
					type.kind = MyGDScriptDataType::NATIVE;
					type.native_type = obj->get_class_name();

					Ref<Script> scr = obj->get_script();
					if (scr.is_valid()) {
						type.script_type = scr.ptr();
						Ref<MyGDScript> my_gdscript = scr;
						if (my_gdscript.is_valid()) {
							type.kind = MyGDScriptDataType::GDSCRIPT;
						} else {
							type.kind = MyGDScriptDataType::SCRIPT;
						}
					}
				} else {
					type.builtin_type = Variant::NIL;
				}
			}

			uint32_t addr = generator->add_or_get_constant(p_constant);
			return MyGDScriptCodeGenerator::Address(MyGDScriptCodeGenerator::Address::CONSTANT, addr, type);
		}

		void start_block() {
			HashMap<StringName, MyGDScriptCodeGenerator::Address> old_locals = locals;
			locals_stack.push_back(old_locals);
			generator->start_block();
		}

		void end_block() {
			locals = locals_stack.back()->get();
			locals_stack.pop_back();
			generator->end_block();
		}
	};

	bool _is_class_member_property(CodeGen &codegen, const StringName &p_name);
	bool _is_class_member_property(MyGDScript *owner, const StringName &p_name);
	bool _is_local_or_parameter(CodeGen &codegen, const StringName &p_name);

	void _set_error(const String &p_error, const MyGDScriptParser::Node *p_node);

	MyGDScriptDataType _gdtype_from_datatype(const MyGDScriptParser::DataType &p_datatype, MyGDScript *p_owner, bool p_handle_metatype = true);

	MyGDScriptCodeGenerator::Address _parse_expression(CodeGen &codegen, Error &r_error, const MyGDScriptParser::ExpressionNode *p_expression, bool p_root = false, bool p_initializer = false);
	MyGDScriptCodeGenerator::Address _parse_match_pattern(CodeGen &codegen, Error &r_error, const MyGDScriptParser::PatternNode *p_pattern, const MyGDScriptCodeGenerator::Address &p_value_addr, const MyGDScriptCodeGenerator::Address &p_type_addr, const MyGDScriptCodeGenerator::Address &p_previous_test, bool p_is_first, bool p_is_nested);
	List<MyGDScriptCodeGenerator::Address> _add_block_locals(CodeGen &codegen, const MyGDScriptParser::SuiteNode *p_block);
	void _clear_block_locals(CodeGen &codegen, const List<MyGDScriptCodeGenerator::Address> &p_locals);
	Error _parse_block(CodeGen &codegen, const MyGDScriptParser::SuiteNode *p_block, bool p_add_locals = true, bool p_clear_locals = true);
	MyGDScriptFunction *_parse_function(Error &r_error, MyGDScript *p_script, const MyGDScriptParser::ClassNode *p_class, const MyGDScriptParser::FunctionNode *p_func, bool p_for_ready = false, bool p_for_lambda = false);
	MyGDScriptFunction *_make_static_initializer(Error &r_error, MyGDScript *p_script, const MyGDScriptParser::ClassNode *p_class);
	Error _parse_setter_getter(MyGDScript *p_script, const MyGDScriptParser::ClassNode *p_class, const MyGDScriptParser::VariableNode *p_variable, bool p_is_setter);
	Error _prepare_compilation(MyGDScript *p_script, const MyGDScriptParser::ClassNode *p_class, bool p_keep_state);
	Error _compile_class(MyGDScript *p_script, const MyGDScriptParser::ClassNode *p_class, bool p_keep_state);
	FunctionLambdaInfo _get_function_replacement_info(MyGDScriptFunction *p_func, int p_index = -1, int p_depth = 0, MyGDScriptFunction *p_parent_func = nullptr);
	Vector<FunctionLambdaInfo> _get_function_lambda_replacement_info(MyGDScriptFunction *p_func, int p_depth = 0, MyGDScriptFunction *p_parent_func = nullptr);
	ScriptLambdaInfo _get_script_lambda_replacement_info(MyGDScript *p_script);
	bool _do_function_infos_match(const FunctionLambdaInfo &p_old_info, const FunctionLambdaInfo *p_new_info);
	void _get_function_ptr_replacements(HashMap<MyGDScriptFunction *, MyGDScriptFunction *> &r_replacements, const FunctionLambdaInfo &p_old_info, const FunctionLambdaInfo *p_new_info);
	void _get_function_ptr_replacements(HashMap<MyGDScriptFunction *, MyGDScriptFunction *> &r_replacements, const Vector<FunctionLambdaInfo> &p_old_infos, const Vector<FunctionLambdaInfo> *p_new_infos);
	void _get_function_ptr_replacements(HashMap<MyGDScriptFunction *, MyGDScriptFunction *> &r_replacements, const ScriptLambdaInfo &p_old_info, const ScriptLambdaInfo *p_new_info);
	int err_line = 0;
	int err_column = 0;
	StringName source;
	String error;
	MyGDScriptParser::ExpressionNode *awaited_node = nullptr;
	bool has_static_data = false;

public:
	static void convert_to_initializer_type(Variant &p_variant, const MyGDScriptParser::VariableNode *p_node);
	static void make_scripts(MyGDScript *p_script, const MyGDScriptParser::ClassNode *p_class, bool p_keep_state);
	Error compile(const MyGDScriptParser *p_parser, MyGDScript *p_script, bool p_keep_state = false);

	String get_error() const;
	int get_error_line() const;
	int get_error_column() const;

	MyGDScriptCompiler();
};
