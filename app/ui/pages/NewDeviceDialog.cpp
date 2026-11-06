#include "NewDeviceDialog.h"

#include "devicehub/IPlugin.h"
#include "manager/PluginManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <limits>

namespace devicehub {

namespace {

void applyValue(QWidget *editor, const QVariant &value)
{
    if (auto *edit = qobject_cast<QLineEdit *>(editor)) {
        edit->setText(value.toString());
    } else if (auto *spin = qobject_cast<QSpinBox *>(editor)) {
        spin->setValue(value.toInt());
    } else if (auto *check = qobject_cast<QCheckBox *>(editor)) {
        check->setChecked(value.toBool());
    } else if (auto *combo = qobject_cast<QComboBox *>(editor)) {
        const int index = combo->findText(value.toString());
        if (index >= 0)
            combo->setCurrentIndex(index);
    }
}

QString editorText(QWidget *editor)
{
    if (auto *edit = qobject_cast<QLineEdit *>(editor))
        return edit->text().trimmed();
    if (auto *combo = qobject_cast<QComboBox *>(editor))
        return combo->currentText().trimmed();
    return {};
}

} // namespace

NewDeviceDialog::NewDeviceDialog(PluginManager *pluginManager, QWidget *parent)
    : NewDeviceDialog(pluginManager, DeviceConfig{}, parent)
{
}

NewDeviceDialog::NewDeviceDialog(PluginManager *pluginManager,
                                 const DeviceConfig &config,
                                 QWidget *parent)
    : QDialog(parent)
    , m_pluginManager(pluginManager)
    , m_config(config)
    , m_initialParams(config.params)
{
    setupUi();
    setWindowTitle(m_config.id.isEmpty() ? QStringLiteral("新建设备")
                                         : QStringLiteral("编辑设备"));

    populatePlugins();
    connect(m_pluginCombo, &QComboBox::currentIndexChanged,
            this, &NewDeviceDialog::rebuildForm);

    m_nameEdit->setText(m_config.name);
    m_groupEdit->setText(m_config.group);
    rebuildForm();
}

void NewDeviceDialog::setupUi()
{
    resize(480, 520);

    auto *root = new QVBoxLayout(this);

    auto *topForm = new QFormLayout();

    m_pluginCombo = new QComboBox(this);
    topForm->addRow(QStringLiteral("插件"), m_pluginCombo);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(QStringLiteral("例如:开发板 A"));
    topForm->addRow(QStringLiteral("名称"), m_nameEdit);

    m_groupEdit = new QLineEdit(this);
    m_groupEdit->setPlaceholderText(QStringLiteral("默认"));
    topForm->addRow(QStringLiteral("分组"), m_groupEdit);

    root->addLayout(topForm);

    m_paramsLayout = new QFormLayout();
    root->addLayout(m_paramsLayout);
    root->addStretch();

    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this, &NewDeviceDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
    root->addWidget(m_buttonBox);
}

void NewDeviceDialog::populatePlugins()
{
    m_pluginCombo->clear();

    QStringList pluginIds;
    if (m_pluginManager)
        pluginIds = m_pluginManager->pluginIds();
    pluginIds.sort();

    for (const QString &id : pluginIds) {
        IPlugin *plugin = m_pluginManager->plugin(id);
        if (!plugin)
            continue;
        m_pluginCombo->addItem(
            plugin->icon(),
            QStringLiteral("%1 (%2)").arg(plugin->displayName(), plugin->version()),
            id);
    }

    if (!m_config.pluginId.isEmpty()) {
        int index = m_pluginCombo->findData(m_config.pluginId);
        if (index < 0) {
            // 插件被禁用或加载失败时,仍然允许查看和修改配置
            m_pluginCombo->addItem(
                QStringLiteral("%1 (未加载)").arg(m_config.pluginId),
                m_config.pluginId);
            index = m_pluginCombo->count() - 1;
        }
        m_pluginCombo->setCurrentIndex(index);
    }

    if (m_pluginCombo->count() == 0) {
        m_pluginCombo->addItem(QStringLiteral("无可用插件"), QString());
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void NewDeviceDialog::rebuildForm()
{
    while (QLayoutItem *item = m_paramsLayout->takeAt(0)) {
        if (QWidget *widget = item->widget())
            delete widget;
        delete item;
    }
    m_editors.clear();
    m_specs.clear();

    if (!m_pluginManager)
        return;

    IPlugin *plugin = m_pluginManager->plugin(currentPluginId());
    if (!plugin)
        return;

    m_specs = plugin->paramSpecs();

    const bool useInitial = !m_config.pluginId.isEmpty()
        && currentPluginId() == m_config.pluginId;

    for (const ParamSpec &spec : m_specs) {
        QWidget *editor = nullptr;
        QWidget *field = nullptr;

        if (spec.type == QStringLiteral("int")) {
            auto *spin = new QSpinBox(this);
            spin->setRange(std::numeric_limits<int>::min(),
                           std::numeric_limits<int>::max());
            editor = spin;
            field = spin;
        } else if (spec.type == QStringLiteral("bool")) {
            auto *check = new QCheckBox(this);
            editor = check;
            field = check;
        } else if (spec.type == QStringLiteral("enum")) {
            auto *combo = new QComboBox(this);
            combo->addItems(spec.options);
            editor = combo;
            field = combo;
        } else if (spec.type == QStringLiteral("file")) {
            auto *container = new QWidget(this);
            auto *layout = new QHBoxLayout(container);
            layout->setContentsMargins(0, 0, 0, 0);

            auto *edit = new QLineEdit(container);
            auto *browse = new QPushButton(QStringLiteral("浏览…"), container);
            layout->addWidget(edit, 1);
            layout->addWidget(browse);

            connect(browse, &QPushButton::clicked, this, [this, edit]() {
                const QString file = QFileDialog::getOpenFileName(
                    this, QStringLiteral("选择文件"), edit->text());
                if (!file.isEmpty())
                    edit->setText(file);
            });

            editor = edit;
            field = container;
        } else {
            auto *edit = new QLineEdit(this);
            if (spec.type == QStringLiteral("password"))
                edit->setEchoMode(QLineEdit::Password);
            editor = edit;
            field = edit;
        }

        const QVariant value = useInitial
            ? m_initialParams.value(spec.key, spec.defaultValue)
            : spec.defaultValue;
        applyValue(editor, value);

        QString label = spec.label.isEmpty() ? spec.key : spec.label;
        if (spec.required)
            label += QStringLiteral(" *");

        m_paramsLayout->addRow(label, field);
        m_editors.insert(spec.key, editor);
    }
}

QString NewDeviceDialog::currentPluginId() const
{
    return m_pluginCombo ? m_pluginCombo->currentData().toString() : QString();
}

QVariantMap NewDeviceDialog::collectParams() const
{
    QVariantMap params;

    for (const ParamSpec &spec : m_specs) {
        // 密码默认不落盘,后续交给 CredentialStore
        if (spec.type == QStringLiteral("password"))
            continue;

        QWidget *editor = m_editors.value(spec.key, nullptr);
        if (!editor)
            continue;

        if (auto *check = qobject_cast<QCheckBox *>(editor)) {
            params.insert(spec.key, check->isChecked());
        } else if (auto *spin = qobject_cast<QSpinBox *>(editor)) {
            params.insert(spec.key, spin->value());
        } else if (auto *combo = qobject_cast<QComboBox *>(editor)) {
            if (!combo->currentText().isEmpty())
                params.insert(spec.key, combo->currentText());
        } else if (auto *edit = qobject_cast<QLineEdit *>(editor)) {
            if (!edit->text().isEmpty())
                params.insert(spec.key, edit->text());
        }
    }

    return params;
}

void NewDeviceDialog::accept()
{
    if (currentPluginId().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("无法保存"),
                             QStringLiteral("没有可用的插件。"));
        return;
    }

    const QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("无法保存"),
                             QStringLiteral("请填写设备名称。"));
        m_nameEdit->setFocus();
        return;
    }

    QStringList missing;
    for (const ParamSpec &spec : m_specs) {
        if (!spec.required)
            continue;

        QWidget *editor = m_editors.value(spec.key, nullptr);
        if (!editor)
            continue;

        if (auto *check = qobject_cast<QCheckBox *>(editor)) {
            Q_UNUSED(check)
            continue;
        }
        if (qobject_cast<QSpinBox *>(editor))
            continue;

        if (editorText(editor).isEmpty()) {
            missing.append(spec.label.isEmpty() ? spec.key : spec.label);
        }
    }

    if (!missing.isEmpty()) {
        QMessageBox::warning(
            this, QStringLiteral("无法保存"),
            QStringLiteral("请填写以下必填项:\n%1").arg(missing.join(QStringLiteral("\n"))));
        return;
    }

    m_config.name = name;
    m_config.pluginId = currentPluginId();

    const QString group = m_groupEdit->text().trimmed();
    m_config.group = group.isEmpty() ? QStringLiteral("默认") : group;
    m_config.params = collectParams();

    QDialog::accept();
}

} // namespace devicehub
