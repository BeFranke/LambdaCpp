# some of this is taken from https://jhbell.com/using-cmake-and-gcov

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall --coverage")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")
cmake_minimum_required(VERSION 3.16)
project(lambda)

set(CMAKE_CXX_STANDARD 17)

add_library(lambda_lib
        src/lib/lambda-exceptions.hpp
        src/lib/lambda-syntax.hpp
        src/lib/lambda-struct.hpp
        src/lib/lambda-struct.cpp
        src/lib/tokenizer.hpp
        src/lib/church-encoding.hpp
        src/lib/church-encoding.cpp
        )
set_target_properties(lambda_lib PROPERTIES LINKER_LANGUAGE CXX)
IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
	enable_testing()
	### START UNIT TEST INSERTION
	# testing setup from https://github.com/google/googletest/tree/master/googletest#incorporating-into-an-existing-cmake-project
	# Download and unpack googletest at configure time
	configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        	RESULT_VARIABLE result
        	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
	if(result)
    		message(FATAL_ERROR "CMake step for googletest failed: ${result}")
	endif()
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
        	RESULT_VARIABLE result
        	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
	if(result)
    		message(FATAL_ERROR "Build step for googletest failed: ${result}")
	endif()

	# Prevent overriding the parent project's compiler/linker
	# settings on Windows
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	# Add googletest directly to our build. This defines
	# the gtest and gtest_main targets.
	add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        	${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        	EXCLUDE_FROM_ALL)

	# The gtest/gtest_main targets carry header search path
	# dependencies automatically when using CMake 2.8.11 or
	# later. Otherwise we have to add them here ourselves.
	if (CMAKE_VERSION VERSION_LESS 2.8.11)
    		include_directories("${gtest_SOURCE_DIR}/include")
	endif()

	# Now simply link against gtest or gtest_main as needed.
	# lambda-struct.hpp test
	add_executable(all-tests test/tokenizer.cpp test/lambda-syntax.cpp test/lambda-struct.cpp test/church-encoding.cpp src/lib/lambda-struct.cpp)
	# build tests before running
	# source https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests/56448477#56448477
	add_test(test_build
  		"${CMAKE_COMMAND}"
  		--build "${CMAKE_BINARY_DIR}"
  		--config "$<CONFIG>"
  		--target all-tests
		)
	target_link_libraries(all-tests gtest_main)
	target_link_libraries(all-tests lambda_lib)
	set_tests_properties(test_build PROPERTIES FIXTURES_SETUP    test_fixture)
	add_test(NAME all-tests COMMAND all-tests)
	set_tests_properties(all-tests PROPERTIES FIXTURES_REQUIRED test_fixture)
	### END UNIT TEST INSERTION
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)
# repl
add_executable(REPL src/repl.cpp)
target_link_libraries(REPL lambda_lib)
