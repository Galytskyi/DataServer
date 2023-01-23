#include "MainWindow.h"
#include "Options.h"

#include <QApplication>

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

// -------------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setApplicationName("DataServer");
	a.setOrganizationName("Radiy");

	theOptions.load();

	MainWindow w;
	w.show();

	return a.exec();
}

// -------------------------------------------------------------------------------------------------------------------
