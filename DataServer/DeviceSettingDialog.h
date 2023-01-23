#ifndef DEVICESETTINGDIALOG_H
#define DEVICESETTINGDIALOG_H

#include <QSpinBox>

// ==============================================================================================

class DeviceSettingDialog : public QDialog
{
	Q_OBJECT

public:

	explicit DeviceSettingDialog(int brightness, QWidget *parent = nullptr);
	virtual ~DeviceSettingDialog() override;

public:

	int brightness() { return m_brightness; }

private:

	int m_brightness = 0;
	QSpinBox* m_brightnessSpinBox = nullptr;

	QDialogButtonBox* m_buttonBox = nullptr;

	bool createInterface();

private slots:

	void onOk();
};

// ==============================================================================================

#endif // DEVICESETTINGDIALOG_H
