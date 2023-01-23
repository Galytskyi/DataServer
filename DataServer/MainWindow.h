#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "PacketView.h"
#include "DeviceBase.h"
#include "PanelDeviceList.h"
#include "PanelPictureView.h"
#include "ServerSocket.h"

// ==============================================================================================

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	virtual ~MainWindow() override;

private:

	PacketBase				m_packetBase;
	ServerSocket			m_serverSocket;

private:

	// Actions of main menu
	//
							// menu - Image
							//
	QAction*				m_pLoadImageAction = nullptr;
	QAction*				m_pRemoveImageAction = nullptr;

							// menu - View
							//
	QAction*				m_pDeviceListAction = nullptr;

							// menu - Tools
							//
	QAction*				m_pDeviceSettingAction = nullptr;
	QAction*				m_pOptionsAction = nullptr;

							// menu - ?
							//
	QAction*				m_pAboutQtAction = nullptr;
	QAction*				m_pDocumentationAction = nullptr;

							//
							//
	QMenu*					m_pContextMenu = nullptr;

private:

	// Elements of interface - Menu
	//
	QMenu*					m_pImageMenu = nullptr;
	QMenu*					m_pViewMenu = nullptr;
	QMenu*					m_pToolsMenu = nullptr;
	QMenu*					m_pInfoMenu = nullptr;

	// Elements of interface - ToolBar
	//
	QToolBar*				m_pControlToolBar = nullptr;

	// Elements of interface - View
	//
	PacketView*				m_packetView = nullptr;

	// Elements of interface - Panels
	//
	PanelDeviceList*		m_pDeviceListPanel = nullptr;
	PanelPictureView*		m_pPictureViewPanel = nullptr;

	// Elements of interface - StatusBar
	//
	QLabel*					m_statusEmpty = nullptr;
	QLabel*					m_statusClient = nullptr;
	QLabel*					m_statusServer = nullptr;

private:

	bool					createInterface();

	void					createActions();
	void					createMenu();
	bool					createToolBars();
	void					createPanels();
	void					createPacketView();
	void					createStatusBar();

	void					checkImgDir();

	void					loadSettings();
	void					saveSettings();

protected:

	void					closeEvent(QCloseEvent* e) override;

signals:

	// from packetReceived
	//
	void					appendPacket(DataPacket*);

private slots:

	// Slots of main menu
	//

	// menu - Image
	//
	void					onLoadImage();
	void					onRemoveImage();

	// menu - View
	//
	void					onDeviceList();

	// menu - Tools
	//
	void					onDeviceSetting();
	void					onOptions();

	// menu - ?
	//
	void					OnAboutQt();
	void					onDocumentation();

	// Slots of contex menu
	//
	void					onContextMenu(QPoint);

	// Slot of ServerSocket
	//
	void					onServerStarted();
	void					onServerStoped();
	void					onDeviceCountChanged(int count);
	void					onManagedPacketReceived(DataDevice* pDevice, const QByteArray& data);
	void					onDataPacketReceived(DataDevice* pDevice, const QByteArray& data);


	// Slot of PacketView
	//
	void					onPacketCliked(DataPacket* pPacket);
};

// ==============================================================================================

#endif // MAINWINDOW_H
