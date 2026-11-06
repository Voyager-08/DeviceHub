#pragma once

#include "devicehub/Types.h"
#include "storage/ConfigStore.h"

#include <QDialog>
#include <QHash>

class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QLineEdit;

namespace devicehub {

class PluginManager;

// 新建 / 编辑设备对话框:表单由插件的 ParamSpec 自动生成
class NewDeviceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewDeviceDialog(PluginManager *pluginManager,
                             QWidget *parent = nullptr);
    NewDeviceDialog(PluginManager *pluginManager,
                    const DeviceConfig &config,
                    QWidget *parent = nullptr);

    DeviceConfig deviceConfig() const { return m_config; }

private:
    void setupUi();
    void populatePlugins();
    void rebuildForm();
    QString currentPluginId() const;
    QVariantMap collectParams() const;
    void accept() override;

    PluginManager *m_pluginManager = nullptr;
    DeviceConfig m_config;
    QVariantMap m_initialParams;
    QList<ParamSpec> m_specs;

    QComboBox *m_pluginCombo = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QLineEdit *m_groupEdit = nullptr;
    QFormLayout *m_paramsLayout = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
    QHash<QString, QWidget *> m_editors;
};

} // namespace devicehub
