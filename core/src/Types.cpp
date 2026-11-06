#include "devicehub/Types.h"

#include "devicehub/Dashboard.h"

#include <QMetaType>

namespace devicehub {

QString connectionStateToString(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Disconnected:
        return QStringLiteral("Disconnected");
    case ConnectionState::Connecting:
        return QStringLiteral("Connecting");
    case ConnectionState::Connected:
        return QStringLiteral("Connected");
    case ConnectionState::Reconnecting:
        return QStringLiteral("Reconnecting");
    case ConnectionState::Error:
        return QStringLiteral("Error");
    }
    return QStringLiteral("Unknown");
}

void registerMetaTypes()
{
    qRegisterMetaType<ConnectionState>("devicehub::ConnectionState");
    qRegisterMetaType<ErrorCode>("devicehub::ErrorCode");
    qRegisterMetaType<ErrorInfo>("devicehub::ErrorInfo");
    qRegisterMetaType<ParamSpec>("devicehub::ParamSpec");
    qRegisterMetaType<DashboardItemSpec>("devicehub::DashboardItemSpec");
}

} // namespace devicehub
