#pragma once

#include <QList>
#include <QStringList>
#include <QWidget>

class QButtonGroup;
class QPushButton;
class QVBoxLayout;

namespace devicehub {

// 左侧导航栏:一组互斥的按钮,只负责发出页面索引
class NavigationBar : public QWidget
{
    Q_OBJECT

public:
    explicit NavigationBar(const QStringList &items, QWidget *parent = nullptr);
    ~NavigationBar() override;

    int currentIndex() const;
    int count() const;
    void setCurrentIndex(int index);

signals:
    void currentIndexChanged(int index);

private:
    QVBoxLayout *m_layout = nullptr;
    QButtonGroup *m_group = nullptr;
    QList<QPushButton *> m_buttons;
};

} // namespace devicehub
