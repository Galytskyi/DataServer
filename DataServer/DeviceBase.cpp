#include "DeviceBase.h"

#include "Database.h"

// -------------------------------------------------------------------------------------------------------------------

DeviceBase theDeviceBase;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DeviceParam::DeviceParam()
{
	DeviceParam::clear();
}

// -------------------------------------------------------------------------------------------------------------------

DeviceParam::DeviceParam(quint64 imei)
{
	DeviceParam::clear();

	m_imei = imei;
}

// -------------------------------------------------------------------------------------------------------------------

DeviceParam::~DeviceParam()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceParam::clear()
{
	m_deviceID = -1;

	m_imei = UNDEFINED_IMEI;
	m_location = QT_TRANSLATE_NOOP("DeviceParam", "Unknown");;
	m_brightness = 0;
}

// -------------------------------------------------------------------------------------------------------------------

QString DeviceParam::imeiStr(int digits) const
{
	return QString::number(m_imei).right(digits);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DeviceBase::DeviceBase(QObject* parent)
	: QObject{parent}
{
}

// -------------------------------------------------------------------------------------------------------------------

 DeviceBase::~DeviceBase()
 {
 }

 // -------------------------------------------------------------------------------------------------------------------

 int DeviceBase::count() const
 {
	 QMutexLocker locker(&m_deviceMutex);

	 return static_cast<int>(m_deviceList.size());
 }

 // -------------------------------------------------------------------------------------------------------------------

 void DeviceBase::clear()
 {
	 QMutexLocker locker(&m_deviceMutex);

	 qDeleteAll(m_deviceList);
 }

 // -------------------------------------------------------------------------------------------------------------------

 int DeviceBase::load()
 {
	 QElapsedTimer responseTime;
	 responseTime.start();

	 if (theDatabase.isOpen() == false)
	 {
		 return false;
	 }

	 SqlTable* pTable = theDatabase.openTable(SQL_TABLE_DEVICE);
	 if (pTable == nullptr)
	 {
		 return false;
	 }

	 m_deviceMutex.lock();

		 int recordCount = pTable->recordCount();
		 if (recordCount > 0)
		 {
			 m_deviceList.resize(static_cast<quint64>(recordCount));

			 DeviceParam* pDeviceList = new DeviceParam[static_cast<quint64>(recordCount)];;
			 if (pDeviceList != nullptr)
			 {
				int readedCount = pTable->read(pDeviceList);
				if (readedCount == recordCount)
				{
					for(int i = 0; i < recordCount; i++)
					{
						m_deviceList[i] = pDeviceList + i;
					}
				}
				else
				{
					assert(0);
					qDebug() << __FUNCTION__ << ": Error of reading" <<
								" readed: " << readedCount <<
								" form: " << recordCount;

					delete [] pDeviceList;
				}
			 }
		 }

	 m_deviceMutex.unlock();

	 pTable->close();

	 qDebug() << __FUNCTION__ << ": Devices"		<<
				 ", Loaded records: " << count()	<<
				 ", Time for load: " << responseTime.elapsed() << " ms";

	 emit deviceBaseLoaded(m_deviceList);

	 return count();
 }

 // -------------------------------------------------------------------------------------------------------------------

 int DeviceBase::append(DeviceParam* pDevice)
 {
	 if (pDevice == nullptr)
	 {
		 return -1;
	 }

	 bool result = append_db(pDevice);
	 if (result == false)
	 {
		 return -1;
	 }

	 int index = -1;

	 m_deviceMutex.lock();

		 m_deviceList.push_back(pDevice);
		 index = static_cast<int>(m_deviceList.size() - 1);

	 m_deviceMutex.unlock();

	 return index;
 }

 // -------------------------------------------------------------------------------------------------------------------

 bool DeviceBase::append_db(DeviceParam* pDevice)
 {
	 if (pDevice == nullptr)
	 {
		 return false;
	 }

	 if (theDatabase.isOpen() == false)
	 {
		 return false;
	 }

	 SqlTable* pTable = theDatabase.openTable(SQL_TABLE_DEVICE);
	 if (pTable == nullptr)
	 {
		 return false;
	 }

	 int count = pTable->write(pDevice);

	 pTable->close();

	 if (count != 1)
	 {
		 return false;
	 }

	 return true;
 }

 // -------------------------------------------------------------------------------------------------------------------

 void DeviceBase::appendToBase(DeviceParam* pDevice)
 {
	 if (pDevice == nullptr)
	 {
		 return;
	 }

	 int index = append(pDevice);
	 if (index == -1)
	 {
		 QMessageBox::critical(nullptr, tr("Save devices"), tr("Error saving devices to memory"));
		 return;
	 }
 }

 // ----------------------------------------------------------------------------------------------

 DeviceParam* DeviceBase::device(int index) const
 {
	 QMutexLocker locker(&m_deviceMutex);

	 if (index < 0 || index >= static_cast<int>(m_deviceList.size()))
	 {
		 return nullptr;
	 }

	 return m_deviceList[static_cast<quint64>(index)];
 }

 // -------------------------------------------------------------------------------------------------------------------

 bool DeviceBase::remove(const std::vector<quint64>& imeiList)
 {
	 int imeiCount = static_cast<int>(imeiList.size());
	 if (imeiCount == 0)
	 {
		 return false;
	 }

	 if (theDatabase.isOpen() == false)
	 {
		 return false;
	 }

	 SqlTable* pTable = theDatabase.openTable(SQL_TABLE_DEVICE);
	 if (pTable == nullptr)
	 {
		 return false;
	 }

	 int removed = 0;

	 m_deviceMutex.lock();

		 for (quint64 imei : imeiList)
		 {
			 auto foundImei = std::find_if(m_deviceList.begin(), m_deviceList.end(), [&imei](DeviceParam* pDevice)
											 {
												 return pDevice->imei() == imei;
											 });

			 if (foundImei == m_deviceList.end())
			 {
				 continue;
			 }

			 DeviceParam* pDevice = *foundImei;
			 if (pDevice == nullptr)
			 {
				 continue;
			 }

			 int count = pTable->remove(pDevice->deviceID());
			 if (count != 1)
			 {
				 continue;
			 }

			 m_deviceList.erase(foundImei);

			 removed++;
		 }

	 m_deviceMutex.unlock();

	 pTable->close();

	 if (removed != imeiCount)
	 {
		 return false;
	 }

	 return true;
 }

 // -------------------------------------------------------------------------------------------------------------------

 bool DeviceBase::update_db(DeviceParam* pDevice)
 {
	 if (pDevice == nullptr)
	 {
		 return false;
	 }

	 if (pDevice->deviceID() == SQL_INVALID_KEY)
	 {
		 return false;
	 }

	 if (theDatabase.isOpen() == false)
	 {
		 return false;
	 }

	 SqlTable* pTable = theDatabase.openTable(SQL_TABLE_DEVICE);
	 if (pTable == nullptr)
	 {
		 return false;
	 }

	 int count = pTable->write(pDevice, 1, pDevice->deviceID());

	 pTable->close();

	 if (count != 1)
	 {
		 return false;
	 }

	 return true;
 }

 // -------------------------------------------------------------------------------------------------------------------

 void DeviceBase::removeFromBase(const std::vector<quint64>& imeiList)
 {
	 bool result = remove(imeiList);
	 if (result == false)
	 {
		 QMessageBox::critical(nullptr, tr("Delete devices"), tr("Error remove devices from memory"));
	 }
 }

 // -------------------------------------------------------------------------------------------------------------------

 int DeviceBase::find(quint64 imei)
 {
	 QMutexLocker locker(&m_deviceMutex);

	 auto foundImei = std::find_if(m_deviceList.begin(), m_deviceList.end(), [&imei](DeviceParam* pDevice)
									{
										return pDevice->imei() == imei;
									});

	 if (foundImei == m_deviceList.end())
	 {
		 return -1;
	 }

	 return foundImei - m_deviceList.begin();
 }

 // -------------------------------------------------------------------------------------------------------------------
 // -------------------------------------------------------------------------------------------------------------------
 // -------------------------------------------------------------------------------------------------------------------
