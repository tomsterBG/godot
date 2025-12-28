const A = preload("res://completion/class_a.notest.mygd")

class LocalInnerClass:
    const AInner = preload("res://completion/class_a.notest.mygd")
    class InnerInnerClass:
        const AInnerInner = preload("res://completion/class_a.notest.mygd")
        enum InnerInnerInnerEnum {}
        class InnerInnerInnerClass:
            pass
    enum InnerInnerEnum {}

enum TestEnum {}

var test_var: LocalInnerClass.InnerInnerClass.➡
