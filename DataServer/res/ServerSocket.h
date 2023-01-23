#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QTcpServer>

#include "ClientSocket.h"

// ==============================================================================================

class ServerSocket : public QTcpServer
{
	Q_OBJECT

public:

	explicit ServerSocket();
	virtual ~ServerSocket() override;

private:

	QByteArray m_data;

	std::list<QTcpSocket*> m_clientSocketList;

signals:

	void serverListening();

	void packetProccessing(int bytes);
	void packetReceived(const QByteArray& data);

public slots:

	void startServer();
	void socketReady();
	void socketDisconnected();

protected:

	void incomingConnection(qintptr socketDescriptor) override;
};

// ==============================================================================================

#endif // SERVERSOCKET_H
