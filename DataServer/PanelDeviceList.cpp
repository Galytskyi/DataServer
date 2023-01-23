#include "PanelDeviceList.h"

#include "ProcessData.h"
#include "DeviceListDialog.h"
#include "DeviceSettingDialog.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PanelDeviceModel::PanelDeviceModel(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

PanelDeviceModel::~PanelDeviceModel()
{
	QMutexLocker l(&m_deviceMutex);

	m_deviceList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int PanelDeviceModel::columnCount(const QModelIndex&) const
{
	return PANEL_DEVICE_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int PanelDeviceModel::rowCount(const QModelIndex&) const
{
	return m_deviceCount;
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceModel::updateRow(int row)
{
	if (row < 0 || row >= m_deviceCount)
	{
		return;
	}

	for (int column = 0; column < PANEL_DEVICE_LIST_COLUMN_COUNT; column ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
	}
}

// -------------------------------------------------------------------------------------------------------------------

QVariant PanelDeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < PANEL_DEVICE_LIST_COLUMN_COUNT)
		{
			result = PanelDeviceListColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant PanelDeviceModel::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int rowIndex = index.row();
	if (rowIndex < 0 || rowIndex >= m_deviceCount)
	{
		return QVariant();
	}

	int columnIndex = index.column();
	if (columnIndex < 0 || columnIndex > PANEL_DEVICE_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	DataDevice* pDevice = at(rowIndex);
	if (pDevice == nullptr)
	{
		return QVariant();
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (columnIndex)
		{
			case PANEL_DEVICE_LIST_COLUMN_IMEI:		result = Qt::AlignCenter;	break;
			case PANEL_DEVICE_LIST_COLUMN_LOCATION:	result = Qt::AlignCenter;	break;
			case PANEL_DEVICE_LIST_COLUMN_IP:		result = Qt::AlignCenter;	break;
			case PANEL_DEVICE_LIST_COLUMN_TIME:		result = Qt::AlignCenter;	break;
			case PANEL_DEVICE_LIST_COLUMN_BRIGHT:	result = Qt::AlignCenter;	break;
			case PANEL_DEVICE_LIST_COLUMN_STATE:	result = Qt::AlignCenter;	break;

			default:
				assert(0);
		}

		return result;
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(rowIndex, columnIndex, pDevice);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString PanelDeviceModel::text(int row, int column, DataDevice* pDevice) const
{
	if (row < 0 || row >= m_deviceCount)
	{
		return QString();
	}

	if (column < 0 || column > PANEL_DEVICE_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (pDevice == nullptr)
	{
		return QString();
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case PANEL_DEVICE_LIST_COLUMN_IMEI:		result = pDevice->imeiStr(theOptions.imei().imeiDigits());	break;
		case PANEL_DEVICE_LIST_COLUMN_LOCATION:	result = pDevice->location();								break;
		case PANEL_DEVICE_LIST_COLUMN_IP:		result = pDevice->ip();										break;
		case PANEL_DEVICE_LIST_COLUMN_TIME:		result = timeToStr(pDevice->connectTime());					break;
		case PANEL_DEVICE_LIST_COLUMN_BRIGHT:	result = QString::number(pDevice->brightness());			break;
		case PANEL_DEVICE_LIST_COLUMN_STATE:	result = pDevice->stateStr();								break;

		default:
			assert(0);
	}

	return result;

}

// -------------------------------------------------------------------------------------------------------------------

bool PanelDeviceModel::append(DataDevice* pDevice)
{
	if (pDevice == nullptr)
	{
		return false;
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return false;
	}

	// append into DeviceList
	//
	int indexTable = m_deviceCount;

	beginInsertRows(QModelIndex(), indexTable, indexTable);

		m_deviceMutex.lock();

			m_deviceList.push_back(pDevice);
			m_deviceCount = static_cast<int>(m_deviceList.size());

		m_deviceMutex.unlock();

	endInsertRows();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

DataDevice* PanelDeviceModel::at(int index) const
{
	if (index < 0 || index >= m_deviceCount)
	{
		return nullptr;
	}

	QMutexLocker l(&m_deviceMutex);
	return m_deviceList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceModel::remove(quint64 imei)
{
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	// remove from DeviceList
	//
	QMutexLocker locker(&m_deviceMutex);

	auto foundImei = std::find_if(m_deviceList.begin(), m_deviceList.end(), [&imei](DataDevice* pDevice)
								   {
									   return pDevice->imei() == imei;
								   });

	if (foundImei == m_deviceList.end())
	{
		return;
	}

	int index = foundImei - m_deviceList.begin();

	beginRemoveRows(QModelIndex(), index, index);

		m_deviceList.erase(foundImei);
		m_deviceCount = static_cast<int>(m_deviceList.size());

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceModel::update(quint64 imei)
{
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	for(int index = 0; index < m_deviceCount; index++)
	{
		DataDevice* pDevice = at(index);
		if (pDevice == nullptr)
		{
			continue;
		}

		if (pDevice->imei() != imei)
		{
			continue;
		}

		updateRow(index);

		if (pDevice->state() == DataDeviceState::TimeoutResponse && pDevice->lastRequestType() == REQUEST_SET_DEVICE_PARAMS)
		{
			QMessageBox::critical(	nullptr,
									tr("Device settings"),
									tr("IMEI: %1\nError of request \"Set device settings\"!").arg(pDevice->imei()));
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceModel::set(const std::vector<DataDevice*>& list_add)
{
	quint64 count = list_add.size();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, static_cast<int>(count - 1));

		m_deviceMutex.lock();

			m_deviceList = list_add;
			m_deviceCount = static_cast<int>(m_deviceList.size());

		m_deviceMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceModel::clear()
{
	quint64 count = m_deviceCount;
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, static_cast<int>(count - 1));

		m_deviceMutex.lock();

			m_deviceList.clear();
			m_deviceCount = static_cast<int>(m_deviceList.size());

		m_deviceMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PanelDeviceList::PanelDeviceList(ServerSocket* pServerSocket, QWidget* parent) :
	QDockWidget(parent) ,
	m_pServerSocket(pServerSocket)
{
	setWindowTitle(tr("Device list panel"));
	setObjectName(windowTitle());

	createInterface();
	createContextMenu();

	if (pServerSocket == nullptr)
	{
		return;
	}
}

// -------------------------------------------------------------------------------------------------------------------

PanelDeviceList::~PanelDeviceList()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::createInterface()
{
	m_pDeviceWindow = new QMainWindow;

	//
	//
	m_pView = new QTableView(m_pDeviceWindow);
	m_pView->setModel(&m_model);
	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	for(int column = 0; column < PANEL_DEVICE_LIST_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, PanelDeviceListWidth[column]);
	}

	createHeaderContextMenu();

	//
	//
	m_pDeviceWindow->setCentralWidget(m_pView);

	setWidget(m_pDeviceWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::createContextMenu()
{
	if (m_pDeviceWindow == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr("&Devices"), m_pDeviceWindow);

	m_pRequestPictureAction = m_pContextMenu->addAction(tr("&Load picture"));
	m_pRequestPictureAction->setIcon(QIcon(":/icons/Load.png"));

	m_pBrightnessMenu = new QMenu(tr("&Brightness"), m_pDeviceWindow);

	m_pRequestBrightness000Action = m_pBrightnessMenu->addAction(tr("0%"));
	m_pRequestBrightness025Action = m_pBrightnessMenu->addAction(tr("25%"));
	m_pRequestBrightness050Action = m_pBrightnessMenu->addAction(tr("50%"));
	m_pRequestBrightness075Action = m_pBrightnessMenu->addAction(tr("75%"));
	m_pRequestBrightness100Action = m_pBrightnessMenu->addAction(tr("100%"));

	m_pContextMenu->addMenu(m_pBrightnessMenu);

	m_pDeviceSettingAction = m_pContextMenu->addAction(tr("&Device settings ..."));
	m_pDeviceSettingAction->setIcon(QIcon(":/icons/Settings.png"));

	m_pContextMenu->addSeparator();

	m_pSetLocatinAction = m_pContextMenu->addAction(tr("&Set Location ..."));
	m_pSetLocatinAction->setIcon(QIcon(":/icons/Location.png"));

	m_pContextMenu->addSeparator();

	m_pFindAction = m_pContextMenu->addAction(tr("&Find"));
	m_pFindAction->setIcon(QIcon(":/icons/Find.png"));

	m_pContextMenu->addSeparator();

	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pSelectAllAction = m_pContextMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));

	connect(m_pRequestPictureAction, &QAction::triggered, this, &PanelDeviceList::onRequestPicture, Qt::QueuedConnection);

	connect(m_pRequestBrightness000Action, &QAction::triggered, this, &PanelDeviceList::onRequestBrightness, Qt::QueuedConnection);
	connect(m_pRequestBrightness025Action, &QAction::triggered, this, &PanelDeviceList::onRequestBrightness, Qt::QueuedConnection);
	connect(m_pRequestBrightness050Action, &QAction::triggered, this, &PanelDeviceList::onRequestBrightness, Qt::QueuedConnection);
	connect(m_pRequestBrightness075Action, &QAction::triggered, this, &PanelDeviceList::onRequestBrightness, Qt::QueuedConnection);
	connect(m_pRequestBrightness100Action, &QAction::triggered, this, &PanelDeviceList::onRequestBrightness, Qt::QueuedConnection);

	connect(m_pDeviceSettingAction, &QAction::triggered, this, &PanelDeviceList::onDeviceSetting, Qt::QueuedConnection);

	connect(m_pSetLocatinAction, &QAction::triggered, this, &PanelDeviceList::onSetLocation, Qt::QueuedConnection);

	connect(m_pFindAction, &QAction::triggered, this, &PanelDeviceList::onFind, Qt::QueuedConnection);
	connect(m_pCopyAction, &QAction::triggered, this, &PanelDeviceList::onCopy, Qt::QueuedConnection);
	connect(m_pSelectAllAction, &QAction::triggered, this, &PanelDeviceList::onSelectAll, Qt::QueuedConnection);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &PanelDeviceList::onContextMenu, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::createHeaderContextMenu()
{
	if (m_pView == nullptr)
	{
		return;
	}

	m_headerContextMenu = new QMenu(m_pView);
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &PanelDeviceList::onHeaderContextMenu);

	for(int column = 0; column < PANEL_DEVICE_LIST_COLUMN_COUNT; column++)
	{
		QAction* pAction = m_headerContextMenu->addAction(qApp->translate("PanelDeviceList.h", PanelDeviceListColumn[column]));
		if (pAction == nullptr)
		{
			continue;
		}

		pAction->setCheckable(true);
		pAction->setChecked(true);

		m_pColumnActionList.push_back(pAction);
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered),	this, &PanelDeviceList::onColumnAction);

	hideColumn(PANEL_DEVICE_LIST_COLUMN_IP, true);
	hideColumn(PANEL_DEVICE_LIST_COLUMN_BRIGHT, true);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::hideColumn(int column, bool hide)
{
	int columnCount = static_cast<int>(m_pColumnActionList.size());
	if (column < 0 || column >= columnCount)
	{
		return;
	}

	if (hide == true)
	{
		m_pView->hideColumn(column);
		m_pColumnActionList[static_cast<quint64>(column)]->setChecked(false);
	}
	else
	{
		m_pView->showColumn(column);
		m_pColumnActionList[static_cast<quint64>(column)]->setChecked(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onRequestPicture()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int deviceCount = m_model.count();
	if (deviceCount == 0)
	{
		return;
	}

	std::vector<quint64> imeiList;

	QModelIndexList selectedList = m_pView->selectionModel()->selectedRows();
	std::sort(selectedList.begin(), selectedList.end());
	for(auto selectedIndex : selectedList)
	{
		int index = selectedIndex.row();
		if (index < 0 || index >= m_model.count())
		{
			continue;
		}

		DataDevice* pDevice = m_model.at(index);
		if (pDevice == nullptr)
		{
			continue;
		}

		if (pDevice->imei() == UNDEFINED_IMEI)
		{
			continue;
		}

		imeiList.push_back(pDevice->imei());
	}

	if (imeiList.size() == 0)
	{
		return;
	}

	emit requestPicture(imeiList);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onRequestBrightness()
{
	QAction* action = dynamic_cast<QAction*>(sender());
	if (action == nullptr)
	{
		return ;
	}

	int brightness = 128;

	if (action == m_pRequestBrightness000Action)
	{
		brightness = 0;
	}

	if (action == m_pRequestBrightness025Action)
	{
		brightness = 64;
	}

	if (action == m_pRequestBrightness050Action)
	{
		brightness = 128;
	}

	if (action == m_pRequestBrightness075Action)
	{
		brightness = 192;
	}

	if (action == m_pRequestBrightness100Action)
	{
		brightness = 255;
	}

	if (m_pView == nullptr)
	{
		return;
	}

	const QModelIndexList selectedList = m_pView->selectionModel()->selectedRows();
	if (selectedList.isEmpty() == true)
	{
		return;
	}

	auto selectedIndex = selectedList.first();
	if (selectedIndex.isValid() == false)
	{
		return;
	}

	int deviceCount = m_model.count();
	if (deviceCount == 0)
	{
		return;
	}

	int index = selectedIndex.row();
	if (index < 0 || index >= deviceCount)
	{
		return;
	}

	DataDevice* pDevice = m_model.at(index);
	if (pDevice == nullptr)
	{
		return;
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return;
	}

	pDevice->setBrightness(brightness);

	emit requestBrightness(pDevice->imei(), pDevice->brightness());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onDeviceSetting()
{
	if (m_pView == nullptr)
	{
		return;
	}

	const QModelIndexList selectedList = m_pView->selectionModel()->selectedRows();
	if (selectedList.isEmpty() == true)
	{
		return;
	}

	auto selectedIndex = selectedList.first();
	if (selectedIndex.isValid() == false)
	{
		return;
	}

	int deviceCount = m_model.count();
	if (deviceCount == 0)
	{
		return;
	}

	int index = selectedIndex.row();
	if (index < 0 || index >= deviceCount)
	{
		return;
	}

	DataDevice* pDevice = m_model.at(index);
	if (pDevice == nullptr)
	{
		return;
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return;
	}

	DeviceSettingDialog dialog(pDevice->brightness(), this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	pDevice->setBrightness(dialog.brightness());

	emit requestBrightness(pDevice->imei(), pDevice->brightness());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onSetLocation()
{
	const QModelIndexList selectedList = m_pView->selectionModel()->selectedRows();
	if (selectedList.isEmpty() == true)
	{
		return;
	}

	auto selectedIndex = selectedList.first();
	if (selectedIndex.isValid() == false)
	{
		return;
	}

	int deviceCount = m_model.count();
	if (deviceCount == 0)
	{
		return;
	}

	int index = selectedIndex.row();
	if (index < 0 || index >= deviceCount)
	{
		return;
	}

	DataDevice* pDataDevice = m_model.at(index);
	if (pDataDevice == nullptr)
	{
		return;
	}

	if (pDataDevice->imei() == UNDEFINED_IMEI)
	{
		return;
	}

	DeviceParam* pDevice = nullptr;

	index = theDeviceBase.find(pDataDevice->imei());
	if (index == -1)
	{
		pDevice = new DeviceParam(pDataDevice->imei());

		int indexDB = theDeviceBase.append(pDevice);
		if (indexDB == -1)
		{
			return;
		}
	}
	else
	{
		pDevice = theDeviceBase.device(index);
	}

	if (pDevice == nullptr)
	{
		return;
	}

	DeviceParamDialog dialog(pDevice, this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return;
	}

	bool result = theDeviceBase.update_db(pDevice);
	if (result == false)
	{
		QMessageBox::critical(this, tr("Update devices"), tr("Error update devices in database!"));
		return;
	}

	pDataDevice->setLocation(pDevice->location());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onFind()
{
	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onCopy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onSelectAll()
{
	if (m_pView == nullptr)
	{
		return;
	}

	m_pView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	int columnCount = static_cast<int>(m_pColumnActionList.size());
	for(int column = 0; column < columnCount; column++)
	{
		if (m_pColumnActionList[static_cast<quint64>(column)] == action)
		{
			hideColumn(column, !action->isChecked());

			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool PanelDeviceList::append(DataDevice* pDevice)
{
	if (m_pView == nullptr)
	{
		return false;
	}

	if (pDevice == nullptr)
	{
		return false;
	}

	if (pDevice->imei() == UNDEFINED_IMEI)
	{
		return false;
	}

	bool result = m_model.append(pDevice);

	if (m_model.count() == 1)
	{
		m_pView->selectRow(0);
	}

	int index = theDeviceBase.find(pDevice->imei());
	if (index != -1)
	{
		DeviceParam* pDeviceParam = theDeviceBase.device(index);
		if (pDeviceParam == nullptr)
		{
			return result;
		}

		if (pDeviceParam->imei() == UNDEFINED_IMEI)
		{
			return result;
		}

		if (pDeviceParam->imei() != pDevice->imei())
		{
			return result;
		}

		pDevice->setLocation(pDeviceParam->location());
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::remove(quint64 imei)
{
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	m_model.remove(imei);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelDeviceList::update(quint64 imei)
{
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	m_model.update(imei);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
