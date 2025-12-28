const A: = preload("base_outer_resolution_a.notest.mygd")
const B: = preload("base_outer_resolution_b.notest.mygd")
const C: = preload("base_outer_resolution_c.notest.mygd")

const Extend: = preload("base_outer_resolution_extend.notest.mygd")

func test() -> void:
	Extend.test_a(A.new())
	Extend.test_b(B.new())
	Extend.InnerClass.test_c(C.new())
	Extend.InnerClass.InnerInnerClass.test_a_b_c(A.new(), B.new(), C.new())
	Extend.InnerClass.InnerInnerClass.test_enum(C.TestEnum.HELLO_WORLD)
	Extend.InnerClass.InnerInnerClass.test_a_prime(A.APrime.new())
