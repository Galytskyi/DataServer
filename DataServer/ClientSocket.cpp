#include "ClientSocket.h"

#include <WinSock2.h>
#include <mstcpip.h>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ClientSocket::ClientSocket(qintptr socketDescriptor, QObject* parent)
	: QObject(parent)
{
	m_pClientSocket = new QTcpSocket(parent);
	if (m_pClientSocket == nullptr)
	{
		assert(0);
		return;
	}

	bool result = m_pClientSocket->setSocketDescriptor(socketDescriptor);
	if (result == false)
	{
		m_pClientSocket->disconnectFromHost();
		delete m_pClientSocket;
		m_pClientSocket = nullptr;

		return;
	}

	connect(m_pClientSocket, &QTcpSocket::readyRead, this, &ClientSocket::onSocketReady, Qt::DirectConnection);
	connect(m_pClientSocket, &QTcpSocket::disconnected, this, &ClientSocket::disconnected);

	//
	//
	connect(&m_waitResonseTimer, &QTimer::timeout, this, &ClientSocket::onTimeoutResponse);

	//
	//
	enableKeepAlive();

	//
	//
	m_pDevice = createDevice();
	if (m_pDevice == nullptr)
	{
		assert(0);
	}
}

// -------------------------------------------------------------------------------------------------------------------

ClientSocket::~ClientSocket()
{
	deleteDevice();
}

// -------------------------------------------------------------------------------------------------------------------

DataDevice* ClientSocket::createDevice()
{
	DataDevice* pDevice = new DataDevice;
	if (pDevice == nullptr)
	{
		return nullptr;
	}

	pDevice->setIp(m_pClientSocket->peerAddress().toString());
	pDevice->setConnectTime(QDateTime::currentDateTime());

	return pDevice;
}

// -------------------------------------------------------------------------------------------------------------------

void ClientSocket::deleteDevice()
{
	if (m_pDevice == nullptr)
	{
		return;
	}

	delete m_pDevice;
	m_pDevice = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void ClientSocket::stop()
{
	m_waitResonseTimer.stop();

	if (m_pClientSocket == nullptr)
	{
		return;
	}

	m_pClientSocket->disconnectFromHost();
}

// -------------------------------------------------------------------------------------------------------------------

void ClientSocket::enableKeepAlive()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	// m_pClientSocket->setSocketOption(QAbstractSocket::KeepAliveOption, true);

	// #include <WinSock2.h>
	// #include <mstcpip.h>
	// LIBS += -lws2_32
	// for disconnect
	//
	struct tcp_keepalive  alive;	// your options for "keepalive" mode
	alive.onoff = TRUE;				// turn it on
	alive.keepalivetime = 1000;		// delay (ms) between requests, here is 30s, default is 2h (7200000)
	alive.keepaliveinterval = 1000; // delay between "emergency" ping requests, their number (6) is not configurable

	DWORD  dwBytesRet = 0;
	WSAIoctl(m_pClientSocket->socketDescriptor(), SIO_KEEPALIVE_VALS, &alive, sizeof(alive), NULL, 0, &dwBytesRet, NULL, NULL);

//	int enableKeepAlive = 1;
//	int maxIdle = 10;	 /* seconds */
//	int count = 3;		// send up to 3 keepalive packets out, then disconnect if no response
//	int interval = 2;  // send a keepalive packet out every 2 seconds (after the 5 second idle period)
//	int result;

//	qintptr socketDescriptor;

//	result = setsockopt(socketDescriptor, SOL_SOCKET, SO_KEEPALIVE, (const char*) &enableKeepAlive, sizeof(enableKeepAlive));
//	result = setsockopt(socketDescriptor, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));
//	result = setsockopt(socketDescriptor, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));
//	result = setsockopt(socketDescriptor, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
}

// -------------------------------------------------------------------------------------------------------------------

void ClientSocket::onSocketReady()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	if (m_pDevice == nullptr)
	{
		return;
	}

	const QByteArray& receivedData = m_pClientSocket->readAll();
	m_data.append(receivedData);

	qDebug() << receivedData.size() << m_data.size();

	if (receivedData.size() < sizeof(DeviceDataHeader))
	{
		return;
	}

	DeviceDataHeader header;
	memcpy(&header, receivedData.data(), sizeof(DeviceDataHeader));

	if (header.signature == DEVICE_SIGNATURE)
	{
		bool result = parseManagedPacket(header.requestType, receivedData);		// this is managed packet

		if (result == false && header.requestType == REQUEST_GET_DEVICE_INFO)	// the first packet from device is header
		{
			m_pClientSocket->disconnectFromHost();
			return;
		}
	}
	else
	{
		parseDataPacket();														// this is data packet
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool ClientSocket::parseManagedPacket(int requestType, const QByteArray& data)
{
	if (m_pDevice == nullptr)
	{
		return false;
	}

	m_data.clear();

	switch (requestType)
	{
		case REQUEST_GET_DEVICE_INFO:
			{
				if (data.size() < sizeof(DeviceRequestGetInfo))
				{
					return false;
				}

				DeviceRequestGetInfo di;
				memcpy(&di, data.data(), sizeof(DeviceRequestGetInfo));

				if (di.headerVersion != LAST_DEVICE_VERSION)
				{
					return false;
				}

				if (ERR_DEVICE_TYPE(di.deviceType) == true)
				{
					return false;
				}

				m_pDevice->setDeviceInfo(di);

				if (m_pDevice->imei() == UNDEFINED_IMEI)
				{
					return false;
				}

				emit deviceInfoReceived(this);
			}
			break;

		case REQUEST_GET_IMAGE:
			{
				if (data.size() < sizeof(DeviceRequestGetImage))
				{
					return false;
				}
			}
			break;

		case REQUEST_SET_DEVICE_PARAMS:
			{
				if (data.size() < sizeof(DeviceRequestSetParams))
				{
					return false;
				}
			}
			break;

		default:

			assert(0);
			return false;
	}

	// managed packet was received
	//
	emit managedPacketReceived(requestType, m_pDevice, data);
	readyParseNextRequest();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ClientSocket::parseDataPacket()
{
	if (m_pDevice == nullptr)
	{
		return false;
	}

	// update amount of data
	//
	m_pDevice->setState(DataDeviceState::ReceivingData);
	m_pDevice->setReceivedBytes(static_cast<int>(m_data.size()));
	emit deviceReceivingData(m_pDevice->imei());

	// start wait response timer
	//
	m_waitResonseTimer.start(WAIT_RESPONSE_TIME);

	// if packet was received completely
	//
	if (m_data.size() >= m_pDevice->imageSize())
	{
		emit dataPacketReceived(m_pDevice, m_data);
		m_data.clear();
		readyParseNextRequest();
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void ClientSocket::readyParseNextRequest()
{
	m_pDevice->setState(DataDeviceState::WaitRequest);
	emit deviceStateChanged(m_pDevice->imei());

	m_waitResonseTimer.stop();
}

// -------------------------------------------------------------------------------------------------------------------

void ClientSocket::onRequestPicture()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	if (m_pDevice == nullptr)
	{
		return;
	}

	if (m_pDevice->state() == DataDeviceState::WaitResponse)
	{
		return;
	}

	if (m_pDevice->state() == DataDeviceState::ReceivingData)
	{
		return;
	}

	//
	//
	m_data.clear();

	//
	//
	DeviceRequestGetImage request;

	qint64 writtenBytes = m_pClientSocket->write((char*)&request, sizeof(DeviceRequestGetImage));
	if (writtenBytes != sizeof(DeviceRequestGetImage))
	{
		assert(0);

		m_pDevice->setState(DataDeviceState::ErrorSentRequest);
		emit deviceStateChanged(m_pDevice->imei());

		return;
	}

	m_pDevice->setLastRequestType(request.requestType);
	m_pDevice->setState(DataDeviceState::WaitResponse);
	emit deviceStateChanged(m_pDevice->imei());

	//
	//
	m_waitResonseTimer.start(WAIT_RESPONSE_TIME);
}

// -------------------------------------------------------------------------------------------------------------------

void ClientSocket::onRequestDeviceSetting(int brightness)
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	if (m_pDevice == nullptr)
	{
		return;
	}

	if (m_pDevice->state() == DataDeviceState::WaitResponse)
	{
		return;
	}

	if (m_pDevice->state() == DataDeviceState::ReceivingData)
	{
		return;
	}

	//
	//
	DeviceRequestSetParams request;

	request.brightness = brightness;

	qint64 writtenBytes = m_pClientSocket->write((char*)&request, sizeof(DeviceRequestSetParams));
	if (writtenBytes != sizeof(DeviceRequestSetParams))
	{
		assert(0);

		m_pDevice->setState(DataDeviceState::ErrorSentRequest);
		emit deviceStateChanged(m_pDevice->imei());

		return;
	}

	m_pDevice->setLastRequestType(request.requestType);
	m_pDevice->setState(DataDeviceState::WaitResponse);
	emit deviceStateChanged(m_pDevice->imei());

	//
	//
	m_waitResonseTimer.start(WAIT_RESPONSE_TIME);
}

// -------------------------------------------------------------------------------------------------------------------

void ClientSocket::onTimeoutResponse()
{
	if (m_pDevice == nullptr)
	{
		return;
	}

	m_pDevice->setState(DataDeviceState::TimeoutResponse);
	emit deviceStateChanged(m_pDevice->imei());

	m_waitResonseTimer.stop();
}

// -------------------------------------------------------------------------------------------------------------------

bool ClientSocket::saveDataToFile()
{
	QString fileName = timeToStr(QDateTime::currentDateTime()) + ".bin";
	fileName.replace(':', '-');

	QFile packetFile(fileName);
	if (packetFile.open(QIODevice::WriteOnly) == false)
	{
		return false;
	}

	packetFile.write(m_data, m_data.size());
	packetFile.close();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

