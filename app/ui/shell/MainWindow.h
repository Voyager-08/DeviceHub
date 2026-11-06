#pragma once

#include <QMainWindow>

class QStackedWidget;

namespace devicehub {

class DeviceManager;
class NavigationBar;
class PluginManager;
class SessionManager;

// 主窗口外壳:左侧导航栏 + 右侧四页内容区
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    MainWindow(PluginManager *pluginManager,
               DeviceManager *deviceManager,
               SessionManager *sessionManager,
               QWidget *parent = nullptr);
    ~MainWindow() override;

    NavigationBar *navigationBar() const { return m_navigation; }
    QStackedWidget *pageStack() const { return m_stack; }

private:
    NavigationBar *m_navigation = nullptr;
    QStackedWidget *m_stack = nullptr;
};

} // namespace devicehub
