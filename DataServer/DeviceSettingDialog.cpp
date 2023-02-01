#include "DeviceSettingDialog.h"

// -------------------------------------------------------------------------------------------------------------------

DeviceSettingDialog::DeviceSettingDialog(int brightness, QWidget* parent)
	: QDialog(parent)
	, m_brightness(brightness)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

DeviceSettingDialog::~DeviceSettingDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool DeviceSettingDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Settings.png"));
	setWindowTitle(tr("Device settings"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	resize(static_cast<int>(screen.width() * 0.15), static_cast<int>(screen.height() * 0.06));
	move(screen.center() - rect().center());

	// brightness
	//
	QGroupBox* brightnessGroup = new QGroupBox(tr("Configuration of device flashlight brightness"));
	QVBoxLayout *brightnessLabelLayout = new QVBoxLayout;

	QLabel* brightnessLabel = new QLabel(tr("Brightness"), this);
	brightnessLabelLayout->addWidget(brightnessLabel);

	QVBoxLayout *brightnessEditLayout = new QVBoxLayout;
	m_brightnessSpinBox = new QSpinBox(this);
	m_brightnessSpinBox->setMinimum(0);
	m_brightnessSpinBox->setMaximum(100);
	m_brightnessSpinBox->setSingleStep(1);
	m_brightnessSpinBox->setValue(round(static_cast<double>(m_brightness) / 255.0 * 100.0));
	brightnessEditLayout->addWidget(m_brightnessSpinBox);

	QHBoxLayout *brightnessLayout = new QHBoxLayout;
	brightnessLayout->addLayout(brightnessLabelLayout);
	brightnessLayout->addLayout(brightnessEditLayout);

	brightnessGroup->setLayout(brightnessLayout);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DeviceSettingDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DeviceSettingDialog::reject);

	// Main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(brightnessGroup);
	mainLayout->addStretch();
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void DeviceSettingDialog::onOk()
{
	// brightness
	//
	double brightness = static_cast<double>(m_brightnessSpinBox->value());
	if (brightness < 0 || brightness > 100)
	{
		QMessageBox::critical(nullptr, windowTitle(), tr("Field brightness is wrong: %1\nRange: 0 .. 100 %").arg(brightness));
		return;
	}

	m_brightness = static_cast<int> (round(brightness * 255.0 / 100.0));

	//
	//
	accept();
}

// -------------------------------------------------------------------------------------------------------------------
