#pragma once

#include <QString>
#include <QStringList>
#include <QVariant>

namespace devicehub {

// 连接状态
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting,
    Error
};

// 统一错误码,具体协议的错误必须映射到这里
enum class ErrorCode {
    None,
    Unknown,
    Timeout,
    AuthFailed,
    HostKeyRejected,
    NetworkError,
    ProtocolError,
    NotSupported
};

// 统一错误信息
struct ErrorInfo {
    ErrorCode code = ErrorCode::None;
    QString message;
};

// 描述一个连接参数,用于自动生成表单
struct ParamSpec {
    QString key;
    QString label;
    QString type;               // string | int | bool | enum | password | file
    QVariant defaultValue;
    QStringList options;        // type 为 enum 时使用
    bool required = false;
};

// 连接状态的调试字符串
QString connectionStateToString(ConnectionState state);

// 注册跨线程信号槽需要的元类型,应用启动时调用一次
void registerMetaTypes();

} // namespace devicehub

Q_DECLARE_METATYPE(devicehub::ConnectionState)
Q_DECLARE_METATYPE(devicehub::ErrorCode)
Q_DECLARE_METATYPE(devicehub::ErrorInfo)
Q_DECLARE_METATYPE(devicehub::ParamSpec)
