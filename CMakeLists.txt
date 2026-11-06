cmake_minimum_required(VERSION 3.21)

project(DeviceHub
    VERSION 0.1.0
    DESCRIPTION "万物皆可像 Linux 设备一样被连接监控"
    LANGUAGES CXX)

# 统一 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 自动处理 moc/uic/rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 统一输出目录:可执行文件放 bin/,插件放 plugins/
set(DEVICEHUB_BIN_DIR    "${CMAKE_BINARY_DIR}/bin")
set(DEVICEHUB_PLUGIN_DIR "${CMAKE_BINARY_DIR}/plugins")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DEVICEHUB_BIN_DIR}")
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config} "${DEVICEHUB_BIN_DIR}")
endforeach()

if(NOT DEFINED CMAKE_PREFIX_PATH AND DEFINED ENV{QTDIR})
    set(CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets)

add_subdirectory(core)
add_subdirectory(app)
# 中间文件与插件输出目录分开,保证 plugins/ 下只有 .dll/.so
add_subdirectory(plugins "${CMAKE_BINARY_DIR}/plugins-build")

# 测试目录留待第 2 步接入,存在时再加入
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()
