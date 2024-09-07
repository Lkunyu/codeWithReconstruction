cmake_minimum_required (VERSION 3.1)

project (code)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g2 -ggdb")

SET(CMAKE_CXX_STANDARD 20)

find_package(ROOT REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIR})
include(${ROOT_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.C)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

add_library(DTOFBT STATIC ${sources} ${headers})
target_link_libraries(DTOFBT ${ROOT_LIBRARIES})

add_executable(ReadData "ReadData.C")
target_link_libraries(ReadData ${ROOT_LIBRARIES} DTOFBT)

add_executable(T0Cali "T0Cali.C")
target_link_libraries(T0Cali ${ROOT_LIBRARIES} DTOFBT)

add_executable(T0Result "T0Result.C")
target_link_libraries(T0Result ${ROOT_LIBRARIES} DTOFBT)

add_executable(ReadTracker "ReadTracker.C")
target_link_libraries(ReadTracker ${ROOT_LIBRARIES} DTOFBT)

add_executable(CombineData "CombineData.C")
target_link_libraries(CombineData ${ROOT_LIBRARIES} DTOFBT)

add_executable(RecData "RecData.C")
target_link_libraries(RecData ${ROOT_LIBRARIES} DTOFBT)

file(COPY ${PROJECT_SOURCE_DIR}/pars DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/T0channelCaliIn-old.txt DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/T0Cali1.txt DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/T0channeltemp.txt DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/mergeCali.C DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/run.sh DESTINATION ${PROJECT_BINARY_DIR})


