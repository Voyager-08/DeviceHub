#include "SessionManager.h"

#include "devicehub/IDevice.h"
#include "manager/DeviceManager.h"

namespace devicehub {

namespace {

void setError(QString *errorMessage, const QString &message)
{
    if (errorMessage)
        *errorMessage = message;
}

} // namespace

SessionManager::SessionManager(DeviceManager *deviceManager, QObject *parent)
    : QObject(parent)
    , m_deviceManager(deviceManager)
{
}

SessionManager::~SessionManager() = default;

IDevice *SessionManager::device(const QString &id) const
{
    return m_devices.value(id, nullptr);
}

QStringList SessionManager::deviceIds() const
{
    return m_devices.keys();
}

IDevice *SessionManager::ensureDevice(const QString &id, QString *errorMessage)
{
    setError(errorMessage, {});

    if (IDevice *existing = m_devices.value(id, nullptr))
        return existing;

    if (!m_deviceManager) {
        setError(errorMessage, QStringLiteral("device manager is not available"));
        return nullptr;
    }

    IDevice *device = m_deviceManager->createDevice(id, this, errorMessage);
    if (!device)
        return nullptr;

    m_devices.insert(id, device);

    connect(device, &IDevice::stateChanged, this,
            [this, id](ConnectionState state) {
                emit deviceStateChanged(id, state);
            });
    connect(device, &IDevice::errorOccurred, this,
            [this, id](const ErrorInfo &error) {
                emit deviceError(id, error);
            });
    connect(device, &IDevice::dashboardValueChanged, this,
            [this, id](const QString &itemId, const QVariant &value) {
                emit deviceDashboardValueChanged(id, itemId, value);
            });
    connect(device, &IDevice::dashboardItemsChanged, this,
            [this, id]() { emit deviceDashboardItemsChanged(id); });

    emit deviceOpened(id);
    return device;
}

IDevice *SessionManager::openDevice(const QString &id, QString *errorMessage)
{
    IDevice *device = ensureDevice(id, errorMessage);
    if (!device)
        return nullptr;

    const ConnectionState state = device->state();
    if (state == ConnectionState::Disconnected || state == ConnectionState::Error)
        device->connectDevice();

    return device;
}

void SessionManager::connectDevice(const QString &id)
{
    if (IDevice *device = m_devices.value(id, nullptr)) {
        const ConnectionState state = device->state();
        if (state == ConnectionState::Disconnected
            || state == ConnectionState::Error) {
            device->connectDevice();
        }
    }
}

void SessionManager::disconnectDevice(const QString &id)
{
    if (IDevice *device = m_devices.value(id, nullptr))
        device->disconnectDevice();
}

void SessionManager::closeDevice(const QString &id)
{
    IDevice *device = m_devices.take(id);
    if (!device)
        return;

    device->disconnectDevice();
    device->deleteLater();
    emit deviceClosed(id);
}

void SessionManager::closeAll()
{
    const QStringList ids = m_devices.keys();
    for (const QString &id : ids)
        closeDevice(id);
}

} // namespace devicehub
