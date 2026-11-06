#pragma once

#include "devicehub/ICapability.h"

#include <QString>
#include <QtGlobal>

namespace devicehub {

// 文件传输能力
class IFileTransfer : public ICapability
{
    Q_OBJECT

public:
    explicit IFileTransfer(QObject *parent = nullptr)
        : ICapability(parent)
    {
    }

    ~IFileTransfer() override = default;

    QString capabilityId() const override { return QStringLiteral("filetransfer"); }

    virtual void upload(const QString &localPath, const QString &remotePath) = 0;
    virtual void download(const QString &remotePath, const QString &localPath) = 0;
    virtual void cancel() = 0;

signals:
    void progressChanged(qint64 transferred, qint64 total);
    void finished(bool ok);
};

} // namespace devicehub
