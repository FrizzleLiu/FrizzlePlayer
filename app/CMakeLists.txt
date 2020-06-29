cmake_minimum_required(VERSION 3.4.1)

file(GLOB SOURCE src/main/cpp/*.cpp)
add_library( # Sets the name of the library.
        frizzleplayer
        SHARED
        ${SOURCE})
include_directories(src/main/cpp/include)

#set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")
set(my_lib_path ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${my_lib_path}")
find_library( # Sets the name of the path variable.
        log-lib
        log)

target_link_libraries( # Specifies the target library.
        frizzleplayer
        avfilter avformat avcodec avutil swresample swscale
        android
        z
        OpenSLES
        ${log-lib})