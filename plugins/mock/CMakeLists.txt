# 开发用 Mock 插件:不连接任何真实设备,用于验证整个框架
add_library(devicehub_plugin_mock MODULE
    MockPlugin.h
    MockPlugin.cpp
    MockDevice.h
    MockDevice.cpp
    MockTransport.h
    MockTransport.cpp
    MockTerminal.h
    MockTerminal.cpp
    metadata.json
)

target_include_directories(devicehub_plugin_mock PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(devicehub_plugin_mock
    PRIVATE
        devicehub_core
        Qt6::Widgets
)

set_target_properties(devicehub_plugin_mock PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${DEVICEHUB_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${DEVICEHUB_PLUGIN_DIR}"
)

foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(devicehub_plugin_mock PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY_${config} "${DEVICEHUB_PLUGIN_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_${config} "${DEVICEHUB_PLUGIN_DIR}"
    )
endforeach()
