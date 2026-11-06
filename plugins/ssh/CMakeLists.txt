# SSH 插件:第 1 步仅生成空的动态插件模块,真正的实现见后续步骤
add_library(devicehub_plugin_ssh MODULE
    SshPlugin.h
    SshPlugin.cpp
    metadata.json
)

target_link_libraries(devicehub_plugin_ssh
    PRIVATE
        devicehub_core
        Qt6::Core
)

set_target_properties(devicehub_plugin_ssh PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${DEVICEHUB_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${DEVICEHUB_PLUGIN_DIR}"
)

# 多配置生成器也要输出到同一目录
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(devicehub_plugin_ssh PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY_${config} "${DEVICEHUB_PLUGIN_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_${config} "${DEVICEHUB_PLUGIN_DIR}"
    )
endforeach()
