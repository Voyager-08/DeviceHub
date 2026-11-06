# 插件统一输出到 ${CMAKE_BINARY_DIR}/plugins

# Mock 插件只在开发构建中编译
option(DEVICEHUB_BUILD_MOCK_PLUGIN "Build the mock plugin for development" ON)

if(DEVICEHUB_BUILD_MOCK_PLUGIN)
    add_subdirectory(mock)
endif()

add_subdirectory(ssh)
