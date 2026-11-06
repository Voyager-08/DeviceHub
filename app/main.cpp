#include "ui/shell/MainWindow.h"

#include "devicehub/Types.h"
#include "manager/DeviceManager.h"
#include "manager/PluginManager.h"
#include "manager/SessionManager.h"

#include <QApplication>
#include <QDebug>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("DeviceHub"));
    QCoreApplication::setApplicationName(QStringLiteral("DeviceHub"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    devicehub::registerMetaTypes();

    // 插件目录固定为 <应用目录>/plugins
    const QString pluginDir =
        QDir(QCoreApplication::applicationDirPath())
            .filePath(QStringLiteral("plugins"));

    devicehub::PluginManager pluginManager;
    pluginManager.setPluginDirectory(pluginDir);

    devicehub::DeviceManager deviceManager(&pluginManager);

    QString configError;
    if (!deviceManager.load(&configError))
        qWarning().noquote() << "Failed to load device config:" << configError;

    const QHash<QString, bool> enabledStates =
        deviceManager.configStore().loadPluginEnabledStates();
    if (!enabledStates.isEmpty())
        pluginManager.setPluginEnabledStates(enabledStates);

    pluginManager.loadAll();

    devicehub::SessionManager sessionManager(&deviceManager);

    devicehub::MainWindow window(&pluginManager, &deviceManager,
                                 &sessionManager);
    window.show();

    return app.exec();
}
