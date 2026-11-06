#include "DashboardEditDialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSet>
#include <QVBoxLayout>

namespace devicehub {

namespace {

QString entryText(const DashboardItemEntry &entry)
{
    const QString device = entry.deviceName.isEmpty()
        ? entry.deviceId
        : entry.deviceName;
    return QStringLiteral("%1 · %2").arg(device, entry.spec.title);
}

QString entryToolTip(const DashboardItemEntry &entry)
{
    return entry.spec.description;
}

} // namespace

DashboardEditDialog::DashboardEditDialog(
    const QList<DashboardItemEntry> &entries,
    const QStringList &visibleKeys,
    QWidget *parent)
    : QDialog(parent)
    , m_entries(entries)
    , m_initialVisibleKeys(visibleKeys)
{
    setWindowTitle(QStringLiteral("编辑仪表盘"));
    resize(720, 460);

    auto *root = new QVBoxLayout(this);

    auto *columns = new QHBoxLayout();

    auto *visibleColumn = new QVBoxLayout();
    visibleColumn->addWidget(new QLabel(QStringLiteral("当前卡片(可拖动排序)"), this));
    m_visibleList = new QListWidget(this);
    m_visibleList->setDragDropMode(QAbstractItemView::InternalMove);
    m_visibleList->setDefaultDropAction(Qt::MoveAction);
    m_visibleList->setSelectionMode(QAbstractItemView::SingleSelection);
    visibleColumn->addWidget(m_visibleList, 1);

    auto *buttons = new QVBoxLayout();
    buttons->addStretch();
    auto *addButton = new QPushButton(QStringLiteral("添加选中 →"), this);
    auto *removeButton = new QPushButton(QStringLiteral("← 移除选中"), this);
    buttons->addWidget(addButton);
    buttons->addWidget(removeButton);
    buttons->addStretch();

    auto *availableColumn = new QVBoxLayout();
    availableColumn->addWidget(new QLabel(QStringLiteral("可用项"), this));
    m_availableList = new QListWidget(this);
    m_availableList->setSelectionMode(QAbstractItemView::SingleSelection);
    availableColumn->addWidget(m_availableList, 1);

    columns->addLayout(visibleColumn, 1);
    columns->addLayout(buttons);
    columns->addLayout(availableColumn, 1);
    root->addLayout(columns, 1);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttonBox);

    connect(addButton, &QPushButton::clicked, this, &DashboardEditDialog::addChecked);
    connect(removeButton, &QPushButton::clicked, this, &DashboardEditDialog::removeCurrent);

    fillLists();
}

void DashboardEditDialog::fillLists()
{
    m_visibleList->clear();
    m_availableList->clear();

    const QSet<QString> visible(m_initialVisibleKeys.begin(),
                                m_initialVisibleKeys.end());

    for (const QString &key : m_initialVisibleKeys) {
        for (const DashboardItemEntry &entry : m_entries) {
            if (entry.key != key)
                continue;
            auto *item = new QListWidgetItem(entryText(entry), m_visibleList);
            item->setData(Qt::UserRole, entry.key);
            item->setToolTip(entryToolTip(entry));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled
                           | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
            break;
        }
    }

    for (const DashboardItemEntry &entry : m_entries) {
        if (visible.contains(entry.key))
            continue;

        auto *item = new QListWidgetItem(entryText(entry), m_availableList);
        item->setData(Qt::UserRole, entry.key);
        item->setToolTip(entryToolTip(entry));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled
                       | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
}

void DashboardEditDialog::addChecked()
{
    for (int i = m_availableList->count() - 1; i >= 0; --i) {
        QListWidgetItem *item = m_availableList->item(i);
        if (item->checkState() != Qt::Checked)
            continue;

        const QString key = item->data(Qt::UserRole).toString();
        const QString text = item->text();
        const QString toolTip = item->toolTip();
        delete m_availableList->takeItem(i);

        auto *added = new QListWidgetItem(text, m_visibleList);
        added->setData(Qt::UserRole, key);
        added->setToolTip(toolTip);
        added->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled
                        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    }
}

void DashboardEditDialog::removeCurrent()
{
    QListWidgetItem *item = m_visibleList->currentItem();
    if (!item)
        return;

    const QString key = item->data(Qt::UserRole).toString();
    const QString text = item->text();
    const QString toolTip = item->toolTip();
    delete m_visibleList->takeItem(m_visibleList->row(item));

    auto *available = new QListWidgetItem(text, m_availableList);
    available->setData(Qt::UserRole, key);
    available->setToolTip(toolTip);
    available->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled
                        | Qt::ItemIsUserCheckable);
    available->setCheckState(Qt::Unchecked);
}

QStringList DashboardEditDialog::visibleKeys() const
{
    QStringList keys;
    if (!m_visibleList)
        return m_initialVisibleKeys;

    for (int i = 0; i < m_visibleList->count(); ++i)
        keys.append(m_visibleList->item(i)->data(Qt::UserRole).toString());
    return keys;
}

} // namespace devicehub
