#ifndef PACKETBASE_H
#define PACKETBASE_H

#include <QObject>
#include <QDateTime>
#include <QMutex>

// ==============================================================================================

#define IMAGE_DIRECTORY "IMG"

// ==============================================================================================

enum ImageType
{
	NoImageType = -1,
	Raw = 0,
	Bmp = 1,
	Jpg = 2,
};

const int ImageTypeCount = 3;

#define ERR_IMAGE_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= ImageTypeCount)

QString ImageTypeCaption(int imageType);

// ==============================================================================================

#define IMG_PACKET_TIME_FORMAT "dd-MM-yyyy hh:mm:ss"

// ==============================================================================================

const int MAX_IMAGE_DATA_SIZE = 17600;

// ==============================================================================================

class DataPacket
{

public:

	explicit DataPacket();
	virtual ~DataPacket();

public:

	void virtual clear();

	int packetID() const { return m_packetID; }
	void setPacketID(int id) { m_packetID = id; }

	quint64 imei() const { return m_IMEI; }
	QString imeiStr(int digits) const;
	void setImei(quint64 imei) { m_IMEI = imei; }

	QDateTime packetTime() const { return m_packetTime; }
	void setPacketTime(const QDateTime& time) { m_packetTime = time; }

	ImageType imageType() const { return m_imageType; }
	void setImageType(ImageType type) { m_imageType = type; }
	void setImageType(int type) { m_imageType = static_cast<ImageType>(type); }

	int imageSize() const { return m_imageSize; }
	void setImageSize(int size) { m_imageSize = size; }

	int imageWidth() const { return m_imageWidth; }
	void setImageWidth(int width) { m_imageWidth = width; }

	int imageHeight() const { return m_imageHeight; }
	void setImageHeight(int height) { m_imageHeight = height; }

	int brightness() const { return m_brightness; }
	void setBrightness(int brightness) { m_brightness = brightness; }

	QString location() const { return m_location; }
	void setLocation(const QString& location) { m_location = location; }

	QByteArray& imageData() { return m_imageData; }
	void setImageData(const QByteArray& imageData) { m_imageData = imageData; }

private:

	int m_packetID = -1;											// primary key of record in SQL table
	quint64 m_IMEI = 0;

	QDateTime m_packetTime;											// time when packet was received

	ImageType m_imageType = ImageType::NoImageType;
	int m_imageSize = 0;
	int m_imageWidth = 0;
	int m_imageHeight = 0;

	int m_brightness = 0;
	QString m_location;

	QByteArray m_imageData;
};

// ==============================================================================================

class PacketBase : public QObject
{
	Q_OBJECT

public:

	explicit PacketBase(QObject *parent = nullptr);
	virtual ~PacketBase() override;

public:

	int count() const;
	void clear();

	int load();

	int append(DataPacket* pPacket);
	DataPacket* packet(int index) const;
	bool remove(int index);
	bool remove( const std::vector<int>& keyList);	// keyList this is list of packetID

private:

	mutable QMutex m_packetMutex;
	std::vector<DataPacket*> m_packetList;

signals:

	void packetBaseLoaded(const std::vector<DataPacket*>& list);

public slots:

	void appendToBase(DataPacket* pPacket);
	void removeFromBase(const std::vector<int>& keyList);
};

// ==============================================================================================

QString timeToStr(const QDateTime& m_packetTime);
QString imageDir(const QString& fileName);

// ==============================================================================================


#endif // PACKETBASE_H
