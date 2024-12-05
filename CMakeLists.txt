cmake_minimum_required(VERSION 3.29)
project(ttimer C)

set(CMAKE_C_STANDARD 11)

# 编译cmd版
add_executable(ttimer
        main.c
        cJSON.c
        ttimer.c
        file.c
        file.h
        ttimer.h
        cJSON.h)


# 编译GUI版
## 查找 GTK 包
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
## 包含目录
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
## 添加可执行文件
add_executable(ttimer_gui
        main_gui.c
        gui.c
        cJSON.c
        ttimer.c
        file.c
        gui.h
        file.h
        ttimer.h
        cJSON.h)
## 链接库
target_link_libraries(ttimer_gui ${GTK3_LIBRARIES})
set_target_properties(ttimer_gui PROPERTIES
        LINK_FLAGS "-mwindows"
)
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
