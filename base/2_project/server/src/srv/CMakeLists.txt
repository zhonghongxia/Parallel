AUX_SOURCE_DIRECTORY(. DIR_SRCS)
ADD_EXECUTABLE(srv ${DIR_SRCS})
TARGET_LINK_LIBRARIES(srv jsoncpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -O2 ")
target_link_libraries(srv boost_system boost_thread) 

