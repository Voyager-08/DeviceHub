# DeviceHub

Qt6/C++ 桌面软件,宗旨是「万物皆可像 Linux 设备一样被连接」。

统一用「传输层 + 设备 + 能力」三个概念描述任意设备;主窗口外壳固定,设备界面、仪表盘内容、插件设置页全部由插件提供。当前阶段只实现 Linux(SSH + 交互式终端),后续的串口、TCP、USB、Modbus、机械臂等全部以插件形式加入。

## 目录结构

```
DeviceHub/
├─ CMakeLists.txt
├─ CMakePresets.json
├─ core/          # 静态库 devicehub_core,只放接口与公共类型
├─ app/           # 主程序:管理器、存储、UI
├─ plugins/       # 插件,每个插件输出为独立动态库
│  ├─ mock/       # 开发用假插件(可通过 CMake 选项关闭)
│  └─ ssh/
├─ tests/         # 单元测试
└─ docs/
```

依赖方向:`plugins → core ← app`。

## 构建依赖

- CMake 3.21+
- C++17 编译器(Windows 用 MinGW 或 MSVC,Linux 用 GCC/Clang)
- Qt 6(Core、Widgets;后续步骤还需要 Network、SerialPort、SerialBus)
- libssh2(仅 SSH 插件使用,后续步骤引入)

### Qt6

Windows 下安装 Qt 6.5+ 的 MinGW 或 MSVC 套件即可。把 `QTDIR` 环境变量指向 Qt 套件目录,例如:

```
set QTDIR=D:\QT\QT_x\6.5.3\mingw_64
```

Linux 下用系统包:

```
sudo apt install qt6-base-dev qt6-serialport-dev
```

### libssh2

通过 vcpkg:

```
vcpkg install libssh2
```

或 Linux 系统包:

```
sudo apt install libssh2-1-dev
```

## 构建

Windows(MinGW):

```
cmake --preset windows-debug -DCMAKE_PREFIX_PATH=D:/QT/QT_x/6.5.3/mingw_64
cmake --build --preset windows-debug
```

Linux:

```
cmake --preset linux-debug
cmake --build --preset linux-debug
```

产物:

- 主程序:`build/<preset>/bin/devicehub.exe`(Linux 为 `devicehub`)
- 插件:`build/<preset>/plugins/`

## 开发进度

已按任务书 v2 完成基础框架与补丁:

- [x] CMake 工程骨架:core、app、plugins/mock、plugins/ssh
- [x] core 接口:Types、Dashboard、ITransport、IDevice、ICapability、IPlugin、六种能力
- [x] Mock 插件:假传输、假终端、四个仪表盘项(每秒更新)、设备界面、自定义卡片、设置页
- [x] PluginManager:扫描、加载、API 版本检查、加载状态与失败原因、插件启用状态
- [x] ConfigStore:设备列表、仪表盘布局、插件启用状态、应用设置(主题/语言)
- [x] DeviceManager:设备增删改查、按配置通过插件创建设备
- [x] v2 第 5 步:主窗口外壳(导航栏 + 四个空白页面切换)
- [x] v2 第 6 步:设备页(分组列表、新建设备表单、加载 Mock 插件界面、右键菜单)
- [x] v2 第 7 步:仪表盘页(五种内置卡片、自定义卡片、200ms 合并刷新、断线变灰、编辑与布局保存)
- [ ] v2 第 8 步:插件页与设置页(含主题切换与关于)

Mock 插件默认参与构建,发布时可关闭:

```
cmake --preset windows-release -DDEVICEHUB_BUILD_MOCK_PLUGIN=OFF
```
