#include "MockTerminal.h"

namespace devicehub::test {

MockTerminal::MockTerminal(QObject *parent)
    : ITerminal(parent)
{
}

void MockTerminal::start(int cols, int rows)
{
    Q_UNUSED(cols)
    Q_UNUSED(rows)
    emit outputReceived(QByteArrayLiteral("Mock terminal ready\r\n$ "));
}

void MockTerminal::resize(int cols, int rows)
{
    Q_UNUSED(cols)
    Q_UNUSED(rows)
}

void MockTerminal::sendInput(const QByteArray &data)
{
    emit outputReceived(data);

    if (data.contains("exit"))
        emit finished();
}

} // namespace devicehub::test
