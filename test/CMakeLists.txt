project(test_${libname})

# dependencies:
set(shared_lib_dependency_private)

# local dependencies
include_directories(
	../lib/include/
	../lib/src/
	)
link_directories(
	)

function(add_sub_test sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(test_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (ZEC_OPTION_STATIC)
	target_compile_definitions(test_${sub_dir_name} PRIVATE ZEC_OPTION_STATIC)
endif()
	add_test(
		NAME	test_${sub_dir_name}
		COMMAND test_${sub_dir_name}
		WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
		)
endfunction()

#test lib:
add_sub_test(uuid)
