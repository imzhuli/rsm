# Look for the header file.
find_path(MBEDTLS_INCLUDE_DIR mbedtls/ssl.h)
# Look for the library.
find_library(MBEDTLS_LIBRARY NAMES mbedcrypto mbedtls mbedx509)

# Handle the QUIETLY and REQUIRED arguments and set MBEDTLS_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MBEDTLS DEFAULT_MSG MBEDTLS_LIBRARY MBEDTLS_INCLUDE_DIR)
# Copy the results to the output variables.

if(MBEDTLS_FOUND)
	set(MBEDTLS_LIBRARIES ${MBEDTLS_LIBRARY})
	set(MBEDTLS_INCLUDE_DIRS ${MBEDTLS_INCLUDE_DIR})
else()
	set(MBEDTLS_LIBRARIES)
	set(MBEDTLS_INCLUDE_DIRS)
endif()
