# 核心接口层:静态库 devicehub_core,只依赖 Qt6::Core
add_library(devicehub_core STATIC
    include/devicehub/Version.h
    include/devicehub/Types.h
    include/devicehub/Dashboard.h
    include/devicehub/ITransport.h
    include/devicehub/IDevice.h
    include/devicehub/ICapability.h
    include/devicehub/IPlugin.h
    include/devicehub/capabilities/ITerminal.h
    include/devicehub/capabilities/ICommand.h
    include/devicehub/capabilities/IFileTransfer.h
    include/devicehub/capabilities/IMonitor.h
    include/devicehub/capabilities/IRegister.h
    include/devicehub/capabilities/IMotion.h
    src/Version.cpp
    src/Types.cpp
)

target_include_directories(devicehub_core
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# 接口里有 QWidget/QIcon,core 需要链接 Qt Widgets
target_link_libraries(devicehub_core
    PUBLIC
        Qt6::Core
        Qt6::Widgets
)

target_compile_features(devicehub_core PUBLIC cxx_std_17)

target_compile_definitions(devicehub_core PRIVATE
    DEVICEHUB_VERSION="${PROJECT_VERSION}"
)
