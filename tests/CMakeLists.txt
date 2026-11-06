find_package(Qt6 REQUIRED COMPONENTS Test)

set(DEVICEHUB_TEST_PLUGIN_DIR "${CMAKE_BINARY_DIR}/test-plugins")

# 测试插件统一输出到 test-plugins,避免污染主程序的 plugins 目录
function(devicehub_test_plugin target)
    set_target_properties(${target} PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY "${DEVICEHUB_TEST_PLUGIN_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${DEVICEHUB_TEST_PLUGIN_DIR}"
    )
    foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        set_target_properties(${target} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY_${config} "${DEVICEHUB_TEST_PLUGIN_DIR}"
            RUNTIME_OUTPUT_DIRECTORY_${config} "${DEVICEHUB_TEST_PLUGIN_DIR}"
        )
    endforeach()
endfunction()

# Mock 插件源码与开发版共用 plugins/mock
set(DEVICEHUB_MOCK_PLUGIN_DIR "${CMAKE_SOURCE_DIR}/plugins/mock")

# 正常插件
add_library(devicehub_mock_plugin MODULE
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockPlugin.h"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockPlugin.cpp"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/metadata.json"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockTransport.h"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockTransport.cpp"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockTerminal.h"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockTerminal.cpp"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockDevice.h"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockDevice.cpp"
)
target_include_directories(devicehub_mock_plugin PRIVATE
    "${DEVICEHUB_MOCK_PLUGIN_DIR}"
)
target_link_libraries(devicehub_mock_plugin PRIVATE
    devicehub_core
    Qt6::Widgets
)
devicehub_test_plugin(devicehub_mock_plugin)

# API 版本不兼容的插件
add_library(devicehub_bad_version_plugin MODULE
    mock/plugin/BadVersionPlugin.h
    mock/plugin/BadVersionPlugin.cpp
    mock/plugin/metadata_bad.json
)
target_include_directories(devicehub_bad_version_plugin PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/mock/plugin"
)
target_link_libraries(devicehub_bad_version_plugin PRIVATE
    devicehub_core
    Qt6::Core
)
devicehub_test_plugin(devicehub_bad_version_plugin)

# core 单元测试
add_executable(devicehub_tests
    tst_core.cpp
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockTransport.h"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockTransport.cpp"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockTerminal.h"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockTerminal.cpp"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockDevice.h"
    "${DEVICEHUB_MOCK_PLUGIN_DIR}/MockDevice.cpp"
)
target_include_directories(devicehub_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/plugins"
)
target_link_libraries(devicehub_tests PRIVATE
    devicehub_core
    Qt6::Test
)
add_test(NAME devicehub_tests COMMAND devicehub_tests)

# 插件管理器测试
add_executable(devicehub_plugin_tests
    tst_plugin_manager.cpp
)
target_compile_definitions(devicehub_plugin_tests PRIVATE
    DEVICEHUB_TEST_PLUGIN_DIR="${DEVICEHUB_TEST_PLUGIN_DIR}"
)
target_link_libraries(devicehub_plugin_tests PRIVATE
    devicehub_app
    Qt6::Test
)
add_dependencies(devicehub_plugin_tests
    devicehub_mock_plugin
    devicehub_bad_version_plugin
)
add_test(NAME devicehub_plugin_tests COMMAND devicehub_plugin_tests)

# 设备配置与 DeviceManager 测试
add_executable(devicehub_device_tests
    tst_device_manager.cpp
)
target_compile_definitions(devicehub_device_tests PRIVATE
    DEVICEHUB_TEST_PLUGIN_DIR="${DEVICEHUB_TEST_PLUGIN_DIR}"
    DEVICEHUB_TEST_TEMP_DIR="${CMAKE_CURRENT_BINARY_DIR}/tmp"
)
target_link_libraries(devicehub_device_tests PRIVATE
    devicehub_app
    Qt6::Test
)
add_dependencies(devicehub_device_tests
    devicehub_mock_plugin
)
add_test(NAME devicehub_device_tests COMMAND devicehub_device_tests)

# 设备页 GUI 集成测试(offscreen)
add_executable(devicehub_devices_tests
    tst_devices_page.cpp
)
target_compile_definitions(devicehub_devices_tests PRIVATE
    DEVICEHUB_TEST_PLUGIN_DIR="${DEVICEHUB_TEST_PLUGIN_DIR}"
    DEVICEHUB_TEST_TEMP_DIR="${CMAKE_CURRENT_BINARY_DIR}/tmp"
)
target_link_libraries(devicehub_devices_tests PRIVATE
    devicehub_app
    Qt6::Test
)
add_dependencies(devicehub_devices_tests
    devicehub_mock_plugin
)
add_test(NAME devicehub_devices_tests COMMAND devicehub_devices_tests)
set_tests_properties(devicehub_devices_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# 仪表盘内置卡片测试
add_executable(devicehub_cards_tests
    tst_cards.cpp
)
target_link_libraries(devicehub_cards_tests PRIVATE
    devicehub_app
    Qt6::Test
)
add_test(NAME devicehub_cards_tests COMMAND devicehub_cards_tests)
set_tests_properties(devicehub_cards_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# 仪表盘页集成测试(offscreen)
add_executable(devicehub_dashboard_tests
    tst_dashboard_page.cpp
)
target_compile_definitions(devicehub_dashboard_tests PRIVATE
    DEVICEHUB_TEST_PLUGIN_DIR="${DEVICEHUB_TEST_PLUGIN_DIR}"
    DEVICEHUB_TEST_TEMP_DIR="${CMAKE_CURRENT_BINARY_DIR}/tmp"
)
target_link_libraries(devicehub_dashboard_tests PRIVATE
    devicehub_app
    Qt6::Test
)
add_dependencies(devicehub_dashboard_tests
    devicehub_mock_plugin
)
add_test(NAME devicehub_dashboard_tests COMMAND devicehub_dashboard_tests)
set_tests_properties(devicehub_dashboard_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
