cmake_minimum_required(VERSION 3.10)
project(forte)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(pugixml)

add_executable(forte
    main.cpp
    server.cpp
    command_handler.cpp
)

target_include_directories(forte PRIVATE pugixml/include pugixml/src)
target_link_libraries(forte PRIVATE pugixml)

if(WIN32)
    target_link_libraries(forte PRIVATE ws2_32)
    
    target_compile_definitions(forte PRIVATE _CRT_SECURE_NO_WARNINGS)
    
    message(STATUS "?? Windows configuration applied")
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(forte PRIVATE m)
    message(STATUS "?? Linux configuration applied")
endif()
