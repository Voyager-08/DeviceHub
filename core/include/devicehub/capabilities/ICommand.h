#pragma once

#include "devicehub/ICapability.h"

#include <QByteArray>
#include <QString>

namespace devicehub {

// 单次命令执行能力
class ICommand : public ICapability
{
    Q_OBJECT

public:
    explicit ICommand(QObject *parent = nullptr)
        : ICapability(parent)
    {
    }

    ~ICommand() override = default;

    QString capabilityId() const override { return QStringLiteral("command"); }

    virtual void execute(const QString &command) = 0;
    virtual void cancel() = 0;

signals:
    void outputReceived(const QByteArray &data);
    void finished(int exitCode);
};

} // namespace devicehub
