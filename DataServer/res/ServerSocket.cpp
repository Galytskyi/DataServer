#include "ServerSocket.h"

#include "Options.h"

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

void ServerSocket::startServer()
{
	bool result = listen(QHostAddress(DEFAULT_SERVER_IP), DEFAULT_SERVER_PORT);
	if (result == false)
	{
		return;
	}

	emit serverListening();
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::	incomingConnection(qintptr socketDescriptor)
{
	// check
	//
	if (m_clientSocketList.size() > maxPendingConnections())
	{
		QTcpSocket client;
		client.setSocketDescriptor(socketDescriptor);
		client.disconnectFromHost();
		return;
	}

	// create client socket
	//
	QTcpSocket* pClientSocket = new QTcpSocket(this);
	if (pClientSocket == nullptr)
	{
		assert(0);
		return;
	}

	m_data.clear();

	pClientSocket->setSocketDescriptor(socketDescriptor);

	connect(pClientSocket, &QTcpSocket::readyRead, this, &ServerSocket::socketReady);
	connect(pClientSocket, &QTcpSocket::disconnected, this, &ServerSocket::socketDisconnected);

	// append client socket
	//
	m_clientSocketList.push_back(pClientSocket);

	//
	//
	emit packetProccessing(0);
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::socketReady()
{
	QTcpSocket* pClientSocket = dynamic_cast<QTcpSocket*>(sender());
	if (pClientSocket == nullptr)
	{
		return;
	}

	 m_data.append(pClientSocket->readAll());

	 if (m_data.size() == MAX_IMAGE_DATA_SIZE)
	 {
		emit packetReceived(m_data);
	 }

	 emit packetProccessing(static_cast<int>(m_data.size()));
}

// -------------------------------------------------------------------------------------------------------------------

void ServerSocket::socketDisconnected()
{
	QTcpSocket* pClientSocket = dynamic_cast<QTcpSocket*>(sender());
	if (pClientSocket == nullptr)
	{
		return;
	}

	emit packetProccessing(-1);

	m_clientSocketList.remove(pClientSocket);

	pClientSocket->deleteLater();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
