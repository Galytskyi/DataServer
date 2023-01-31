#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QTcpServer>

#include "ClientSocket.h"
#include "DataDevice.h"
#include "Options.h"

// ==============================================================================================

class ServerSocket : public QTcpServer
{
	Q_OBJECT

public:

	explicit ServerSocket();
	virtual ~ServerSocket() override;

public:

	void setServerOption(const ServerOption& option) { m_serverOption = option; }

	void start();
	void stop();

private:

	ServerOption m_serverOption;

	QTimer m_tryListenTimer;

	std::map<quint64, ClientSocket*> m_clientSocketMap;

protected:

	void incomingConnection(qintptr socketDescriptor) override;

signals:

	void serverStarted();
	void serverStoped();

	void deviceConnected(DataDevice* pDevice);
	void deviceDisconnected(quint64 imei);
	void deviceCountChanged(int count);

	void deviceStateChanged(quint64 imei);
	void deviceReceivingData(quint64 imei);
	void managedPacketReceived(int requestType, DataDevice* pDevice, const QByteArray& data);
	void dataPacketReceived(DataDevice* pDevice, const QByteArray& data);

public slots:

	void onTryListen();
	void onRequestPicture(const std::vector<quint64>& imeiList);
	void onRequestDeviceSetting(quint64 imei, int brightness);

	void onDeviceInfoReceived(ClientSocket* pClientSocket);
	void onDeviceStateChanged(quint64 imei);
	void onDeviceReceivingData(quint64 imei);
	void onManagedPacketReceived(int requestType, DataDevice* pDataDevice, const QByteArray& data);
	void onDataPacketReceived(DataDevice* pDataDevice, const QByteArray& data);

	void onClientSocketDisconnected();
};

// ==============================================================================================

#endif // SERVERSOCKET_H
