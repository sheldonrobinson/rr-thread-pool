cmake_minimum_required(VERSION 2.8)
project(rr-thread-pool)

ADD_DEFINITIONS(-std=c++11)

include_directories(BEFORE src)

add_library(tp-lib OBJECT
    src/Cond.cpp
    src/MessageQueue.cpp
    src/Mutex.cpp
    src/Thread.cpp
    src/ThreadPool.cpp
    src/Trace.cpp
    src/Cond.h
    src/Locker.h
    src/Message.h
    src/MessageQueue.h
    src/Mutex.h
    src/Task.h
    src/Thread.h
    src/ThreadPool.h
    src/Trace.h)

add_library(tp-doc OBJECT
    doc/Documentation.h)

add_executable(tp-ut
    $<TARGET_OBJECTS:tp-lib>
    test/test_Main.cpp
    test/test_MessageQueue.cpp
    test/test_PI.cpp
    test/test_Thread.cpp
    test/test_ThreadPool.cpp)


FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)

SET(DOXYGEN_INPUT Doxyfile)
SET(DOXYGEN_OUTPUT html)

ADD_CUSTOM_COMMAND(
  OUTPUT ${DOXYGEN_OUTPUT}
  COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation..."
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT}
  COMMAND ${CMAKE_COMMAND} -E echo "Done."
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${DOXYGEN_INPUT})

ADD_CUSTOM_TARGET(doc ALL DEPENDS ${DOXYGEN_OUTPUT})

ENDIF(DOXYGEN_FOUND)