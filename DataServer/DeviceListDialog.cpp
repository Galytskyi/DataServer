#include "DeviceListDialog.h"

#include "ProcessData.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DeviceListModel::DeviceListModel(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

DeviceListModel::~DeviceListModel()
{
	QMutexLocker l(&m_deviceMutex);

	m_deviceList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int DeviceListModel::columnCount(const QModelIndex&) const
{
	return DEVICE_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int DeviceListModel::rowCount(const QModelIndex&) const
{
	return m_deviceCount;
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListModel::updateRow(int row)
{
	if (row < 0 || row >= m_deviceCount)
	{
		return;
	}

	for (int column = 0; column < DEVICE_LIST_COLUMN_COUNT; column ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
	}
}

// -------------------------------------------------------------------------------------------------------------------

QVariant DeviceListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < DEVICE_LIST_COLUMN_COUNT)
		{
			result = DeviceListColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
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
	if (columnIndex < 0 || columnIndex > DEVICE_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	DeviceParam* pDevice = at(rowIndex);
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
			case DEVICE_LIST_COLUMN_IMEI:		result = Qt::AlignCenter;	break;
			case DEVICE_LIST_COLUMN_LOCATION:	result = Qt::AlignCenter;	break;
			case DEVICE_LIST_COLUMN_BRIGHT:		result = Qt::AlignCenter;	break;

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

QString DeviceListModel::text(int row, int column, DeviceParam* pDevice) const
{
	if (row < 0 || row >= m_deviceCount)
	{
		return QString();
	}

	if (column < 0 || column > DEVICE_LIST_COLUMN_COUNT)
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
		case DEVICE_LIST_COLUMN_IMEI:		result = pDevice->imeiStr(DEVICE_IMEI_SIZE);		break;
		case DEVICE_LIST_COLUMN_LOCATION:	result = pDevice->location();						break;
		case DEVICE_LIST_COLUMN_BRIGHT:		result = pDevice->brightnessStr();					break;

		default:
			assert(0);
	}

	return result;

}

// -------------------------------------------------------------------------------------------------------------------

bool DeviceListModel::append(DeviceParam* pDevice)
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

DeviceParam* DeviceListModel::at(int index) const
{
	if (index < 0 || index >= m_deviceCount)
	{
		return nullptr;
	}

	QMutexLocker l(&m_deviceMutex);
	return m_deviceList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListModel::remove(quint64 imei)
{
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	// remove from DeviceList
	//
	QMutexLocker locker(&m_deviceMutex);

	auto foundImei = std::find_if(m_deviceList.begin(), m_deviceList.end(), [&imei](DeviceParam* pDevice)
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

void DeviceListModel::update(quint64 imei)
{
	if (imei == UNDEFINED_IMEI)
	{
		return;
	}

	for(int index = 0; index < m_deviceCount; index++)
	{
		DeviceParam* pDevice = at(index);
		if (pDevice == nullptr)
		{
			continue;
		}

		if (pDevice->imei() != imei)
		{
			continue;
		}

		updateRow(index);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListModel::set(const std::vector<DeviceParam*>& list_add)
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

void DeviceListModel::clear()
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

DeviceParamDialog::DeviceParamDialog(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
	updateParams();
}


// -------------------------------------------------------------------------------------------------------------------

DeviceParamDialog::DeviceParamDialog(DeviceParam* pDevice, QWidget* parent) :
	QDialog(parent) ,
	m_pDevice(pDevice)
{
	if (m_pDevice == nullptr)
	{
		return;
	}

	createInterface();
	updateParams();
}

// -------------------------------------------------------------------------------------------------------------------

DeviceParamDialog::~DeviceParamDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceParamDialog::createInterface()
{
	if (m_pDevice == nullptr)
	{
		return;
	}

	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Device.png"));
	setWindowTitle(tr("Device"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	resize(static_cast<int>(screen.width() * 0.20), static_cast<int>(screen.height() * 0.10));
	move(screen.center() - rect().center());

	// Params
	//
	QGroupBox* paramsGroup = new QGroupBox(QString());
	QVBoxLayout* paramsLayout = new QVBoxLayout;

		// Imei
		//
	QHBoxLayout* imeiLayout = new QHBoxLayout;

	QLabel* pImeiLabel = new QLabel(tr("IMEI"), this);
	m_pImeiEdit = new QLineEdit(QString(), this);

	pImeiLabel->setFixedWidth(100);


	imeiLayout->addWidget(pImeiLabel);
	imeiLayout->addWidget(m_pImeiEdit);

		// Location
		//
	QHBoxLayout* locationLayout = new QHBoxLayout;

	QLabel* pLocationLabel = new QLabel(tr("Location"), this);
	m_pLocationEdit = new QLineEdit(QString(), this);


	pLocationLabel->setFixedWidth(100);

	locationLayout->addWidget(pLocationLabel);
	locationLayout->addWidget(m_pLocationEdit);

	//
	//
	paramsLayout->addLayout(imeiLayout);
	paramsLayout->addLayout(locationLayout);

	paramsGroup->setLayout(paramsLayout);

	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	// connects
	//
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DeviceParamDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DeviceParamDialog::reject);

	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addWidget(paramsGroup);
	mainLayout->addStretch();
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	if (m_pDevice->imei() == UNDEFINED_IMEI)
	{
		m_pImeiEdit->setReadOnly(false);
		m_pImeiEdit->setDisabled(false);

		m_pImeiEdit->setFocus();
	}
	else
	{
		m_pImeiEdit->setReadOnly(true);
		m_pImeiEdit->setDisabled(true);

		m_pLocationEdit->setFocus();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceParamDialog::updateParams()
{
	if (m_pDevice == nullptr)
	{
		return;
	}

	if (m_pImeiEdit == nullptr || m_pLocationEdit == nullptr)
	{
		return;
	}

	m_pImeiEdit->setText(m_pDevice->imeiStr(DEVICE_IMEI_SIZE));
	m_pLocationEdit->setText(m_pDevice->location());
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceParamDialog::onOk()
{
	if (m_pDevice == nullptr)
	{
		return;
	}

	//
	//
	if (m_pImeiEdit == nullptr || m_pLocationEdit == nullptr)
	{
		return;
	}

	//
	//
	QString imei = m_pImeiEdit->text();
	if (imei.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("IMEI is empty!"));
		m_pImeiEdit->setFocus();
		return;
	}

	if (imei.toULongLong() == UNDEFINED_IMEI)
	{
		QMessageBox::information(this, windowTitle(), tr("IMEI is not correct!"));
		m_pImeiEdit->setFocus();
		return;
	}

	QString location = m_pLocationEdit->text().trimmed();
	if (location.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Location is empty!"));
		m_pLocationEdit->setFocus();
		return;
	}

	//
	//
	m_pDevice->setImei(imei.toULongLong());
	m_pDevice->setLocation(location);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DeviceListDialog::DeviceListDialog(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

DeviceListDialog::~DeviceListDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Device.png"));
	setWindowTitle(tr("Devices"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	resize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.5));
	move(screen.center() - rect().center());

	m_pMenuBar = new QMenuBar(this);

	// menu
	//
	m_pDeviceMenu = new QMenu(tr("&Device"), this);

	// action
	//
	m_pCreateAction = m_pDeviceMenu->addAction(tr("&Create ..."));
	m_pCreateAction->setIcon(QIcon(":/icons/Add.png"));
	m_pCreateAction->setShortcut(Qt::Key_Insert);

	m_pEditAction = m_pDeviceMenu->addAction(tr("&Edit ..."));
	m_pEditAction->setIcon(QIcon(":/icons/Edit.png"));

	m_pRemoveAction = m_pDeviceMenu->addAction(tr("&Remove"));
	m_pRemoveAction->setIcon(QIcon(":/icons/Remove.png"));
	m_pRemoveAction->setShortcut(Qt::Key_Delete);

	m_pEditMenu = new QMenu(tr("&Edit"), this);

	m_pFindAction = m_pEditMenu->addAction(tr("&Find"));
	m_pFindAction->setIcon(QIcon(":/icons/Find.png"));
	m_pFindAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_F});

	m_pEditMenu->addSeparator();

	m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	m_pCopyAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_C});

	m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_pSelectAllAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_A});

	//
	//
	m_pMenuBar->addMenu(m_pDeviceMenu);
	m_pMenuBar->addMenu(m_pEditMenu);

	//
	//
	connect(m_pCreateAction, &QAction::triggered, this, &DeviceListDialog::OnNew);
	connect(m_pEditAction, &QAction::triggered, this, &DeviceListDialog::onEdit);
	connect(m_pRemoveAction, &QAction::triggered, this, &DeviceListDialog::onRremove);

	connect(m_pCopyAction, &QAction::triggered, this, &DeviceListDialog::onCopy, Qt::QueuedConnection);
	connect(m_pFindAction, &QAction::triggered, this, &DeviceListDialog::onFind, Qt::QueuedConnection);
	connect(m_pSelectAllAction, &QAction::triggered, this, &DeviceListDialog::onSelectAll, Qt::QueuedConnection);

	//
	//
	m_pView = new QTableView(this);

	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	connect(m_pView, &QTableView::doubleClicked , this, &DeviceListDialog::onListDoubleClicked);
	m_pView->setModel(&m_model);

	for(int column = 0; column < DEVICE_LIST_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, DeviceListWidth[column]);
	}

	//
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pView);

	setLayout(mainLayout);

	//
	//
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::createContextMenu()
{
	if (m_pView == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &DeviceListDialog::onContextMenu);

	m_pContextMenu->addAction(m_pCreateAction);
	m_pContextMenu->addAction(m_pEditAction);
	m_pContextMenu->addAction(m_pRemoveAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pCopyAction);
	m_pContextMenu->addAction(m_pSelectAllAction);
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::deviceBaseLoaded()
{
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::updateList()
{
	m_model.clear();

	std::vector<DeviceParam*> deviceList;

	int count = theDeviceBase.count();
	deviceList.reserve(count);

	for(int i = 0; i < count; i++)
	{
		deviceList.push_back(theDeviceBase.device(i));
	}

	m_model.set(deviceList);
}


// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::OnNew()
{
	DeviceParam* pDevice = new DeviceParam;
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

	int found = theDeviceBase.find(pDevice->imei());
	if (found != -1)
	{
		QMessageBox::information(this, windowTitle(), tr("Device with IMEI: %1, already exist!").arg(pDevice->imei()));
		return;
	}

	theDeviceBase.append(pDevice);

	m_model.append(pDevice);
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::onEdit()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_model.count())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select device for edit!"));
		return;
	}

	DeviceParam* pDevice = m_model.at(index);
	if (pDevice == nullptr)
	{
		return;
	}

	DeviceParamDialog dialog(pDevice, this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	bool result = theDeviceBase.update_db(pDevice);
	if (result == false)
	{
		QMessageBox::critical(this, tr("Update devices"), tr("Error update devices in database!"));
		return;
	}

	emit deviceLocationChanged(pDevice->imei(), pDevice->location());
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::onRremove()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int selectedConnectionCount = static_cast<int>(m_pView->selectionModel()->selectedRows().count());
	if (selectedConnectionCount == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select device for remove!"));
		return;
	}

	if (QMessageBox::question(this,
							  windowTitle(), tr("Do you want delete %1 device(s)?").
							  arg(selectedConnectionCount)) == QMessageBox::No)
	{
		return;
	}

	std::vector<DeviceParam*> deviceList;
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

		DeviceParam* pDevice = m_model.at(index);
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

	// remove from database
	//
	bool result = theDeviceBase.remove(imeiList);
	if (result == false)
	{
		QMessageBox::critical(this, tr("Delete devices"), tr("Error remove devices!"));
		return;
	}

	// remove from model
	//
	for(quint64 imei : imeiList)
	{
		m_model.remove(imei);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::onFind()
{
	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::onCopy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::onSelectAll()
{
	if (m_pView == nullptr)
	{
		return;
	}

	m_pView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::onListDoubleClicked(const QModelIndex&)
{
	onEdit();
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceListDialog::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------


