#ifndef DATADEVICE_H
#define DATADEVICE_H

#include <QObject>

#include "DevicePacket.h"
#include "PacketBase.h"

// ==============================================================================================

enum DataDeviceState
{
	WaitRequest = 0,
	WaitResponse = 1,
	ReceivingData = 2,
	ErrorSentRequest = 3,
	TimeoutResponse = 4,
};

const int DataDeviceStateCount = 5;

#define ERR_DATA_DEVICE_STATE(state) (static_cast<int>(state) < 0 || static_cast<int>(state) >= DataDeviceStateCount)

QString DataDeviceStateCaption(int state);

// ==============================================================================================

class DataDevice
{
public:

	explicit DataDevice();
	virtual ~DataDevice();

public:

	void virtual clear();

	DeviceRequestGetInfo deviceInfo() const { return m_di; }
	void setDeviceInfo(const DeviceRequestGetInfo& di);

	quint64 imei() const { return m_imei; }
	QString imeiStr(int digits) const;
	void setImei(quint64 imei) { m_imei = imei; }

	QString location() const { return m_location; }
	void setLocation(const QString& location) { m_location = location; }

	QString ip() const { return m_ip; }
	void setIp(const QString& ip) { m_ip = ip; }

	QDateTime connectTime() const { return m_connectTime; }
	void setConnectTime(const QDateTime& time) { m_connectTime = time; }

	ImageType imageType() const { return static_cast<ImageType>(m_di.imageType); }
	void setImageType(ImageType type) { m_di.imageType = type; }
	void setImageType(int type) { m_di.imageType = static_cast<ImageType>(type); }

	int imageSize() const { return m_di.imageSize; }
	void setImageSize(int size) { m_di.imageSize = size; }

	int imageWidth() const { return m_di.imageWidth; }
	void setImageWidth(int width) { m_di.imageWidth = width; }

	int imageHeight() const { return m_di.imageHeight; }
	void setImageHeight(int height) { m_di.imageHeight = height; }

	int brightness() const { return m_di.brightness; }
	QString brightnessStr() const;
	void setBrightness(int brightness) { m_di.brightness = brightness; }

	DataDeviceState state() const { return m_state; }
	QString stateStr() const;
	void setState(DataDeviceState state) { m_state = state; }

	int receivedBytes() const { return m_receivedBytes; }
	void setReceivedBytes(int bytes) { m_receivedBytes = bytes; }

	int lastRequestType() const { return m_lastRequestType; }
	void setLastRequestType(int requestType) { m_lastRequestType = requestType; }

	//DataDevice& operator=(DataDevice& from);

private:

	DeviceRequestGetInfo m_di;
	quint64 m_imei = UNDEFINED_IMEI;

	QString m_location;
	QString m_ip;
	QDateTime m_connectTime;										// time when device was connected

	DataDeviceState m_state = DataDeviceState::WaitRequest;
	int m_receivedBytes = 0;

	int m_lastRequestType = REQUEST_GET_DEVICE_INFO;
};

// ==============================================================================================

#endif // DEVICEBASE_H
