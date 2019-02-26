cmake_minimum_required (VERSION 3.1)
PROJECT (resize)

SET (CMAKE_CXX_STANDARD 11)

IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug build.")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
  message("Release build.")
ELSE()
  message("Some other build type.")
ENDIF()


INCLUDE_DIRECTORIES(thirdparty/lodepng ./)
ADD_LIBRARY( smartResize STATIC
             smartResize.cc
             Seam.cc
             SmartImageScale.cc )

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/smartResize.hpp
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)

ADD_EXECUTABLE(resize
    example.cc
    thirdparty/lodepng/lodepng.cpp)

TARGET_LINK_LIBRARIES( resize
                       smartResize )