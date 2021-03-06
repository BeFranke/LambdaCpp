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

message("build type: ${CMAKE_BUILD_TYPE}")
set_target_properties(lambda_lib PROPERTIES LINKER_LANGUAGE CXX)
#IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
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
	add_executable(lambda-struct-test test/lambda-struct.cpp)
 	target_link_libraries(lambda-struct-test gtest_main)
	target_link_libraries(lambda-struct-test lambda_lib)
	add_test(NAME lambda-struct-test COMMAND lambda-struct-test)

	add_executable(tokenizer-test test/tokenizer.cpp)
 	target_link_libraries(tokenizer-test gtest_main)
	target_link_libraries(tokenizer-test lambda_lib)
	add_test(NAME tokenizer-test COMMAND tokenizer-test) 

	add_executable(church-encoding-test test/church-encoding.cpp)
	target_link_libraries(church-encoding-test gtest_main)
	target_link_libraries(church-encoding-test lambda_lib)
	add_test(NAME church-encoding-test COMMAND church-encoding-test)	
	
	add_executable(lambda-syntax-test test/lambda-syntax.cpp)
 	target_link_libraries(lambda-syntax-test gtest_main)
	target_link_libraries(lambda-syntax-test lambda_lib)
	add_test(NAME lambda-syntax-test COMMAND lambda-syntax-test)

	### END UNIT TEST INSERTION
#ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)
# repl
add_executable(REPL src/repl.cpp)
target_link_libraries(REPL lambda_lib)
