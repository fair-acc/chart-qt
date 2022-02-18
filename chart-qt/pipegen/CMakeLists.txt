cmake_minimum_required(VERSION 3.21)
project(pipegen)

find_package(Qt6 COMPONENTS Core Gui)

add_executable(pipegen main.cpp)

target_include_directories(pipegen PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS})
set_property(TARGET pipegen PROPERTY CXX_STANDARD 20)
target_link_libraries(pipegen Qt::Core Qt::Gui)

install(TARGETS pipegen)
