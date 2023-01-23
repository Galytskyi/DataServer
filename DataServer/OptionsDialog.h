#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "Options.h"

// ==============================================================================================

class OptionsDialog : public QDialog
{
	Q_OBJECT

public:

	explicit OptionsDialog(const ServerOption& serverOption, const DatabaseOption& databaseOption, const ImeiOption& imeiOption, QWidget *parent = nullptr);
	virtual ~OptionsDialog() override;

public:

	ServerOption& serverOption() { return m_serverOption; }
	DatabaseOption& databaseOption() { return m_databaseOption; }
	ImeiOption& imeiOption() { return m_imeiOption; }

private:

	ServerOption m_serverOption;
	DatabaseOption m_databaseOption;
	ImeiOption m_imeiOption;

	QLineEdit* m_serverIPEdit = nullptr;
	QLineEdit* m_serverPortEdit = nullptr;
	QLineEdit* m_serverConnectionsEdit = nullptr;

	QLineEdit* m_databaseIPEdit = nullptr;
	QLineEdit* m_databasePortEdit = nullptr;
	QLineEdit* m_databaseUserEdit = nullptr;
	QLineEdit* m_databasePasswordEdit = nullptr;

	QLineEdit* m_digitsImeiEdit = nullptr;

	QDialogButtonBox* m_buttonBox = nullptr;

	bool createInterface();

private slots:

	void onOk();
};

// ==============================================================================================

#endif // OPTIONSDIALOG_H
