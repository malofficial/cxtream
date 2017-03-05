add_subdirectory("utility")

add_boost_test("test.batch" "batch.cpp" "" "")

add_boost_test("test.buffer" "buffer.cpp" "" "-lpthread")

add_boost_test("test.create" "create.cpp" "" "")

add_boost_test("test.drop" "drop.cpp" "" "")

add_boost_test("test.for_each" "for_each.cpp" "" "")

add_boost_test("test.transform" "transform.cpp" "" "")
