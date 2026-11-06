#include "MainWindow.h"

#include "NavigationBar.h"

#include "ui/pages/DashboardPage.h"
#include "ui/pages/DevicesPage.h"
#include "ui/pages/PluginsPage.h"
#include "ui/pages/SettingsPage.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QStatusBar>
#include <QWidget>

namespace devicehub {

MainWindow::MainWindow(QWidget *parent)
    : MainWindow(nullptr, nullptr, nullptr, parent)
{
}

MainWindow::MainWindow(PluginManager *pluginManager,
                       DeviceManager *deviceManager,
                       SessionManager *sessionManager,
                       QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("DeviceHub"));
    resize(1200, 780);

    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_navigation = new NavigationBar(
        {QStringLiteral("仪表盘"),
         QStringLiteral("设备"),
         QStringLiteral("插件"),
         QStringLiteral("设置")},
        central);

    m_stack = new QStackedWidget(central);
    m_stack->addWidget(
        new DashboardPage(pluginManager, deviceManager, sessionManager, m_stack));
    m_stack->addWidget(
        new DevicesPage(pluginManager, deviceManager, sessionManager, m_stack));
    m_stack->addWidget(new PluginsPage(m_stack));
    m_stack->addWidget(new SettingsPage(m_stack));

    layout->addWidget(m_navigation);
    layout->addWidget(m_stack, 1);

    setCentralWidget(central);

    connect(m_navigation, &NavigationBar::currentIndexChanged,
            m_stack, &QStackedWidget::setCurrentIndex);
    m_navigation->setCurrentIndex(0);
    m_stack->setCurrentIndex(0);

    statusBar()->showMessage(QStringLiteral("就绪"));
}

MainWindow::~MainWindow() = default;

} // namespace devicehub
