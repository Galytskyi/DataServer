#ifndef DEVICELISTDIALOG_H
#define DEVICELISTDIALOG_H

#include "DeviceBase.h"

// ==============================================================================================

const char* const			DeviceListColumn[] =
{
							QT_TRANSLATE_NOOP("DeviceList.h", "IMEI"),
							QT_TRANSLATE_NOOP("DeviceList.h", "Location"),
};

const int					DEVICE_LIST_COLUMN_COUNT	= sizeof(DeviceListColumn)/sizeof(DeviceListColumn[0]);

const int					DEVICE_LIST_COLUMN_IMEI		= 0,
							DEVICE_LIST_COLUMN_LOCATION	= 1;

const int					DeviceListWidth[DEVICE_LIST_COLUMN_COUNT] =
{
							150,	// DEVICE_LIST_COLUMN_IMEI
							300,	// DEVICE_LIST_COLUMN_LOCATION
};

// ----------------------------------------------------------------------------------------------

class DeviceListModel : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit DeviceListModel(QObject* parent = nullptr);
	virtual ~DeviceListModel() override;

public:

	int count() const { return static_cast<int>(m_deviceCount); }

	QString text(int row, int column, DeviceParam* pDevice) const;

	bool append(DeviceParam* pDevice);
	DeviceParam* at(int index) const;
	void remove(quint64 imei);
	void update(quint64 imei);

	void set(const std::vector<DeviceParam*>& list_add);
	void clear();

private:

	mutable QMutex m_deviceMutex;
	std::vector<DeviceParam*> m_deviceList;
	int m_deviceCount = 0;

	int columnCount(const QModelIndex &parent) const override;
	int rowCount(const QModelIndex &parent=QModelIndex()) const override;

	void updateRow(int row);

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class DeviceParamDialog : public QDialog
{
	Q_OBJECT

public:

	explicit DeviceParamDialog(QWidget* parent = nullptr);
	explicit DeviceParamDialog(DeviceParam* pDevice, QWidget* parent = nullptr);
	virtual ~DeviceParamDialog() override;

public:

	DeviceParam* device() const { return m_pDevice; }

private:

	DeviceParam* m_pDevice;

	QLineEdit* m_pImeiEdit = nullptr;
	QLineEdit* m_pLocationEdit = nullptr;

	QDialogButtonBox* m_buttonBox = nullptr;

	void createInterface();
	void updateParams();

private slots:

	// slots of buttons
	//
	void onOk();
};

// ==============================================================================================

class DeviceListDialog : public QDialog
{
	Q_OBJECT

public:

	DeviceListDialog(QWidget* parent = nullptr);
	virtual ~DeviceListDialog() override;

private:

	QMenuBar* m_pMenuBar = nullptr;
	QMenu* m_pDeviceMenu = nullptr;
	QMenu* m_pEditMenu = nullptr;
	QMenu* m_pContextMenu = nullptr;

	QAction* m_pCreateAction = nullptr;
	QAction* m_pEditAction = nullptr;
	QAction* m_pRemoveAction = nullptr;

	QAction* m_pFindAction = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pSelectAllAction = nullptr;

	DeviceListModel m_model;
	QTableView* m_pView = nullptr;

	void createInterface();
	void createContextMenu();

public slots:

	// slots for updating
	//
	void deviceBaseLoaded();
	void updateList();

private slots:

	// slots of menu
	//
		// Devices
		//
	void OnNew();
	void onEdit();
	void onRremove();

		// Edit
		//
	void onFind();
	void onCopy();
	void onSelectAll();

	// slots for list
	//
	void onListDoubleClicked(const QModelIndex&);

	// slots for list
	//
	void onContextMenu(QPoint);
};

// ==============================================================================================

#endif // DEVICELISTDIALOG_H


