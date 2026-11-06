#pragma once

#include <QMetaType>
#include <QSize>
#include <QString>

namespace devicehub {

// 仪表盘卡片类型
enum class DashboardKind {
    Value,      // 数值卡:大号数字 + 单位
    Gauge,      // 仪表盘/进度环,有最小最大值
    Trend,      // 折线趋势图,保留最近 N 个点
    Status,     // 状态灯:正常/警告/错误
    Text,       // 文本
    Custom      // 插件自带控件
};

// 仪表盘项由插件声明,仪表盘页只负责摆放与渲染
struct DashboardItemSpec {
    QString id;                   // 同一设备内唯一且稳定,如 cpu_usage
    QString title;                // CPU 使用率
    QString description;          // 「添加卡片」列表里的说明
    DashboardKind kind = DashboardKind::Value;
    QString unit;                 // %
    double minValue = 0;          // Gauge 用
    double maxValue = 100;
    QSize defaultSize{1, 1};      // 占几格
    bool defaultVisible = true;   // 默认是否显示在仪表盘上
};

} // namespace devicehub

Q_DECLARE_METATYPE(devicehub::DashboardItemSpec)
