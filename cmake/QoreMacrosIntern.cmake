#
# Written by Niclas Rosenvik <youremailsarecrap@gmail.com>
# Based on the tests found in qores configure.ac
#
#

include(CheckCSourceCompiles)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckCXXSymbolExists)
include(CheckFunctionExists)
include(CheckIncludeFileCXX)
include(CMakeParseArguments)
include(CMakePushCheckState)


macro(qore_check_headers_cxx)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs)

    cmake_parse_arguments(_HEADERS "${options}" "${onevalueArgs}" "${multiValueArgs}" ${ARGN})

    foreach (include_file ${_HEADERS_UNPARSED_ARGUMENTS})
        string(REPLACE "." "_" include_file_ ${include_file})
	string(REPLACE "/" "_" _include_file_ ${include_file_})
	string(TOUPPER ${_include_file_} upper_include_file_)
	check_include_file_cxx(${include_file} "HAVE_${upper_include_file_}")
#	string(CONCAT file_output ${file_output} "\n#cmakedefine " ${have_var})
    endforeach()
#    file(WRITE ${CMAKE_BINARY_DIR}/header_defines.txt ${file_output})
#    unset(file_output)
endmacro()



macro(qore_check_funcs)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs)

    cmake_parse_arguments(_FUNCS "${options}" "${onevalueArgs}" "${multiValueArgs}" ${ARGN})

    foreach (FUNCTION_TO_CHECK ${_FUNCS_UNPARSED_ARGUMENTS})
        string(TOUPPER ${FUNCTION_TO_CHECK} UPPER_FUNCTION_TO_CHECK)
	check_function_exists(${FUNCTION_TO_CHECK} "HAVE_${UPPER_FUNCTION_TO_CHECK}")
#	string(CONCAT file_output ${file_output} "\n#cmakedefine " ${HAVE_VAR})
    endforeach()
#    file(WRITE ${CMAKE_BINARY_DIR}/funcs_defines.txt ${file_output})
#    unset(file_output)
endmacro()

function(qore_search_libs _list_to_append_lib_to _function)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs)

    cmake_parse_arguments(_LIBS "${options}" "${onevalueArgs}" "${multiValueArgs}" ${ARGN})
    message(STATUS "Looking for library containing ${_function}")
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_QUIET true)
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_CXX_IMPLICIT_LINK_LIBRARIES})
    check_function_exists(${_function} FUNCTION_FOUND)
    if (FUNCTION_FOUND)
        message(STATUS "Found ${_function} in the implicitly linked libs")
    else()
        set(FUNCTIONNOTFOUND true)
        foreach (_lib ${_LIBS_UNPARSED_ARGUMENTS})
	    find_library(_library_found NAMES ${_lib})
	    
	    if (NOT _library_found EQUAL "_library_found-NOTFOUND")
	        set(CMAKE_REQUIRED_LIBRARIES ${_library_found})
		check_function_exists(${_function} FUNCTION_FOUND_IN_${_lib})
		if (FUNCTION_FOUND_IN_${_lib})
		    set(${_list_to_append_lib_to} ${${_list_to_append_lib_to}} ${_library_found} PARENT_SCOPE)
		    message(STATUS "Found ${_function} in ${_library_found}")
		    unset(_library_found)
		    set(FUNCTIONNOTFOUND false)
		    break()
		endif()
		unset(_library_found)
	    endif()
	endforeach()
        if (FUNCTIONNOTFOUND)
	    message(STATUS "${_function} could not be found in ${_LIBS_UNPARSED_ARGUMENTS} or the implicity linked libs")
	endif()
    endif()
    cmake_pop_check_state()
endfunction()

macro(qore_openssl_checks)

set(CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES})

check_cxx_symbol_exists(EVP_sha512 openssl/evp.h HAVE_OPENSSL_SHA512)

# check for openssl const correctness (later versions have it)
check_cxx_source_compiles("
#include <openssl/x509.h>
int main(void) {
const unsigned char *p;
d2i_X509(0, &p, 0l);
return 0;
}" 
HAVE_OPENSSL_CONST)

# check for const required with SSL_CTX_new
check_cxx_source_compiles("
#include <openssl/ssl.h>
int main(void) {
const SSL_METHOD *meth;
SSL_CTX_new(meth);
return 0;
}" 
NEED_SSL_CTX_NEW_CONST)

# check for return value with HMAC_Update and friends
check_cxx_source_compiles("
#include <openssl/hmac.h>
int main(void) {
int rc = HMAC_Update(0, 0, 0);
return 0;
}" 
HAVE_OPENSSL_HMAC_RV)

unset(CMAKE_REQUIRED_INCLUDES)
unset(CMAKE_REQUIRED_LIBRARIES)

endmacro()



macro(qore_mpfr_checks)

set(CMAKE_REQUIRED_INCLUDES ${MPFR_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${MPFR_LIBRARIES})

check_cxx_symbol_exists(mpfr_divby0_p mpfr.h HAVE_MPFR_DIVBY0)
check_cxx_symbol_exists(mpfr_regular_p mpfr.h HAVE_MPFR_REGULAR)
check_cxx_symbol_exists(mpfr_sprintf mpfr.h HAVE_MPFR_SPRINTF)
check_cxx_symbol_exists(mpfr_buildopt_tls_p mpfr.h HAVE_MPFR_BUILDOPT_TLS_P)

check_cxx_source_compiles("
#include <mpfr.h>
int main(void){
mpfr_exp_t test;
return 0;
}"
HAVE_MPFR_EXP_T)

check_cxx_source_compiles("
#include <mpfr.h>
int main(void){
mpfr_rnd_t test = MPFR_RNDN;
return 0;
}"
HAVE_RNDN)


unset(CMAKE_REQUIRED_INCLUDES)
unset(CMAKE_REQUIRED_LIBRARIES)

endmacro()



macro(qore_find_pthreads)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
    message(STATUS "Found POSIX Threads: TRUE")
else(CMAKE_USE_PTHREADS_INIT)
    message(STATUS "Found POSIX Threads: FALSE")
    message(FATAL_ERROR "POSIX threads do not seem to be supported on this platform, aborting")
endif()
endmacro()



macro(qore_gethost_checks)

check_cxx_symbol_exists(gethostbyaddr_r netdb.h HAVE_GETHOSTBYADDR_R)
check_cxx_symbol_exists(gethostbyname_r netdb.h HAVE_GETHOSTBYNAME_R)

if(HAVE_GETHOSTBYADDR_R)

# Check definition of gethostbyaddr_r (glibc2 defines this with 8 arguments)
check_c_source_compiles("
#undef inline
#if !defined(SCO) && !defined(__osf__) && !defined(_REENTRANT)
#define _REENTRANT
#endif
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
int main(void) {
int skr;
struct hostent *foo = gethostbyaddr_r((const char *) 0,0, 0, (struct hostent *) 0, (char *) NULL,  0, &skr);
return (foo == 0);
}"
HAVE_SOLARIS_STYLE_GETHOST)

endif(HAVE_GETHOSTBYADDR_R)



if(HAVE_GETHOSTBYNAME_R)

# Check definition of gethostbyname_r (glibc2.0.100 is different from Solaris)
check_c_source_compiles("
#undef inline
#if !defined(SCO) && !defined(__osf__) && !defined(_REENTRANT)
#define _REENTRANT
#endif
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
int main(void){
int skr;
skr = gethostbyname_r((const char *) 0,(struct hostent*) 0, (char*) 0, 0, (struct hostent **) 0, &skr);
return 0;
}"
HAVE_GETHOSTBYNAME_R_GLIBC2_STYLE)

# Check 3rd argument of gethostbyname_r
check_c_source_compiles("
#undef inline
#if !defined(SCO) && !defined(__osf__) && !defined(_REENTRANT)
#define _REENTRANT
#endif
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
int main(void){
int skr;
skr = gethostbyname_r((const char *) 0, (struct hostent*) 0, (struct hostent_data*) 0);
return 0;
}"
HAVE_GETHOSTBYNAME_R_RETURN_INT)

endif(HAVE_GETHOSTBYNAME_R)

endmacro()



macro(qore_other_checks)

# check if the compiler supports variadic local arrays
check_cxx_source_compiles("
int main(void){
int x = 2; int y[x];
return 0;
}"
HAVE_LOCAL_VARIADIC_ARRAYS)

# see if struct flock is declared
check_c_source_compiles("
#include <fcntl.h>
int main(void){
struct flock fl;
return 0;
}"
HAVE_STRUCT_FLOCK)

check_cxx_compiler_flag(-fvisibility=hidden HAVE_GCC_VISIBILITY)

check_cxx_symbol_exists(strtoimax inttypes.h HAVE_STRTOIMAX)

#see if compiler supports namespaces
check_cxx_source_compiles("
namespace Outer {namespace Inner { int i = 0; }}
int main(void){
using namespace Outer::Inner;
return i;
}" HAVE_NAMESPACES)

endmacro()

macro(create_git_revision)
if (NOT EXISTS ${CMAKE_SOURCE_DIR}/include/qore/intern/git-revision.h)
   find_package(Git)
   if(GIT_FOUND)
       execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		       OUTPUT_VARIABLE GIT_REV OUTPUT_STRIP_TRAILING_WHITESPACE)
       string(CONCAT GIT_REV_FILE_OUTPUT "#define BUILD \"" ${GIT_REV} "\"")
       file(WRITE ${CMAKE_BINARY_DIR}/include/qore/intern/git-revision.h "#define BUILD \"${GIT_REV}\"")
   else()
       message(FATAL_ERROR "Git is needed to generate git-revision.h")
   endif()
   
endif()

endmacro()


macro(qore_stl_hash_map _hash_map_output_file)

set(CMAKE_REQUIRED_QUIET true)
set(STL_HASH_MAP_FOUND false)
message(STATUS "Checking location of STL unordered_map")

#unordered map
check_cxx_source_compiles("
#include <unordered_map>
int main(void){
unordered_map<int, int> t;
const unordered_map<int, int> &tr = t;
tr.find(1); 
return 0;
}" UNORDERED_MAP_FOUND)

if(UNORDERED_MAP_FOUND)
set(HAVE_UNORDERED_MAP true)
set(HAVE_QORE_HASH_MAP true)
set(HASH_MAP_INCLUDE "<unordered_map>")
set(HASH_NAMESPACE "")
set(HASH_TYPE "unordered_map")
set(STL_HASH_MAP_FOUND true)
endif()

#std unordered map
if(NOT STL_HASH_MAP_FOUND)
check_cxx_source_compiles("
#include <unordered_map>
int main(void){
std::unordered_map<int, int> t;
const std::unordered_map<int, int> &tr = t;
tr.find(1);
return 0;
}" STD_UNORDERED_MAP_FOUND)

if(STD_UNORDERED_MAP_FOUND)
set(HAVE_UNORDERED_MAP true)
set(HAVE_QORE_HASH_MAP true)
set(HASH_MAP_INCLUDE "<unordered_map>")
set(HASH_NAMESPACE "std")
set(HASH_TYPE "std_unordered_map")
set(STL_HASH_MAP_FOUND true)
endif()
endif()

#tr1 unordered map
if(NOT STL_HASH_MAP_FOUND)
check_cxx_source_compiles("
#include <tr1/unordered_map>
int main(void){
std::tr1::unordered_map<int, int> t;
const std::tr1::unordered_map<int, int> &tr = t; 
tr.find(1);
return 0;
}" TR1_UNORDERED_MAP_FOUND)

if(TR1_UNORDERED_MAP_FOUND)
set(HAVE_UNORDERED_MAP true)
set(HAVE_QORE_HASH_MAP true)
set(HASH_MAP_INCLUDE "<tr1/unordered_map>")
set(HASH_NAMESPACE "std::tr1")
set(HASH_TYPE "tr1_unordered_map")
set(STL_HASH_MAP_FOUND true)
endif()
endif()

#hash map
if(NOT STL_HASH_MAP_FOUND)
check_cxx_source_compiles("
#include <hash_map>
int main(void){
hash_map<int, int> t;
return 0;
}" HASH_MAP_FOUND)

if(HASH_MAP_FOUND)
set(HAVE_HASH_MAP true)
set(HAVE_HASH_SET true)
set(HAVE_QORE_HASH_MAP true)
set(HASH_MAP_INCLUDE "<hash_map>")
set(HASH_SET_INCLUDE "<hash_set>")
set(HASH_NAMESPACE "")
set(HASH_TYPE "hash_map")
set(STL_HASH_MAP_FOUND true)
endif()
endif()

#ext hash map
if(NOT STL_HASH_MAP_FOUND)
check_cxx_source_compiles("
#include <ext/hash_map>
int main(void){
__gnu_cxx::hash_map<int, int> t;
return 0;
}" EXT_HASH_MAP_FOUND)

if(EXT_HASH_MAP_FOUND)
set(HAVE_EXT_HASH_MAP true)
set(HAVE_EXT_HASH_SET true)
set(HAVE_QORE_HASH_MAP true)
set(HASH_MAP_INCLUDE "<ext/hash_map>")
set(HASH_SET_INCLUDE "<ext/hash_set>")
set(HASH_NAMESPACE "__gnu_cxx")
set(HASH_TYPE "ext_hash_map")
set(STL_HASH_MAP_FOUND true)
endif()
endif()

#std hash map
if(NOT STL_HASH_MAP_FOUND)
check_cxx_source_compiles("
#include <hash_map>
int main(void){
std::hash_map<int, int> t;
return 0;
}" STD_HASH_MAP_FOUND)

if(STD_HASH_MAP_FOUND)
set(HAVE_HASH_MAP true)
set(HAVE_HASH_SET true)
set(HAVE_QORE_HASH_MAP true)
set(HASH_MAP_INCLUDE "<hash_map>")
set(HASH_SET_INCLUDE "<hash_set>")
set(HASH_NAMESPACE "std")
set(HASH_TYPE "std_hash_map")
set(STL_HASH_MAP_FOUND true)
endif()
endif()

#stdext hash map
if(NOT STL_HASH_MAP_FOUND)
check_cxx_source_compiles("
#include <hash_map>
int main(void){
std::hash_map<int, int> t;
return 0;
}" STDEXT_HASH_MAP_FOUND)

if(STDEXT_HASH_MAP_FOUND)
set(HAVE_HASH_MAP true)
set(HAVE_HASH_SET true)
set(HAVE_QORE_HASH_MAP true)
set(HASH_MAP_INCLUDE "<hash_map>")
set(HASH_SET_INCLUDE "<hash_set>")
set(HASH_NAMESPACE "stdext")
set(HASH_TYPE "stdext_hash_map")
set(STL_HASH_MAP_FOUND true)
endif()
endif()

if(STL_HASH_MAP_FOUND)
message(STATUS "STL unordered_map found: ${HASH_TYPE}")
else()
message(WARNING "Couldn't find an STL unordered_map")
endif()

#write the file.
configure_file(${CMAKE_SOURCE_DIR}/cmake/stl_hash_map.in 
               ${CMAKE_BINARY_DIR}/${_hash_map_output_file})

unset(CMAKE_REQUIRED_QUIET)
endmacro()



macro(qore_stl_slist _stl_slist_output_file)

set(CMAKE_REQUIRED_QUIET true)
set(STL_SLIST_FOUND false)
message(STATUS "Checking location of STL slist")

#slist
check_cxx_source_compiles("
#include <slist>
int main(void){
slist<int> t;
return 0;
}" SLIST_FOUND)

if(SLIST_FOUND)
set(HAVE_SLIST true)
set(HAVE_QORE_SLIST true)
set(SLIST_INCLUDE "<slist>")
set(LIST_SET_INCLUDE "list_set")
set(LIST_NAMESPACE "")
set(SLIST_TYPE "slist")
set(STL_SLIST_FOUND true)
endif()

#ext slist
if(NOT STL_SLIST_FOUND)
check_cxx_source_compiles("
#include <ext/slist>
int main(void){
__gnu_cxx::slist<int> t;
return 0;
}" EXT_SLIST_FOUND)

if(EXT_SLIST_FOUND)
set(HAVE_SLIST true)
set(HAVE_QORE_SLIST true)
set(SLIST_INCLUDE "<ext/slist>")
set(LIST_SET_INCLUDE "<ext/list_set>")
set(LIST_NAMESPACE "__gnu_cxx")
set(SLIST_TYPE "ext_slist")
set(STL_SLIST_FOUND true)
endif()
endif()


#std slist
if(NOT STL_SLIST_FOUND)
check_cxx_source_compiles("
#include <slist>
int main(void){
std::slist<int> t;
return 0;
}" STD_SLIST_FOUND)

if(STD_SLIST_FOUND)
set(HAVE_SLIST true)
set(HAVE_QORE_SLIST true)
set(SLIST_INCLUDE "<slist>")
set(LIST_SET_INCLUDE "<list_set>")
set(LIST_NAMESPACE "std")
set(SLIST_TYPE "std_slist")
set(STL_SLIST_FOUND true)
endif()
endif()

#stdext slist
if(NOT STL_SLIST_FOUND)
check_cxx_source_compiles("
#include <slist>
int main(void){
stdext::slist<int> t;
return 0;
}" STDEXT_SLIST_FOUND)

if(STDEXT_SLIST_FOUND)
set(HAVE_SLIST true)
set(HAVE_QORE_SLIST true)
set(SLIST_INCLUDE "<slist>")
set(LIST_SET_INCLUDE "<list_set>")
set(LIST_NAMESPACE "stdext")
set(SLIST_TYPE "stdext_slist")
set(STL_SLIST_FOUND true)
endif()
endif()

if(STL_SLIST_FOUND)
message(STATUS "STL slist found: ${SLIST_TYPE}")
else()
message(WARNING "couldn't find an STL slist")
endif()

configure_file(${CMAKE_SOURCE_DIR}/cmake/stl_slist.in 
               ${CMAKE_BINARY_DIR}/${_stl_slist_output_file})

unset(CMAKE_REQUIRED_QUIET)

endmacro()

macro(qore_func_strerror_r)
cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_QUIET true)
message(STATUS "Looking for strerror_r")
check_cxx_symbol_exists(strerror_r string.h HAVE_DECL_STRERROR_R)
if(HAVE_DECL_STRERROR_R)
   check_function_exists(strerror_r HAVE_STRERROR_R)
   if(HAVE_STRERROR_R)
      message(STATUS "Looking for strerror_r - found")
      message(STATUS "Checking the return type of strerror_r")
      check_cxx_source_compiles("
      #include <string.h>
      int main(void) {
      char strbuf[100];
      char *retbuf = NULL;
      retbuf = strerror_r(0, strbuf, sizeof(strbuf));
      return 0;
      }" STRERROR_R_CHAR_P)
      if(STRERROR_R_CHAR_P)
         message(STATUS "Checking the return type of strerror_r - char*")
      else()
         message(STATUS "Checking the return type of strerror_r - int")
      endif()
   else()
      message(STATUS "Looking for strerror_r - not found")
   endif()
else()
   message(STATUS "Looking for strerror_r - not found")
endif()
cmake_pop_check_state()
endmacro()
