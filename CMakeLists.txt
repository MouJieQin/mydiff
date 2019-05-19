cmake_minimum_required (VERSION 2.8)

SET(RELEASE_FLAG "RELEASE" CACHE STRING "add a postfix, usually empty on windows")  
SET(DEBUG_FLAG "DEBUG" CACHE STRING "add a postfix, usually d on windows")  
SET(COVERAGE_FLAG "COVERAGE" CACHE STRING "add a postfix, usually empty on windows")  
SET(GPERF_FLAG "GPERF" CACHE STRING "add a postfix, usually empty on windows")  
   
# Set the build type according to what configuration is being built.  
IF (CMAKE_BUILD_TYPE MATCHES "release")  
    SET(BUILD_TYPE ${RELEASE_FLAG})  
ELSEIF (CMAKE_BUILD_TYPE MATCHES "debug")  
    SET(BUILD_TYPE ${DEBUG_FLAG})  
ELSEIF(CMAKE_BUILD_TYPE MATCHES "coverage")  
    SET(BUILD_TYPE ${COVERAGE_FLAG})  
ELSEIF(CMAKE_BUILD_TYPE MATCHES "gperf")  
    SET(BUILD_TYPE ${GPERF_FLAG})  
ELSE()
    SET(BUILD_TYPE ${RELEASE_FLAG})  
ENDIF()


SET(mydiff_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${mydiff_ROOT_DIR}/src)

project (mydiff)

IF (${BUILD_TYPE} STREQUAL ${RELEASE_FLAG})
    ADD_DEFINITIONS(-std=c++11 -O2 -W -Wall)
ELSEIF (${BUILD_TYPE} STREQUAL ${DEBUG_FLAG})
    ADD_DEFINITIONS(-std=c++11 -O0 -g -W -Wall -DDEBUG)
ELSEIF (${BUILD_TYPE} STREQUAL ${COVERAGE_FLAG})
    ADD_DEFINITIONS(-std=c++11 -O0 -W -Wall -g --coverage)
ELSEIF (${BUILD_TYPE} STREQUAL ${GPERF_FLAG})
    ADD_DEFINITIONS(-std=c++11 -O2 -g -W -Wall -DGPERF)
ENDIF()

#add_subdirectory(src/lib)

AUX_SOURCE_DIRECTORY(${mydiff_ROOT_DIR}/src/tpcds DIR_LIB_SRCS)
ADD_EXECUTABLE(mydiff ${mydiff_ROOT_DIR}/src/main/mydiff-main.cpp ${DIR_LIB_SRCS})


IF (${BUILD_TYPE} STREQUAL ${COVERAGE_FLAG})
    TARGET_LINK_LIBRARIES(mydiff -fprofile-arcs -ftest-coverage)
    ADD_CUSTOM_TARGET(coverage
        COMMAND lcov --base-directory . --directory . --capture --output-file CodeCoverage.info --ignore-errors graph --rc lcov_branch_coverage=1
        COMMAND lcov --remove CodeCoverage.info '/Library/*' --output-file CodeCoverage.info.cleaned --rc lcov_branch_coverage=1
        COMMAND genhtml --branch-coverage CodeCoverage.info.cleaned -o CodeCoverageReport --rc lcov_branch_coverage=1
    )
    ADD_CUSTOM_TARGET(open
        COMMAND open CodeCoverageReport/index.html
    )
ELSEIF (${BUILD_TYPE} STREQUAL ${GPERF_FLAG})
    TARGET_LINK_LIBRARIES(mydiff -lprofiler)
ENDIF()

INSTALL(TARGETS mydiff DESTINATION /usr/local/bin)
