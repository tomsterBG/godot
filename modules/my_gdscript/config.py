def can_build(env, platform):
    env.module_add_dependencies("my_gdscript", ["jsonrpc", "websocket"], True)
    return True


def configure(env):
    pass


def get_doc_classes():
    return [
        "@MyGDScript",
        "MyGDScript",
        "MyGDScriptSyntaxHighlighter",
    ]


def get_doc_path():
    return "doc_classes"
