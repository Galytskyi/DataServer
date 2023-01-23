#include "OptionsDialog.h"

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::OptionsDialog(const ServerOption& serverOption, const DatabaseOption& databaseOption, const ImeiOption& imeiOption, QWidget* parent)
	: QDialog(parent)
	, m_serverOption(serverOption)
	, m_databaseOption(databaseOption)
	, m_imeiOption(imeiOption)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::~OptionsDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool OptionsDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setWindowTitle(tr("Options"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	resize(static_cast<int>(screen.width() * 0.15), static_cast<int>(screen.height() * 0.30));
	move(screen.center() - rect().center());

	// server
	//
	QGroupBox* serverGroup = new QGroupBox(tr("Configuration of server connection"));
	QVBoxLayout *serverLabelLayout = new QVBoxLayout;


	QLabel* serverIPLabel = new QLabel(tr("Server IP"), this);
	QLabel* serverPortLabel = new QLabel(tr("Server Port"), this);
	QLabel* serverConnectionsLabel = new QLabel(tr("Connections"), this);
	serverConnectionsLabel->hide();


	serverLabelLayout->addWidget(serverIPLabel);
	serverLabelLayout->addWidget(serverPortLabel);
	//serverLabelLayout->addWidget(serverConnectionsLabel);

	QVBoxLayout *serverEditLayout = new QVBoxLayout;

	m_serverIPEdit = new QLineEdit(m_serverOption.ip(), this);
	m_serverPortEdit = new QLineEdit(QString::number(m_serverOption.port()), this);
	m_serverConnectionsEdit = new QLineEdit(QString::number(m_serverOption.maxPendingConnections()), this);
	m_serverConnectionsEdit->hide();

	serverEditLayout->addWidget(m_serverIPEdit);
	serverEditLayout->addWidget(m_serverPortEdit);
	//serverEditLayout->addWidget(m_serverConnectionsEdit);

	QHBoxLayout *serverLayout = new QHBoxLayout;

	serverLayout->addLayout(serverLabelLayout);
	serverLayout->addLayout(serverEditLayout);

	serverGroup->setLayout(serverLayout);

	// database
	//
	QGroupBox* databaseGroup = new QGroupBox(tr("Configuration of database connection"));
	QVBoxLayout *databaseLabelLayout = new QVBoxLayout;


	QLabel* databaseIPLabel = new QLabel(tr("Server IP"), this);
	QLabel* databasePortLabel = new QLabel(tr("Server Port"), this);
	QLabel* databaseUserLabel = new QLabel(tr("User"), this);
	QLabel* databasePasswordLabel = new QLabel(tr("Password"), this);


	databaseLabelLayout->addWidget(databaseIPLabel);
	databaseLabelLayout->addWidget(databasePortLabel);
	databaseLabelLayout->addWidget(databaseUserLabel);
	databaseLabelLayout->addWidget(databasePasswordLabel);

	QVBoxLayout *databaseEditLayout = new QVBoxLayout;

	m_databaseIPEdit = new QLineEdit(m_databaseOption.ip(), this);
	m_databasePortEdit = new QLineEdit(QString::number(m_databaseOption.port()), this);
	m_databaseUserEdit = new QLineEdit(m_databaseOption.user(), this);
	m_databasePasswordEdit = new QLineEdit(m_databaseOption.password(), this);
	m_databasePasswordEdit->setEchoMode(QLineEdit::Password);

	databaseEditLayout->addWidget(m_databaseIPEdit);
	databaseEditLayout->addWidget(m_databasePortEdit);
	databaseEditLayout->addWidget(m_databaseUserEdit);
	databaseEditLayout->addWidget(m_databasePasswordEdit);

	QHBoxLayout *databaseLayout = new QHBoxLayout;

	databaseLayout->addLayout(databaseLabelLayout);
	databaseLayout->addLayout(databaseEditLayout);

	databaseGroup->setLayout(databaseLayout);

	// digits of imei
	//
	QGroupBox* digitsImeiGroup = new QGroupBox(tr("Amount digits for displaying IMEI"));
	QVBoxLayout *digitsImeiLabelLayout = new QVBoxLayout;


	QLabel* digitsImeiLabel = new QLabel(tr("Amount"), this);
	digitsImeiLabelLayout->addWidget(digitsImeiLabel);

	QVBoxLayout *digitsImeiEditLayout = new QVBoxLayout;
	m_digitsImeiEdit = new QLineEdit(QString::number(m_imeiOption.imeiDigits()), this);
	digitsImeiEditLayout->addWidget(m_digitsImeiEdit);

	QHBoxLayout *digitsImeiLayout = new QHBoxLayout;

	digitsImeiLayout->addLayout(digitsImeiLabelLayout);
	digitsImeiLayout->addLayout(digitsImeiEditLayout);

	digitsImeiGroup->setLayout(digitsImeiLayout);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &OptionsDialog::reject);

	// Main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(serverGroup);
	mainLayout->addWidget(databaseGroup);
	mainLayout->addWidget(digitsImeiGroup);
	mainLayout->addStretch();
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onOk()
{
	// server
	//
	QString serverIP = m_serverIPEdit->text();
	if (serverIP.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field server IP is empty!"));
		return;
	}

	QString serverPort = m_serverPortEdit->text();
	if (serverPort.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field server port is empty!"));
		return;
	}

	QString serverConnections = m_serverConnectionsEdit->text();
	if (serverConnections.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field connections is empty!"));
		return;
	}

	m_serverOption.setIp(serverIP);
	m_serverOption.setPort(serverPort.toInt());
	m_serverOption.setMaxPendingConnections(serverConnections.toInt());

	// databse
	//
	QString databaseIP = m_databaseIPEdit->text();
	if (databaseIP.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field database IP is empty!"));
		return;
	}

	QString databasePort = m_databasePortEdit->text();
	if (databasePort.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field database port is empty!"));
		return;
	}

	QString databaseUser = m_databaseUserEdit->text();
	if (databaseUser.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field database user is empty!"));
		return;
	}

	QString databasePassword = m_databasePasswordEdit->text();
	if (databasePassword.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field database password is empty!"));
		return;
	}

	m_databaseOption.setIp(databaseIP);
	m_databaseOption.setPort(databasePort.toInt());
	m_databaseOption.setUser(databaseUser);
	m_databaseOption.setPassword(databasePassword);

	// digits of imei
	//
	QString digitsImei = m_digitsImeiEdit->text();
	if (digitsImei.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field digits of imei is empty!"));
		return;
	}

	m_imeiOption.setImeiDigits(digitsImei.toInt());

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
