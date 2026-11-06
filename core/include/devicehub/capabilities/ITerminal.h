#pragma once

#include "devicehub/ICapability.h"

#include <QByteArray>
#include <QString>

namespace devicehub {

// 交互式终端能力:输出含 ANSI 转义序列,原样转发给终端控件
class ITerminal : public ICapability
{
    Q_OBJECT

public:
    explicit ITerminal(QObject *parent = nullptr)
        : ICapability(parent)
    {
    }

    ~ITerminal() override = default;

    QString capabilityId() const override { return QStringLiteral("terminal"); }

    virtual void start(int cols, int rows) = 0;   // 申请 PTY 并打开 shell
    virtual void resize(int cols, int rows) = 0;
    virtual void sendInput(const QByteArray &data) = 0;

signals:
    void outputReceived(const QByteArray &data);
    void finished();
};

} // namespace devicehub
