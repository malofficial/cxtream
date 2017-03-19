add_subdirectory("utility")

add_boost_test("test.core.batch" "batch.cpp" "")

add_boost_test("test.core.buffer" "buffer.cpp" "-lpthread")

add_boost_test("test.core.create" "create.cpp" "")

add_boost_test("test.core.csv" "csv.cpp" "")

add_boost_test("test.core.dataframe" "dataframe.cpp" "")

add_boost_test("test.core.drop" "drop.cpp" "")

add_boost_test("test.core.for_each" "for_each.cpp" "")

add_boost_test("test.core.transform" "transform.cpp" "")
