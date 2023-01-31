#ifndef DEVICEBASE_H
#define DEVICEBASE_H

#include <QObject>

#include "DevicePacket.h"

// ==============================================================================================

class DeviceParam
{
public:

	DeviceParam();
	DeviceParam(quint64 imei);
	virtual ~DeviceParam();

public:

	void virtual clear();

	int deviceID() const { return m_deviceID; }
	void setDeviceID(int id) { m_deviceID = id; }

	quint64 imei() const { return m_imei; }
	QString imeiStr(int digits) const;
	void setImei(quint64 imei) { m_imei = imei; }

	QString location() const { return m_location; }
	void setLocation(const QString& location) { m_location = location; }

	int brightness() const { return m_brightness; }
	void setBrightness(int brightness) { m_brightness = brightness; }

private:

	int m_deviceID = -1;											// primary key of record in SQL table

	quint64 m_imei = UNDEFINED_IMEI;
	QString m_location;
	int m_brightness = 0;
};

// ==============================================================================================

class DeviceBase : public QObject
{
	Q_OBJECT

public:

	explicit DeviceBase(QObject* parent = nullptr);
	virtual ~DeviceBase() override;

public:

	int count() const;
	void clear();

	int load();

	int append(DeviceParam* pDevice);
	bool append_db(DeviceParam* pDevice);
	DeviceParam* device(int index) const;
	bool remove(const std::vector<quint64>& imeiList);
	bool update_db(DeviceParam* pDevice);

	int find(quint64 imei);

private:

	mutable QMutex m_deviceMutex;
	std::vector<DeviceParam*> m_deviceList;

signals:

	void deviceBaseLoaded(const std::vector<DeviceParam*>& list);

public slots:

	void appendToBase(DeviceParam* pDevice);
	void removeFromBase(const std::vector<quint64>& imeiList);
};

// ==============================================================================================

extern DeviceBase theDeviceBase;

// ==============================================================================================

#endif // DEVICEBASE_H
