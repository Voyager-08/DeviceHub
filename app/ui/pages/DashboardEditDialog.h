#pragma once

#include "devicehub/Dashboard.h"

#include <QDialog>
#include <QList>
#include <QStringList>

class QListWidget;

namespace devicehub {

// 仪表盘编辑所需的可用项
struct DashboardItemEntry {
    QString key;
    QString deviceId;
    QString itemId;
    QString deviceName;
    DashboardItemSpec spec;
};

// 编辑仪表盘:拖动排序当前卡片,从可用项中添加,或移除
class DashboardEditDialog : public QDialog
{
    Q_OBJECT

public:
    DashboardEditDialog(const QList<DashboardItemEntry> &entries,
                        const QStringList &visibleKeys,
                        QWidget *parent = nullptr);

    QStringList visibleKeys() const;

private:
    void fillLists();
    void addChecked();
    void removeCurrent();

    QList<DashboardItemEntry> m_entries;
    QStringList m_initialVisibleKeys;
    QListWidget *m_visibleList = nullptr;
    QListWidget *m_availableList = nullptr;
};

} // namespace devicehub
