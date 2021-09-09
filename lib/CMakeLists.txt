project(lib_${libname})

##OPTIONS
set(Boost_USE_STATIC_LIBS ON)
find_package(Libevent)
find_package(MbedTls)
find_package(Boost REQUIRED COMPONENTS)

# enable specific macors by system type:
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(PLATFORM_LIBRARIES)
else()
endif()

# local dependencies
include_directories(
	./include/
	${LIBEVENT_INCLUDE_DIRS}
	${MBEDTLS_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	)

link_directories(
	)

# dependencies:
set(shared_lib_dependency_private)
set(shared_lib_dependency_public
	${LIBEVENT_LIBRARIES}
	${MBEDTLS_LIBRARIES}
	${Boost_LIBRARIES}
	${PLATFORM_LIBRARIES}
	${cpp})

# source files:
aux_source_directory(./src/private_zec                             ZEC_SRC)
aux_source_directory(./src/private_zec/Util                        ZEC_SRC)
aux_source_directory(./src/private_zec/Container                   ZEC_SRC)

aux_source_directory(./src/private_zec_ext/Crypto                  ZEC_SRC)
aux_source_directory(./src/private_zec_ext/Utility                 ZEC_SRC)

aux_source_directory(./src/private_redsocks_multi/                 ZEC_SRC)
aux_source_directory(./src/private_redsocks_multi/NetFilter        ZEC_SRC)

if (ZEC_OPTION_STATIC)
#static lib:
	add_library(${libname} ${ZEC_SRC} ${ZEC_NET_SRC} ${ZEC_MYSQL_SRC} ${ZEC_GAME_SRC})
	set_target_properties(${libname} PROPERTIES VERSION 1.0)
	target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname} PRIVATE ZEC_OPTION_STATIC)
	if(ZEC_GLFW_VULKAN)
		target_compile_definitions(${libname} PRIVATE ZEC_GLFW_VULKAN)
	endif()

else()
#dynamic lib:
	add_library(${libname} SHARED  ${ZEC_SRC} ${ZEC_NET_SRC} ${ZEC_MYSQL_SRC} ${ZEC_GAME_SRC})
	set_target_properties(${libname} PROPERTIES VERSION 1.0)
	target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname} PRIVATE ZEC_OPTION_EXPORT_API)
	if(ZEC_GLFW_VULKAN)
		target_compile_definitions(${libname} PRIVATE ZEC_GLFW_VULKAN)
	endif()
endif()

#install
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/zec
	DESTINATION include
	FILES_MATCHING REGEX ".+\.(h|hpp)"
	)
install(TARGETS
	zec
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
