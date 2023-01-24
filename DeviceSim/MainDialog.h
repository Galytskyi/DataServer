#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QTcpSocket>

// ==============================================================================================

QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

// ==============================================================================================

const QString DEFAULT_SERVER_IP = "192.168.75.77";
//const QString DEFAULT_SERVER_IP = "82.207.127.173";
const int DEFAULT_SERVER_PORT = 49152;

const int DEFAULT_FRAME_SIZE = 320;
const int DEFAULT_FRAME_COUNT = 180;

// ==============================================================================================

class MainDialog : public QDialog
{
	Q_OBJECT

public:
	MainDialog(QWidget *parent = nullptr);
	~MainDialog();

public:

	Ui::MainDialog* m_ui = nullptr;

	QTcpSocket* m_pClientSocket = nullptr;
	QByteArray m_data;

private:

	static void startSendDataThread(MainDialog* pThis);

	void updateState();

signals:

	void socketConnect();
	void socketDisconnect();
	void socketSendData(int packetNo);

private slots:

	void socketConnected();
	void socketDisconnected();
	void socketReadyRead();

	void cmd_socketSendHeader();
	void cmd_socketSendData(int packetNo);

	void on_m_connectButton_clicked();
	void on_m_disconnectButton_clicked();
};


// ==============================================================================================

#endif // MAINDIALOG_H
