#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

#include "DataDevice.h"

// ==============================================================================================

const int WAIT_RESPONSE_TIME = 8000; // 8 sec

// ==============================================================================================

class ClientSocket : public QObject
{
	Q_OBJECT

public:

	explicit ClientSocket(qintptr socketDescriptor, QObject* parent = nullptr);
	virtual ~ClientSocket() override;

public:

	DataDevice* device() { return m_pDevice; }

	void stop();

private:

	void enableKeepAlive();

	DataDevice* createDevice();
	void deleteDevice();

	bool parseManagedPacket(int requestType, const QByteArray& data);
	bool parseDataPacket();
	void readyParseNextRequest();

	QTcpSocket* m_pClientSocket = nullptr;
	QByteArray m_data;

	DataDevice* m_pDevice = nullptr;

	QTimer m_waitResonseTimer;

	bool saveDataToFile();

signals:

	void deviceInfoReceived(ClientSocket* pClientSocket);
	void deviceStateChanged(quint64 imei);
	void deviceReceivingData(quint64 imei);
	void managedPacketReceived(DataDevice* pDevice, const QByteArray& data);
	void dataPacketReceived(DataDevice* pDevice, const QByteArray& data);

	void disconnected();

public slots:

	void onSocketReady();
	void onRequestPicture();
	void onRequestDeviceSetting(int brightness);
	void onTimeoutResponse();
};

// ==============================================================================================

#endif // CLIENTSOCKET_H
