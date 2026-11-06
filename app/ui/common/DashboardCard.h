#pragma once

#include "devicehub/Dashboard.h"

#include <QVariant>
#include <QWidget>

class QLabel;
class QToolButton;
class QVBoxLayout;

namespace devicehub {

// 仪表盘卡片基类:统一标题、边框、在线状态与编辑模式
class DashboardCard : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardCard(DashboardItemSpec spec, QWidget *parent = nullptr);
    ~DashboardCard() override;

    QString itemId() const { return m_spec.id; }
    const DashboardItemSpec &spec() const { return m_spec; }

    virtual void setValue(const QVariant &value) = 0;
    virtual void setOnline(bool online);
    bool isOnline() const { return m_online; }

    // 编辑模式显示右上角移除按钮
    void setEditMode(bool edit);

signals:
    void removeRequested(const QString &itemId);

protected:
    QVBoxLayout *contentLayout() const { return m_contentLayout; }
    void paintEvent(QPaintEvent *event) override;

private:
    DashboardItemSpec m_spec;
    QVBoxLayout *m_contentLayout = nullptr;
    QLabel *m_offlineLabel = nullptr;
    QToolButton *m_removeButton = nullptr;
    bool m_online = true;
    bool m_editMode = false;
};

} // namespace devicehub
