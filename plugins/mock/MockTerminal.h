#pragma once

#include "devicehub/capabilities/ITerminal.h"

namespace devicehub::test {

// 假终端能力:启动后回复一句欢迎语,把输入原样回显
class MockTerminal : public ITerminal
{
    Q_OBJECT

public:
    explicit MockTerminal(QObject *parent = nullptr);
    ~MockTerminal() override = default;

    void start(int cols, int rows) override;
    void resize(int cols, int rows) override;
    void sendInput(const QByteArray &data) override;
};

} // namespace devicehub::test
