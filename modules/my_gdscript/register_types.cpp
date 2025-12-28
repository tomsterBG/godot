/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include "my_gdscript.h"
#include "my_gdscript_cache.h"
#include "my_gdscript_parser.h"
#include "my_gdscript_tokenizer_buffer.h"
#include "my_gdscript_utility_functions.h"

#ifdef TOOLS_ENABLED
#include "editor/my_gdscript_highlighter.h"
#include "editor/my_gdscript_translation_parser_plugin.h"

#ifndef MY_GDSCRIPT_NO_LSP
#include "language_server/my_gdscript_language_server.h"
#endif
#endif // TOOLS_ENABLED

#ifdef TESTS_ENABLED
#include "tests/test_my_gdscript.h"
#endif

#include "core/io/file_access.h"
#include "core/io/resource_loader.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#include "editor/export/editor_export.h"
#include "editor/translations/editor_translation_parser.h"

#ifndef MY_GDSCRIPT_NO_LSP
#include "core/config/engine.h"
#endif
#endif // TOOLS_ENABLED

#ifdef TESTS_ENABLED
#include "tests/test_macros.h"
#endif

MyGDScriptLanguage *my_script_language_gd = nullptr;
Ref<ResourceFormatLoaderMyGDScript> my_resource_loader_gd;
Ref<ResourceFormatSaverMyGDScript> my_resource_saver_gd;
MyGDScriptCache *my_gdscript_cache = nullptr;

#ifdef TOOLS_ENABLED

Ref<MyGDScriptEditorTranslationParserPlugin> my_gdscript_translation_parser_plugin;

class EditorExportMyGDScript : public EditorExportPlugin {
	GDCLASS(EditorExportMyGDScript, EditorExportPlugin);

	static constexpr int DEFAULT_SCRIPT_MODE = EditorExportPreset::MODE_SCRIPT_BINARY_TOKENS_COMPRESSED;
	int script_mode = DEFAULT_SCRIPT_MODE;

protected:
	virtual void _export_begin(const HashSet<String> &p_features, bool p_debug, const String &p_path, int p_flags) override {
		script_mode = DEFAULT_SCRIPT_MODE;

		const Ref<EditorExportPreset> &preset = get_export_preset();
		if (preset.is_valid()) {
			script_mode = preset->get_script_export_mode();
		}
	}

	virtual void _export_file(const String &p_path, const String &p_type, const HashSet<String> &p_features) override {
		if (p_path.get_extension() != "mygd" || script_mode == EditorExportPreset::MODE_SCRIPT_TEXT) {
			return;
		}

		Vector<uint8_t> file = FileAccess::get_file_as_bytes(p_path);
		if (file.is_empty()) {
			return;
		}

		String source = String::utf8(reinterpret_cast<const char *>(file.ptr()), file.size());
		MyGDScriptTokenizerBuffer::CompressMode compress_mode = script_mode == EditorExportPreset::MODE_SCRIPT_BINARY_TOKENS_COMPRESSED ? MyGDScriptTokenizerBuffer::COMPRESS_ZSTD : MyGDScriptTokenizerBuffer::COMPRESS_NONE;
		file = MyGDScriptTokenizerBuffer::parse_code_string(source, compress_mode);
		if (file.is_empty()) {
			return;
		}

		add_file(p_path.get_basename() + ".mygdc", file, true);
	}

public:
	virtual String get_name() const override { return "MyGDScript"; }
};

static void _editor_init() {
	Ref<EditorExportMyGDScript> gd_export;
	gd_export.instantiate();
	EditorExport::get_singleton()->add_export_plugin(gd_export);

#ifdef TOOLS_ENABLED
	Ref<MyGDScriptSyntaxHighlighter> my_gdscript_syntax_highlighter;
	my_gdscript_syntax_highlighter.instantiate();
	ScriptEditor::get_singleton()->register_syntax_highlighter(my_gdscript_syntax_highlighter);
#endif

#ifndef MY_GDSCRIPT_NO_LSP
	register_my_gdscript_lsp_types();
	MyGDScriptLanguageServer *lsp_plugin = memnew(MyGDScriptLanguageServer);
	EditorNode::get_singleton()->add_editor_plugin(lsp_plugin);
	Engine::get_singleton()->add_singleton(Engine::Singleton("MyGDScriptLanguageProtocol", MyGDScriptLanguageProtocol::get_singleton()));
#endif // !MY_GDSCRIPT_NO_LSP
}

#endif // TOOLS_ENABLED

void initialize_my_gdscript_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		GDREGISTER_CLASS(MyGDScript);

		my_script_language_gd = memnew(MyGDScriptLanguage);
		ScriptServer::register_language(my_script_language_gd);

		my_resource_loader_gd.instantiate();
		ResourceLoader::add_resource_format_loader(my_resource_loader_gd);

		my_resource_saver_gd.instantiate();
		ResourceSaver::add_resource_format_saver(my_resource_saver_gd);

		my_gdscript_cache = memnew(MyGDScriptCache);

		MyGDScriptUtilityFunctions::register_functions();
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		EditorNode::add_init_callback(_editor_init);

		my_gdscript_translation_parser_plugin.instantiate();
		EditorTranslationParser::get_singleton()->add_parser(my_gdscript_translation_parser_plugin, EditorTranslationParser::STANDARD);
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(MyGDScriptSyntaxHighlighter);
	}
#endif // TOOLS_ENABLED
}

void uninitialize_my_gdscript_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		ScriptServer::unregister_language(my_script_language_gd);

		if (my_gdscript_cache) {
			memdelete(my_gdscript_cache);
		}

		if (my_script_language_gd) {
			memdelete(my_script_language_gd);
		}

		ResourceLoader::remove_resource_format_loader(my_resource_loader_gd);
		my_resource_loader_gd.unref();

		ResourceSaver::remove_resource_format_saver(my_resource_saver_gd);
		my_resource_saver_gd.unref();

		MyGDScriptParser::cleanup();
		MyGDScriptUtilityFunctions::unregister_functions();
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		EditorTranslationParser::get_singleton()->remove_parser(my_gdscript_translation_parser_plugin, EditorTranslationParser::STANDARD);
		my_gdscript_translation_parser_plugin.unref();
	}
#endif // TOOLS_ENABLED
}

#ifdef TESTS_ENABLED
void test_tokenizer() {
	MyGDScriptTests::test(MyGDScriptTests::TestType::TEST_TOKENIZER);
}

void test_tokenizer_buffer() {
	MyGDScriptTests::test(MyGDScriptTests::TestType::TEST_TOKENIZER_BUFFER);
}

void test_parser() {
	MyGDScriptTests::test(MyGDScriptTests::TestType::TEST_PARSER);
}

void test_compiler() {
	MyGDScriptTests::test(MyGDScriptTests::TestType::TEST_COMPILER);
}

void test_bytecode() {
	MyGDScriptTests::test(MyGDScriptTests::TestType::TEST_BYTECODE);
}

REGISTER_TEST_COMMAND("my_gdscript-tokenizer", &test_tokenizer);
REGISTER_TEST_COMMAND("my_gdscript-tokenizer-buffer", &test_tokenizer_buffer);
REGISTER_TEST_COMMAND("my_gdscript-parser", &test_parser);
REGISTER_TEST_COMMAND("my_gdscript-compiler", &test_compiler);
REGISTER_TEST_COMMAND("my_gdscript-bytecode", &test_bytecode);
#endif
