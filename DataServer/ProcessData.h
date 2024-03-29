#ifndef PROCESSDATA_H
#define PROCESSDATA_H

// ==============================================================================================

#define				COMPLETER_OPTIONS_KEY		"/CompleterText/"

// ----------------------------------------------------------------------------------------------

const int			COMPLETER_STRING_COUNT	= 10;

// ----------------------------------------------------------------------------------------------

class CompleterData : public QObject
{
	Q_OBJECT

public:

	explicit CompleterData(QObject* parent = nullptr);
	virtual ~CompleterData() override;

public:

	int				count() const { return m_count; }
	bool			setFilterCount(int count);

	void			setFilterList(const QStringList& list);

	bool			create(QObject* parent);
	bool			appendFilter(const QString& text);

	void			load(const QString& optionsKey);
	void			save(const QString& optionsKey);

	QCompleter*		completer() const { return m_filterCompleter; }

private:

	int				m_count = COMPLETER_STRING_COUNT;

	QStringList		m_filterCompleterList;
	QCompleter*		m_filterCompleter = nullptr;
};

// ==============================================================================================

class CopyData : public QObject
{
	Q_OBJECT

public:

	CopyData(QTableView* pView, bool copyHiddenColumn);
	virtual ~CopyData() override;

public:

	void			exec(int selectedColumn = -1);

private:

	QTableView*		m_pView = nullptr;
	bool			m_copyHiddenColumn = false;

	bool			m_copyCancel = true;

	bool			copyToMemory(int selectedColumn);

public slots:

	void			copyCancel();
	void			copyComplited();
};

// ==============================================================================================

#define				FIND_DATA_OPTIONS_KEY		"Options/Find/FindMeasure/"

// ----------------------------------------------------------------------------------------------

class FindData : public QDialog
{
	Q_OBJECT

public:

	explicit FindData(QTableView* pView);
	virtual ~FindData() override;

public:

	void			loadSettings();
	void			saveSettings();

	virtual void	reject() override;

private:

	QTableView*		m_pView = nullptr;

	CompleterData	m_findCompleter;
	QLineEdit*		m_pFindTextEdit = nullptr;
	QPushButton*	m_findNextButton = nullptr;

	QString			m_findText;

	void			createInterface(QTableView* pView);

	int				firstVisibleColumn();

	int				find(int start);
	void			enableFindNextButton(int start);

public slots:

	void			findTextChanged();
	void			findNext();
};

// ==============================================================================================

#define EXPORT_WINDOW_TITLE QT_TRANSLATE_NOOP("ExportData", "Export data")

// ----------------------------------------------------------------------------------------------

class ExportData : public QObject
{
	Q_OBJECT

public:

	ExportData(QTableView* pView, bool writeHiddenColumn, const QString& fileName);
	virtual ~ExportData() override;

public:

	void			exec();

private:

	QTableView*		m_pView = nullptr;
	bool			m_writeHiddenColumn = false;
	QString			m_fileName;

	QDialog*		m_pProgressDialog = nullptr;
	QProgressBar*	m_progress = nullptr;
	QPushButton*	m_cancelButton = nullptr;

	bool			m_exportCancel = true;

	void			createProgressDialog(QTableView* pView);
	static void		startExportThread(ExportData* pThis, const QString& fileName);

	bool			saveCsvFile(const QString& fileName);

signals:

	void			setValue(int);
	void			setRange(int, int);

	void			exportThreadFinish();

public slots:

	void			exportCancel();
	void			exportComplited();
};

// ==============================================================================================

#endif // PROCESSDATA_H
