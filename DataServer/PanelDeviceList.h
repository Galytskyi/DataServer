#ifndef PANELDEVICELIST_H
#define PANELDEVICELIST_H

#include <QMainWindow>
#include <QDockWidget>

#include "ServerSocket.h"
#include "DataDevice.h"

// ==============================================================================================

const char* const			PanelDeviceListColumn[] =
{
							QT_TRANSLATE_NOOP("PanelDeviceList.h", "IMEI"),
							QT_TRANSLATE_NOOP("PanelDeviceList.h", "Location"),
							QT_TRANSLATE_NOOP("PanelDeviceList.h", "IP"),
							QT_TRANSLATE_NOOP("PanelDeviceList.h", "Connect time"),
							QT_TRANSLATE_NOOP("PanelDeviceList.h", "Brightness"),
							QT_TRANSLATE_NOOP("PanelDeviceList.h", "State"),
};

const int					PANEL_DEVICE_LIST_COLUMN_COUNT	= sizeof(PanelDeviceListColumn)/sizeof(PanelDeviceListColumn[0]);

const int					PANEL_DEVICE_LIST_COLUMN_IMEI		= 0,
							PANEL_DEVICE_LIST_COLUMN_LOCATION	= 1,
							PANEL_DEVICE_LIST_COLUMN_IP			= 2,
							PANEL_DEVICE_LIST_COLUMN_TIME		= 3,
							PANEL_DEVICE_LIST_COLUMN_BRIGHT		= 4,
							PANEL_DEVICE_LIST_COLUMN_STATE		= 5;

const int					PanelDeviceListWidth[PANEL_DEVICE_LIST_COLUMN_COUNT] =
{
							100,	// PANEL_DEVICE_LIST_COLUMN_IMEI
							150,	// PANEL_DEVICE_LIST_COLUMN_LOCATION
							100,	// PANEL_DEVICE_LIST_COLUMN_IP
							150,	// PANEL_DEVICE_LIST_COLUMN_TIME
							100,	// PANEL_DEVICE_LIST_COLUMN_BRIGHT
							150,	// PANEL_DEVICE_LIST_COLUMN_STATE
};

// ----------------------------------------------------------------------------------------------

class PanelDeviceModel : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit PanelDeviceModel(QObject* parent = nullptr);
	virtual ~PanelDeviceModel() override;

public:

	int count() const { return static_cast<int>(m_deviceCount); }

	QString text(int row, int column, DataDevice* pDevice) const;

	bool append(DataDevice* pDevice);
	DataDevice* at(int index) const;
	void remove(quint64 imei);
	void update(quint64 imei);
	void updateLocation(quint64 imei, const QString& location);

	void set(const std::vector<DataDevice*>& list_add);
	void clear();

private:

	mutable QMutex m_deviceMutex;
	std::vector<DataDevice*> m_deviceList;
	int m_deviceCount = 0;

	int columnCount(const QModelIndex &parent) const override;
	int rowCount(const QModelIndex &parent=QModelIndex()) const override;

	void updateRow(int row);

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class PanelDeviceList : public QDockWidget
{
	Q_OBJECT

public:

	explicit PanelDeviceList(ServerSocket* pServerSocket, QWidget* parent = nullptr);
	virtual ~PanelDeviceList() override;

	QTableView* view() const { return m_pView; }
	PanelDeviceModel& model() {return m_model; }

private:

	ServerSocket* m_pServerSocket = nullptr;

	QMenu* m_pContextMenu = nullptr;
	QMenu* m_pBrightnessMenu = nullptr;

	QAction* m_pRequestPictureAction = nullptr;

	QAction* m_pRequestBrightness000Action = nullptr;
	QAction* m_pRequestBrightness025Action = nullptr;
	QAction* m_pRequestBrightness050Action = nullptr;
	QAction* m_pRequestBrightness075Action = nullptr;
	QAction* m_pRequestBrightness100Action = nullptr;

	QAction* m_pDeviceSettingAction = nullptr;

	QAction* m_pSetLocatinAction = nullptr;

	QAction* m_pFindAction = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pSelectAllAction = nullptr;

	QMenu* m_headerContextMenu = nullptr;
	std::vector<QAction*> m_pColumnActionList;

	QMainWindow* m_pDeviceWindow = nullptr;

	QTableView* m_pView = nullptr;
	PanelDeviceModel m_model;

	void createInterface();
	void createContextMenu();

	void createHeaderContextMenu();
	void hideColumn(int column, bool hide);

signals:

	void requestPicture(const std::vector<quint64>& imeiList);
	void requestBrightness(quint64 imei, int brightness);

public slots:

	// slots of menu
	//
	void onContextMenu(QPoint);

	void onRequestPicture();
	void onRequestBrightness();
	void onDeviceSetting();
	void onSetLocation();
	void onFind();
	void onCopy();
	void onSelectAll();

	// slots for list header, to hide or show columns
	//
	void onHeaderContextMenu(QPoint);
	void onColumnAction(QAction* action);

	// slot of ServerSocket
	//
	bool append(DataDevice* pDataDevice);
	void remove(quint64 imei);
	void update(quint64 imei);

	// slot of DeviceListDialog
	//
	void updateLocation(quint64 imei, const QString& location);

};

// ==============================================================================================

#endif // PANELDEVICELIST_H
