#include "ServerSocket.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ServerSocket::ServerSocket()
{
}

// -------------------------------------------------------------------------------------------------------------------

ServerSocket::~ServerSocket()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::start()
{
	setMaxPendingConnections(m_serverOption.maxPendingConnections());

	connect(&m_tryListenTimer, &QTimer::timeout, this, &ServerSocket::onTryListen, Qt::QueuedConnection);
	m_tryListenTimer.start(1000);
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::stop()
{
	m_tryListenTimer.stop();

	std::vector<std::pair<quint64, ClientSocket*>> clientSocketList;
	clientSocketList.resize(m_clientSocketMap.size());

	std::copy(m_clientSocketMap.begin(), m_clientSocketMap.end(), clientSocketList.begin());

	for(std::pair<quint64, ClientSocket*> it : clientSocketList)
	{
		ClientSocket* pClientSocket = it.second;
		if (pClientSocket == nullptr)
		{
			continue;
		}

		pClientSocket->stop();
	}

	close();

	emit serverStoped();
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::incomingConnection(qintptr socketDescriptor)
{
	// check count of connections
	//
	if (m_clientSocketMap.size() > maxPendingConnections())
	{
		QTcpSocket client;
		client.setSocketDescriptor(socketDescriptor);
		client.disconnectFromHost();
		return;
	}


	//
	//
	ClientSocket* pClientSocket = new ClientSocket(socketDescriptor, this);
	if (pClientSocket == nullptr)
	{
		assert(0);
		return;
	}

	qDebug() << "Connect: " << socketDescriptor;

	//
	//
	connect(pClientSocket, &ClientSocket::deviceReceivingData, this, &ServerSocket::onDeviceReceivingData, Qt::QueuedConnection);
	connect(pClientSocket, &ClientSocket::deviceInfoReceived, this, &ServerSocket::onDeviceInfoReceived, Qt::QueuedConnection);
	connect(pClientSocket, &ClientSocket::managedPacketReceived, this, &ServerSocket::onManagedPacketReceived, Qt::QueuedConnection);
	connect(pClientSocket, &ClientSocket::dataPacketReceived, this, &ServerSocket::onDataPacketReceived, Qt::QueuedConnection);
	connect(pClientSocket, &ClientSocket::deviceStateChanged, this, &ServerSocket::onDeviceStateChanged, Qt::QueuedConnection);

	connect(pClientSocket, &ClientSocket::disconnected, this, &ServerSocket::onClientSocketDisconnected);
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::onTryListen()
{
	if (isListening() == true)
	{
		return;
	}

	// qDebug() << "Try to listen to" << m_serverOption.ip();

	bool result = listen(QHostAddress(m_serverOption.ip()), m_serverOption.port());
	if (result == false)
	{
		return;
	}

	m_tryListenTimer.stop();

	emit serverStarted();
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::onRequestPicture(const std::vector<quint64>& imeiList)
{
	for(quint64 imei : imeiList)
	{
		auto it = m_clientSocketMap.find(imei);
		if (it == m_clientSocketMap.end())
		{
			continue;
		}

		ClientSocket* pClientSocket = it->second;
		if (pClientSocket == nullptr)
		{
			continue;
		}

		if (pClientSocket->device() == nullptr)
		{
			continue;
		}

		if (pClientSocket->device()->imei() != imei)
		{
			continue;
		}

		pClientSocket->onRequestPicture();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::onRequestDeviceSetting(quint64 imei, int brightness)
{
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	if (brightness < 0 || brightness > 255)
	{
		return;
	}

	auto it = m_clientSocketMap.find(imei);
	if (it == m_clientSocketMap.end())
	{
		return;
	}

	ClientSocket* pClientSocket = it->second;
	if (pClientSocket == nullptr)
	{
		return;
	}

	if (pClientSocket->device() == nullptr)
	{
		return;
	}

	if (pClientSocket->device()->imei() != imei)
	{
		return;
	}

	pClientSocket->onRequestDeviceSetting(brightness);
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::onDeviceInfoReceived(ClientSocket* pClientSocket)
{
	if (pClientSocket == nullptr)
	{
		return;
	}

	if (pClientSocket->device() == nullptr)
	{
		return;
	}

	quint64 imei = pClientSocket->device()->imei();
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	if (m_clientSocketMap.find(imei) != m_clientSocketMap.end())
	{
		return;
	}

	m_clientSocketMap.emplace(imei, pClientSocket);
	emit deviceCountChanged(static_cast<int>(m_clientSocketMap.size()));

	//
	//
	emit deviceConnected(pClientSocket->device());
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::onDeviceStateChanged(quint64 imei)
{
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	emit deviceStateChanged(imei);
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::onManagedPacketReceived(DataDevice* pDevice, const QByteArray& data)
{
	if (pDevice == nullptr)
	{
		return;
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return;
	}

	emit managedPacketReceived(pDevice, data);
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::onDataPacketReceived(DataDevice* pDevice, const QByteArray& data)
{
	if (pDevice == nullptr)
	{
		return;
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return;
	}

	emit dataPacketReceived(pDevice, data);
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::onDeviceReceivingData(quint64 imei)
{
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	emit deviceReceivingData(imei);
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::onClientSocketDisconnected()
{
	ClientSocket* pClientSocket = dynamic_cast<ClientSocket*>(sender());
	if (pClientSocket == nullptr)
	{
		return;
	}

	if (pClientSocket->device() == nullptr)
	{
		return;
	}

	emit deviceDisconnected(pClientSocket->device()->imei());

	m_clientSocketMap.erase(pClientSocket->device()->imei());
	emit deviceCountChanged(static_cast<int>(m_clientSocketMap.size()));

	delete pClientSocket;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
