#ifndef DEVICEPACKET_H
#define DEVICEPACKET_H

// ==============================================================================================

const quint32 DEVICE_SIGNATURE = 0x1614081E;	// 30-08-2022;

const quint8 REQUEST_GET_DEVICE_INFO = 0x00;
const quint8 REQUEST_GET_IMAGE = 0x01;
const quint8 REQUEST_SET_DEVICE_PARAMS = 0x02;
const quint8 DEVICE_REQUEST_COUNT = 0x03;

// ==============================================================================================

enum DeviceType
{
	NoDeviceType = 0,
	DeviceType_1 = 1,
};

const int DeviceTypeCount = 2;

#define ERR_DEVICE_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= DeviceTypeCount)

// DeviceType 1
// picture: Width=320, Height=180, imageType=RAW 8bit grayscale (1 pixel = 1 byte)
// frames: FrameSize=320 bytes, frameCount=180
// connect after: 20 sec
// header after: 35 sec
// receiving of packet: 50 sec

// ==============================================================================================

const int LAST_DEVICE_VERSION = 1;

// ==============================================================================================

const int DEVICE_IMEI_SIZE = 15;
const int DEVICE_ICCID_SIZE = 20;

// ==============================================================================================

#define UNDEFINED_IMEI 0x0000000000000000ULL

// ==============================================================================================

#pragma pack(push, 1)

struct DeviceRequestGetInfo
{
	quint32 signature = REQUEST_GET_DEVICE_INFO;

	quint8 headerSize = sizeof(DeviceRequestGetInfo);
	quint8 requestType = REQUEST_GET_DEVICE_INFO;

	quint8 headerVersion = LAST_DEVICE_VERSION;

	quint8 deviceType = DeviceType::NoDeviceType;

	quint8 IMEI[DEVICE_IMEI_SIZE];
	quint8 reserv0;
	quint8 ICCID[DEVICE_ICCID_SIZE];

	quint8 imageType = 0;
	quint16 imageSize = 0;
	quint16 imageWidth = 0;
	quint16 imageHeight = 0;

	quint16 frameSize = 0;
	quint16 frameCount = 0;

	quint8 brightness = 0;

	quint8 reserve[8];
};

#pragma pack(pop)

// ==============================================================================================
//
// REQUEST_GET_IMAGE
//

#pragma pack(push, 1)

struct DeviceRequestGetImage
{
	quint32 signature = DEVICE_SIGNATURE;

	quint8 requestSize = sizeof(DeviceRequestGetImage);
	quint8 requestType = REQUEST_GET_IMAGE;
};

#pragma pack(pop)

// ==============================================================================================
//
// REQUEST_SET_DEVICE_PARAMS
//

#pragma pack(push, 1)

struct DeviceRequestSetParams
{
	quint32 signature = DEVICE_SIGNATURE;

	quint8 requestSize = sizeof(DeviceRequestSetParams);
	quint8 requestType = REQUEST_SET_DEVICE_PARAMS;
	quint8 brightness = 0;
	quint8 reserve = 0;
};

#pragma pack(pop)

// ==============================================================================================
//
// REQUEST_HEADER
//

#pragma pack(push, 1)

struct DeviceDataHeader
{
	quint32 signature = 0;

	quint8 dataSize = 0;
	quint8 requestType = 0;
};

#pragma pack(pop)

// ==============================================================================================

const int MIN_DEVICE_PACKET_SIZE = sizeof(DeviceDataHeader);

// ==============================================================================================


#endif // DEVICEPACKET_H
