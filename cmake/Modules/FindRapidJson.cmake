# Look for the header file.
find_path(RAPIDJSON_INCLUDE_DIR rapidjson/document.h)
# Look for the library.

# Handle the QUIETLY and REQUIRED arguments and set RAPIDJSON_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RAPIDJSON DEFAULT_MSG RAPIDJSON_INCLUDE_DIR)
# Copy the results to the output variables.

if(RAPIDJSON_FOUND)
	set(RAPIDJSON_INCLUDE_DIRS ${RAPIDJSON_INCLUDE_DIR})
else()
	set(RAPIDJSON_INCLUDE_DIRS)
endif()
