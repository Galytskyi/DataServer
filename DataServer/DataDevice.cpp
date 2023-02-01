#include "DataDevice.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DataDevice::DataDevice()
{
	DataDevice::clear();
}

// -------------------------------------------------------------------------------------------------------------------

DataDevice::~DataDevice()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DataDevice::clear()
{
	m_imei = UNDEFINED_IMEI;

	m_location = QT_TRANSLATE_NOOP("DataDevice", "Unknown");
	m_ip.clear();
	m_connectTime.setDate(QDate());

	m_state = DataDeviceState::WaitRequest;
	m_receivedBytes = 0;

	m_lastRequestType = REQUEST_GET_DEVICE_INFO;
}

// -------------------------------------------------------------------------------------------------------------------

void DataDevice::setDeviceInfo(const DeviceRequestGetInfo& di)
{
	m_imei = _strtoi64((const char*)di.IMEI, nullptr, 10);
	if (m_imei == UNDEFINED_IMEI)
	{
		return;
	}

	m_di = di;
}

// -------------------------------------------------------------------------------------------------------------------

QString DataDevice::imeiStr(int digits) const
{
	if (m_imei == UNDEFINED_IMEI)
	{
		return QT_TRANSLATE_NOOP("DataDevice", "Unknown");
	}

	return QString::number(m_imei).right(digits);
}

// -------------------------------------------------------------------------------------------------------------------

QString DataDevice::brightnessStr() const
{
	double result = round(static_cast<double>(m_di.brightness) / 255.0 * 100.0);

	return QString::number(result, 'f', 0) + " %";
}

// -------------------------------------------------------------------------------------------------------------------

QString DataDevice::stateStr() const
{
	QString result;

	switch (m_state)
	{
		case DataDeviceState::WaitRequest:
			result = QT_TRANSLATE_NOOP("DataDevice", "Ready");
			break;

		case DataDeviceState::WaitResponse:
			result = QT_TRANSLATE_NOOP("DataDevice", "Wait response ...");
			break;

		case DataDeviceState::ReceivingData:

			if (m_di.imageSize == 0)
			{
				result = QString("0 %");
				break;
			}

			result = QString("%1 %").arg(m_receivedBytes * 100 / m_di.imageSize);
			break;

		case DataDeviceState::ErrorSentRequest:
			result = QT_TRANSLATE_NOOP("DataDevice", "Error: Request was not sent.");
			break;

		case DataDeviceState::TimeoutResponse:
			result = QT_TRANSLATE_NOOP("DataDevice", "Error: Timeout of response.");
			break;

		default:
			result = QT_TRANSLATE_NOOP("DataDevice", "Unknown");
			break;
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QString DataDeviceStateCaption(int state)
{
	QString caption;

	switch (state)
	{
		case DataDeviceState::WaitRequest:		caption = QT_TRANSLATE_NOOP("DataDevice", "Ready");				break;
		case DataDeviceState::WaitResponse:		caption = QT_TRANSLATE_NOOP("DataDevice", "Wait response ...");	break;
		case DataDeviceState::ReceivingData:	caption = QT_TRANSLATE_NOOP("DataDevice", "Receiving data");	break;

		default:
			Q_ASSERT(0);
			caption = QT_TRANSLATE_NOOP("DataDevice", "Unknown");
	}

	return caption;
};

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
