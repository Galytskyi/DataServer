#include "MainWindow.h"

#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>
#include <QDockWidget>
#include <QCloseEvent>
#include <QDesktopServices>

#include "Database.h"
#include "Options.h"
#include "OptionsDialog.h"
#include "DeviceListDialog.h"
#include "DeviceSettingDialog.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	// init interface
	//
	createInterface();

	// open database
	//
	theDatabase.setDatabaseOption(theOptions.database());
	theDatabase.open();
	connect(this, &MainWindow::appendPacket, &theDatabase, &Database::appendToBase, Qt::QueuedConnection);

	// DeviceBase
	//
	theDeviceBase.load();

	// PacketBase
	//
	m_packetBase.load();
	connect(this, &MainWindow::appendPacket, &m_packetBase, &PacketBase::appendToBase, Qt::QueuedConnection);

	// runServerSocket
	//
	m_serverSocket.setServerOption(theOptions.server());

	connect(&m_serverSocket, &ServerSocket::serverStarted, this, &MainWindow::onServerStarted, Qt::QueuedConnection);
	connect(&m_serverSocket, &ServerSocket::serverStoped, this, &MainWindow::onServerStoped, Qt::QueuedConnection);
	connect(&m_serverSocket, &ServerSocket::deviceCountChanged, this, &MainWindow::onDeviceCountChanged, Qt::QueuedConnection);
	connect(&m_serverSocket, &ServerSocket::managedPacketReceived, this, &MainWindow::onManagedPacketReceived, Qt::QueuedConnection);
	connect(&m_serverSocket, &ServerSocket::dataPacketReceived, this, &MainWindow::onDataPacketReceived, Qt::QueuedConnection);

	m_serverSocket.start();
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createInterface()
{
	setWindowTitle(tr("Data server"));

	QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
	resize(static_cast<int>(screen.width() * 0.5), static_cast<int>(screen.height() * 0.5));
	move(screen.center() - rect().center());

	createActions();
	createMenu();
	createToolBars();
	createPanels();
	createPacketView();
	createStatusBar();

	loadSettings();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createActions()
{
	// Image
	//
	m_pLoadImageAction = new QAction(tr("&Load ..."), this);
	m_pLoadImageAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_L});
	m_pLoadImageAction->setIcon(QIcon(":/icons/Load.png"));
	m_pLoadImageAction->setToolTip(tr("Load image"));
	connect(m_pLoadImageAction, &QAction::triggered, this, &MainWindow::onLoadImage);

	m_pRemoveImageAction = new QAction(tr("&Remove ..."), this);
	m_pRemoveImageAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_Delete});
	m_pRemoveImageAction->setIcon(QIcon(":/icons/Remove.png"));
	m_pRemoveImageAction->setToolTip(tr("Remove image"));
	connect(m_pRemoveImageAction, &QAction::triggered, this, &MainWindow::onRemoveImage);

	// View
	//
	m_pDeviceListAction = new QAction(tr("&Devices ..."), this);
	m_pDeviceListAction->setIcon(QIcon(":/icons/Device.png"));
	m_pDeviceListAction->setToolTip(tr("List of known devices"));
	connect(m_pDeviceListAction, &QAction::triggered, this, &MainWindow::onDeviceList);

	// Tools
	//
	m_pDeviceSettingAction = new QAction(tr("&Device settings ..."), this);
	m_pDeviceSettingAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_S});
	m_pDeviceSettingAction->setIcon(QIcon(":/icons/Settings.png"));
	m_pDeviceSettingAction->setToolTip(tr("Set additional parameters of device"));
	connect(m_pDeviceSettingAction, &QAction::triggered, this, &MainWindow::onDeviceSetting);

	m_pOptionsAction = new QAction(tr("&Options ..."), this);
	m_pOptionsAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_O});
	m_pOptionsAction->setIcon(QIcon(":/icons/Options.png"));
	m_pOptionsAction->setToolTip(tr("Application settings"));
	connect(m_pOptionsAction, &QAction::triggered, this, &MainWindow::onOptions);

	// ?
	//
	m_pAboutQtAction = new QAction(tr("About Qt ..."), this);
	m_pAboutQtAction->setIcon(QIcon(":/icons/About Connection.png"));
	m_pAboutQtAction->setToolTip(tr("Show Qt information"));
	connect(m_pAboutQtAction, &QAction::triggered, this, &MainWindow::OnAboutQt);

	m_pDocumentationAction = new QAction(tr("Documentation ..."), this);
	m_pDocumentationAction->setIcon(QIcon(":/icons/About Connection.png"));
	m_pDocumentationAction->setToolTip(tr("Show documentation of data packets"));
	connect(m_pDocumentationAction, &QAction::triggered, this, &MainWindow::onDocumentation);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMenu()
{
	QMenuBar* pMenuBar = menuBar();
	if (pMenuBar == nullptr)
	{
		return;
	}

	// Image
	//
	m_pImageMenu = pMenuBar->addMenu(tr("&Image"));

	m_pImageMenu->addAction(m_pLoadImageAction);
	m_pImageMenu->addAction(m_pRemoveImageAction);

	// View
	//
	m_pViewMenu = pMenuBar->addMenu(tr("&View"));

	m_pViewMenu->addAction(m_pDeviceListAction);
	m_pViewMenu->addSeparator();

	// Tools
	//
	m_pToolsMenu = pMenuBar->addMenu(tr("&Tools"));

	m_pToolsMenu->addAction(m_pDeviceSettingAction);
	m_pToolsMenu->addSeparator();
	m_pToolsMenu->addAction(m_pOptionsAction);

	// ?
	//
	m_pInfoMenu = pMenuBar->addMenu(tr("&?"));

	m_pInfoMenu->addAction(m_pAboutQtAction);
	m_pInfoMenu->addAction(m_pDocumentationAction);
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createToolBars()
{
	// Control panel
	//
	m_pControlToolBar = new QToolBar(this);
	if (m_pControlToolBar != nullptr)
	{
		m_pControlToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		m_pControlToolBar->setWindowTitle(tr("Control panel"));
		m_pControlToolBar->setObjectName(m_pControlToolBar->windowTitle());
		addToolBarBreak(Qt::TopToolBarArea);
		addToolBar(m_pControlToolBar);

		m_pControlToolBar->addAction(m_pLoadImageAction);
		m_pControlToolBar->addSeparator();
		m_pControlToolBar->addAction(m_pRemoveImageAction);
		m_pControlToolBar->addSeparator();
		m_pControlToolBar->addAction(m_pOptionsAction);
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createPanels()
{
	// Device list panel
	//
	m_pDeviceListPanel = new PanelDeviceList(&m_serverSocket, this);
	if (m_pDeviceListPanel != nullptr)
	{
		m_pDeviceListPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

		addDockWidget(Qt::BottomDockWidgetArea, m_pDeviceListPanel);

		m_pDeviceListPanel->show();

		QAction* previewAction = m_pDeviceListPanel->toggleViewAction();
		if (previewAction != nullptr)
		{
			previewAction->setText(tr("&Device list panel ..."));
			previewAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_D});
			previewAction->setIcon(QIcon(":/icons/Preview.png"));
			previewAction->setToolTip(tr("Device panel view"));

			if (m_pViewMenu != nullptr)
			{
				m_pViewMenu->addAction(previewAction);
			}

			if (m_pControlToolBar != nullptr)
			{
				//m_pControlToolBar->addAction(previewAction);
			}
		}

		connect(m_pDeviceListPanel, &PanelDeviceList::requestPicture, &m_serverSocket, &ServerSocket::onRequestPicture, Qt::QueuedConnection);
		connect(m_pDeviceListPanel, &PanelDeviceList::requestBrightness, &m_serverSocket, &ServerSocket::onRequestDeviceSetting, Qt::QueuedConnection);

		connect(&m_serverSocket, &ServerSocket::deviceConnected, m_pDeviceListPanel, &PanelDeviceList::append, Qt::QueuedConnection);
		connect(&m_serverSocket, &ServerSocket::deviceDisconnected, m_pDeviceListPanel, &PanelDeviceList::remove, Qt::DirectConnection);

		connect(&m_serverSocket, &ServerSocket::deviceStateChanged, m_pDeviceListPanel, &PanelDeviceList::update, Qt::QueuedConnection);
		connect(&m_serverSocket, &ServerSocket::deviceReceivingData, m_pDeviceListPanel, &PanelDeviceList::update, Qt::QueuedConnection);

		connect(m_pLoadImageAction, &QAction::triggered, m_pDeviceListPanel, &PanelDeviceList::onRequestPicture, Qt::QueuedConnection);
		connect(m_pDeviceSettingAction, &QAction::triggered, m_pDeviceListPanel, &PanelDeviceList::onDeviceSetting, Qt::QueuedConnection);
	}

	// Picture view panel
	//
	m_pPictureViewPanel = new PanelPictureView(this);
	if (m_pPictureViewPanel != nullptr)
	{
		m_pPictureViewPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

		addDockWidget(Qt::RightDockWidgetArea, m_pPictureViewPanel);

		m_pPictureViewPanel->hide();

		QAction* previewAction = m_pPictureViewPanel->toggleViewAction();
		if (previewAction != nullptr)
		{
			previewAction->setText(tr("&Picture view panel ..."));
			previewAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_I});
			previewAction->setIcon(QIcon(":/icons/Preview.png"));
			previewAction->setToolTip(tr("Picture panel view"));

			if (m_pViewMenu != nullptr)
			{
				m_pViewMenu->addAction(previewAction);
			}

			if (m_pControlToolBar != nullptr)
			{
				//m_pControlToolBar->addAction(previewAction);
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createPacketView()
{
	m_packetView = new PacketView(this);
	if (m_packetView == nullptr)
	{
		return;
	}

	m_packetView->setFrameStyle(QFrame::NoFrame);

	//
	//
	connect(this, &MainWindow::appendPacket, m_packetView, &PacketView::onAppendPacket, Qt::QueuedConnection);
	connect(m_pRemoveImageAction, &QAction::triggered, m_packetView, &PacketView::onRemovePacket, Qt::QueuedConnection);

	connect(m_packetView, &PacketView::updateInBase, &theDatabase, &Database::updateInBase, Qt::QueuedConnection);
	connect(m_packetView, &PacketView::removeFromBase, &theDatabase, &Database::removeFromBase, Qt::QueuedConnection);
	connect(m_packetView, &PacketView::removeFromBase, &m_packetBase, &PacketBase::removeFromBase, Qt::QueuedConnection);

	connect(&m_packetBase, &PacketBase::packetBaseLoaded, m_packetView, &PacketView::onPacketBaseLoaded, Qt::QueuedConnection);

	connect(m_packetView, &PacketView::packetSelected, this, &MainWindow::onPacketCliked, Qt::QueuedConnection);

	//
	//
	setCentralWidget(m_packetView);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createStatusBar()
{
	QStatusBar* pStatusBar = statusBar();
	if (pStatusBar == nullptr)
	{
		return;
	}

	// create
	//
	m_statusEmpty = new QLabel(pStatusBar);
	m_statusClient = new QLabel(pStatusBar);
	m_statusServer = new QLabel(pStatusBar);

	// addWidget
	//
	pStatusBar->addWidget(m_statusServer);
	pStatusBar->addWidget(m_statusClient);
	pStatusBar->addWidget(m_statusEmpty);

	pStatusBar->setLayoutDirection(Qt::RightToLeft);

	// set default value
	//
	m_statusEmpty->setText(QString());

	onDeviceCountChanged(0);
	onServerStoped();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onLoadImage()
{
	// To see PanelDeviceList
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onRemoveImage()
{
	// To see PacketView
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onDeviceList()
{
	DeviceListDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onDeviceSetting()
{
	 // To see PanelDeviceList
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onOptions()
{
	OptionsDialog dialog(theOptions.server(), theOptions.database(), theOptions.imei(), this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	// save new options
	//
	theOptions.setServer(dialog.serverOption());
	theOptions.server().save();

	//
	//
	theOptions.setDatabase(dialog.databaseOption());
	theOptions.database().save();

	//
	//
	theOptions.setImei(dialog.imeiOption());
	theOptions.imei().save();

	// restart
	//
	m_serverSocket.stop();
	m_serverSocket.setServerOption(theOptions.server());
	m_serverSocket.start();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::OnAboutQt()
{
	QMessageBox::aboutQt(this, qAppName());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onDocumentation()
{
	QString filePath = QApplication::applicationDirPath() + "/docs/Packets.pdf";

	QFile file(filePath);
	if (file.exists() == false)
	{
		QMessageBox::critical(this, qAppName(), QObject::tr("Help file '%1' does not exist!").arg(filePath));
		return;
	}

	QUrl url = QUrl::fromLocalFile(filePath);
	QDesktopServices::openUrl(url);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onServerStarted()
{
	if (m_statusServer == nullptr)
	{
		return;
	}

	m_statusServer->setText(tr(" Listening: ") + theOptions.server().ip() + " : " + QString::number(theOptions.server().port()));
	m_statusServer->setStyleSheet("background-color: rgb(0x0, 0x0, 0x0);");
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onServerStoped()
{
	if (m_statusServer == nullptr)
	{
		return;
	}

	m_statusServer->setText(tr(" Not listening "));
	m_statusServer->setStyleSheet("background-color: rgb(255, 160, 160);");
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onDeviceCountChanged(int count)
{
	if (m_statusClient == nullptr)
	{
		return;
	}

	m_statusClient->setText(tr(" Devices: %1 ").arg(count));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onManagedPacketReceived(DataDevice* pDevice, const QByteArray& data)
{
	if (pDevice == nullptr)
	{
		return;
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return;
	}

	if (data.size() == 0)
	{
		return;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onDataPacketReceived(DataDevice* pDevice, const QByteArray& data)
{
	if (pDevice == nullptr)
	{
		return;
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return;
	}

	if (data.size() == 0)
	{
		return;
	}

	//
	//
	QByteArray imageBmpData;

	// read header of BMP
	//
	QFile headerFile(":/Images/HeadBmp.bmp");
	if (headerFile.open(QIODevice::ReadOnly) == false)
	{
		assert(0);
		return;
	}

	imageBmpData = headerFile.readAll();

	headerFile.close();

	// correct size of bmp
	//
	*(int *)&imageBmpData[18] = pDevice->imageWidth();
	*(int *)&imageBmpData[22] = pDevice->imageHeight();

	// append data of image
	//
	imageBmpData.append(data);

	// convert from BMP to JPG in order to minimize size
	//
	QImage img;
	img.loadFromData(imageBmpData);

	QByteArray imageJpgData;
	QBuffer buffer(&imageJpgData);
	buffer.open(QIODevice::WriteOnly);
	img.save(&buffer, "JPG");

	// create record in database
	//
	DataPacket* packet = new DataPacket;
	if (packet == nullptr)
	{
		return;
	}

	packet->setPacketTime(QDateTime::currentDateTime());
	packet->setImei(pDevice->imei());

	packet->setImageType(pDevice->imageType());
	packet->setImageSize(static_cast<int>(imageJpgData.size()));
	packet->setImageWidth(pDevice->imageWidth());
	packet->setImageHeight(pDevice->imageHeight());
	packet->setBrightness(pDevice->brightness());
	packet->setLocation(pDevice->location());
	packet->setImageData(imageJpgData);

	// append packet to:
	// - Database
	// - PaketView
	// - PaketBase
	//
	emit appendPacket(packet);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onPacketCliked(DataPacket* pPacket)
{
	if (m_pPictureViewPanel == nullptr)
	{
		return;
	}

	if (pPacket == nullptr)
	{
		return;
	}

	if (pPacket->packetID() == SQL_INVALID_KEY)
	{
		return;
	}

	if (pPacket->imei() == UNDEFINED_IMEI)
	{
		return;
	}

	m_pPictureViewPanel->showPicture(*pPacket);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::checkImgDir()
{
	QDir dir;

	QString imagePath = QDir::currentPath() + QDir::separator() + IMAGE_DIRECTORY;

	if (dir.exists(imagePath) == true)
	{
		return;
	}

	bool result = dir.mkdir(imagePath);
	if (result == false)
	{
		QMessageBox::critical(this, windowTitle(), tr("Cannot create directory for images!"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadSettings()
{
	QSettings s;

	QByteArray geometry = s.value(QString("%1MainWindow/geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();
	QByteArray state = s.value(QString("%1MainWindow/State").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();

	restoreGeometry(geometry);
	restoreState(state);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::saveSettings()
{
	QSettings s;

	s.setValue(QString("%1MainWindow/Geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY), saveGeometry());
	s.setValue(QString("%1MainWindow/State").arg(WINDOW_GEOMETRY_OPTIONS_KEY), saveState());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* e)
{
	m_serverSocket.stop();

	theDatabase.close(); // close database

	saveSettings();

	QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------

