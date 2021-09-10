project(test_${libname})

# dependencies:
set(shared_lib_dependency_private)

# local dependencies
include_directories(
	../lib/include/
	)
link_directories(
	)

function(add_program sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(app_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(app_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(app_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (ZEC_OPTION_STATIC)
	target_compile_definitions(app_${sub_dir_name} PRIVATE ZEC_OPTION_STATIC)
endif()
endfunction()

add_program(rsm)
