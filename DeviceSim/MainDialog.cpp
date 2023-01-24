#include "MainDialog.h"
#include "ui_maindialog.h"

#include <QMessageBox>
#include <QtConcurrent>

#include "../DataServer/DevicePacket.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MainDialog::MainDialog(QWidget *parent)
	: QDialog(parent)
	, m_ui(new Ui::MainDialog)
{
	m_ui->setupUi(this);

	//
	//
	m_ui->m_serverEdit->setText(DEFAULT_SERVER_IP);
	m_ui->m_stateLabel->setText(QString());

	//
	//
	connect(this, &MainDialog::socketSendData, this, &MainDialog::cmd_socketSendData, Qt::QueuedConnection);

	//
	//
	m_data.resize(DEFAULT_FRAME_SIZE);

	//
	//
	m_pClientSocket = new QTcpSocket(this);
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	connect(m_pClientSocket, &QTcpSocket::connected, this, &MainDialog::socketConnected, Qt::QueuedConnection);
	connect(m_pClientSocket, &QTcpSocket::disconnected, this, &MainDialog::socketDisconnected, Qt::QueuedConnection);
	connect(m_pClientSocket, &QTcpSocket::readyRead, this, &MainDialog::socketReadyRead);

	//
	//
	srand((uint)QTime::currentTime().msec());
}

// -------------------------------------------------------------------------------------------------------------------

MainDialog::~MainDialog()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	delete m_pClientSocket;
	m_pClientSocket = nullptr;

	delete m_ui;
}

// -------------------------------------------------------------------------------------------------------------------

void MainDialog::socketConnected()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	QString stateStr = QMetaEnum::fromType<QAbstractSocket::SocketState>().valueToKey(m_pClientSocket->state());

	qDebug() << stateStr;
	m_ui->m_stateLabel->setText(stateStr);

	cmd_socketSendHeader();
}

// -------------------------------------------------------------------------------------------------------------------

void MainDialog::socketDisconnected()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	QString stateStr = QMetaEnum::fromType<QAbstractSocket::SocketState>().valueToKey(m_pClientSocket->state());

	qDebug() << stateStr;
	m_ui->m_stateLabel->setText(stateStr);
	m_ui->m_imeiLabel->setText(QString());
}

// -------------------------------------------------------------------------------------------------------------------

void MainDialog::socketReadyRead()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	const QByteArray& receivedData = m_pClientSocket->readAll();
	if (receivedData.isEmpty() == true)
	{
		return;
	}

	if (receivedData.size() < sizeof(DeviceDataHeader))
	{
		return;
	}

	DeviceDataHeader header;
	memcpy(&header, receivedData.data(), sizeof(DeviceDataHeader));

	if (header.signature != DEVICE_SIGNATURE)
	{
		return;
	}

	switch (header.requestType)
	{
		case REQUEST_GET_IMAGE:
			{
				QFuture<void> resRun = QtConcurrent::run(MainDialog::startSendDataThread, this);
			}
			break;

		case REQUEST_SET_DEVICE_PARAMS:
			{
				if (receivedData.size() < sizeof(DeviceRequestSetParams))
				{
					return;
				}

				DeviceRequestSetParams request;
				memcpy(&request, receivedData.data(), sizeof(DeviceRequestSetParams));

				qint64 sentBytes = m_pClientSocket->write(receivedData);
				qDebug() << "Reponse:" << sentBytes;
				qDebug() << "Brightness:" << request.brightness;
				QMessageBox::information(this, windowTitle(), tr("Set brightness of flashlight: ") + QString::number(request.brightness));
			}
			break;

		default:
			assert(0);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainDialog::cmd_socketSendHeader()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	QAbstractSocket::SocketState socketState = m_pClientSocket->state();
	if (socketState != QAbstractSocket::ConnectedState)
	{
		return;
	}

	//
	//
	DeviceRequestGetInfo di;

	di.signature = DEVICE_SIGNATURE;

	di.headerSize = sizeof(DeviceRequestGetInfo);
	di.requestType = REQUEST_GET_DEVICE_INFO;
	di.headerVersion = LAST_DEVICE_VERSION;

	di.deviceType = DeviceType::DeviceType_1;

	for(int i = 0; i < DEVICE_IMEI_SIZE; i++)
	{
		di.IMEI[i] = rand() % 10 + '0';
	}

	di.reserv0 = 0;

	for(int i = 0; i < DEVICE_ICCID_SIZE; i++)
	{
		di.ICCID[i] = rand() % 10 + '0';
	}

	di.imageType = 123;
	di.imageSize = 320 * 180;
	di.imageWidth = 320;
	di.imageHeight = 180;

	di.frameSize = 320;
	di.frameCount = 180;

	di.brightness = 27;

	qint64 sentBytes = m_pClientSocket->write((char*)(&di), sizeof(DeviceRequestGetInfo));

	if (sentBytes == sizeof(DeviceRequestGetInfo))
	{
		qDebug() << "Header was sent" << sentBytes;
	}
	else
	{
		qDebug() << "Error - Header was not sent" << sentBytes;
		return;
	}

	m_ui->m_imeiLabel->setText((char*) di.IMEI);
}

// -------------------------------------------------------------------------------------------------------------------

void MainDialog::cmd_socketSendData(int packetNo)
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	QAbstractSocket::SocketState socketState = m_pClientSocket->state();
	if (socketState != QAbstractSocket::ConnectedState)
	{
		return;
	}

	m_ui->m_stateLabel->setText(tr("Packet: ") + QString::number(packetNo + 1));

	if (packetNo < 90)
	{
		m_data.fill(0);
	}
	else
	{
		m_data.fill(static_cast<quint8>(255));
	}

	qint64 sentBytes = m_pClientSocket->write(m_data);

	qDebug() << sentBytes;
}

// -------------------------------------------------------------------------------------------------------------------

void MainDialog::on_m_connectButton_clicked()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	if (m_pClientSocket->state() == QAbstractSocket::ConnectedState)
	{
		return;
	}

	QString serverIP = m_ui->m_serverEdit->text();
	if (serverIP.isEmpty() == true)
	{
		QMessageBox::critical(this, windowTitle(), tr("Server IP is empty!"));
		return;
	}

	//
	//
	m_pClientSocket->connectToHost(serverIP, DEFAULT_SERVER_PORT);

	m_pClientSocket->waitForConnected();

	qDebug() << m_pClientSocket->state();
}

// -------------------------------------------------------------------------------------------------------------------

void MainDialog::on_m_disconnectButton_clicked()
{
	if (m_pClientSocket == nullptr)
	{
		return;
	}

	if (m_pClientSocket->state() != QAbstractSocket::ConnectedState)
	{
		return;
	}

	m_pClientSocket->disconnectFromHost();
}

// -------------------------------------------------------------------------------------------------------------------

void MainDialog::startSendDataThread(MainDialog* pThis)
{
	if (pThis == nullptr)
	{
		return;
	}

	if (pThis->m_pClientSocket == nullptr)
	{
		return;
	}

	QAbstractSocket::SocketState socketState = pThis->m_pClientSocket->state();
	if (socketState != QAbstractSocket::ConnectedState)
	{
		return;
	}

	/*
	// take photo from file
	//
	QByteArray fileData;

	QFile file("image.bmp.raw");
	if (file.open(QIODevice::ReadOnly) == false)
	{
		return;
	}

	fileData = file.readAll();

	file.close();
	*/

	//
	//
	for(int i = 0; i < DEFAULT_FRAME_COUNT; i++)
	{
		/*
		// send photo
		//
		for(int b = 0; b < DEFAULT_FRAME_SIZE; b++)
		{
			int offset = i * DEFAULT_FRAME_SIZE;
			pThis->m_data[b] = fileData[b + offset];
		}
		*/

		emit pThis->socketSendData(i);

		QThread::msleep(1);
	}
}

// -------------------------------------------------------------------------------------------------------------------
