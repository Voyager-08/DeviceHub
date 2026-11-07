# 应用层逻辑抽成静态库,方便单元测试直接链接
add_library(devicehub_app STATIC
    manager/PluginManager.h
    manager/PluginManager.cpp
    manager/DeviceManager.h
    manager/DeviceManager.cpp
    manager/SessionManager.h
    manager/SessionManager.cpp
    storage/ConfigStore.h
    storage/ConfigStore.cpp
    ui/shell/NavigationBar.h
    ui/shell/NavigationBar.cpp
    ui/shell/MainWindow.h
    ui/shell/MainWindow.cpp
    ui/common/DashboardCard.h
    ui/common/DashboardCard.cpp
    ui/common/ValueCard.h
    ui/common/ValueCard.cpp
    ui/common/GaugeCard.h
    ui/common/GaugeCard.cpp
    ui/common/TrendCard.h
    ui/common/TrendCard.cpp
    ui/common/StatusCard.h
    ui/common/StatusCard.cpp
    ui/common/TextCard.h
    ui/common/TextCard.cpp
    ui/common/CustomCard.h
    ui/common/CustomCard.cpp
    ui/pages/DashboardPage.h
    ui/pages/DashboardPage.cpp
    ui/pages/DashboardEditDialog.h
    ui/pages/DashboardEditDialog.cpp
    ui/pages/DevicesPage.h
    ui/pages/DevicesPage.cpp
    ui/pages/NewDeviceDialog.h
    ui/pages/NewDeviceDialog.cpp
    ui/pages/PluginsPage.h
    ui/pages/PluginsPage.cpp
    ui/pages/SettingsPage.h
    ui/pages/SettingsPage.cpp
)

target_include_directories(devicehub_app
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(devicehub_app
    PUBLIC
        devicehub_core
        Qt6::Widgets
)

# 主程序:只包含入口
qt_add_resources(APP_RESOURCES
    resources/resources.qrc
)

add_executable(devicehub
    main.cpp
    ${APP_RESOURCES}
)

target_link_libraries(devicehub
    PRIVATE devicehub_app
)

if(WIN32)
    set_target_properties(devicehub PROPERTIES WIN32_EXECUTABLE TRUE)
endif()