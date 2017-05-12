add_boost_test("test.core.stream.batch" "batch.cpp" "")

add_boost_test("test.core.stream.buffer" "buffer.cpp" "-lpthread")

add_boost_test("test.core.stream.create" "create.cpp" "")

add_boost_test("test.core.stream.drop" "drop.cpp" "")

add_boost_test("test.core.stream.filter" "filter.cpp" "")

add_boost_test("test.core.stream.for_each" "for_each.cpp" "")

add_boost_test("test.core.stream.random_fill" "random_fill.cpp" "")

add_boost_test("test.core.stream.transform" "transform.cpp" "")
