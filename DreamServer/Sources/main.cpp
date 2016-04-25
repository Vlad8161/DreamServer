#include "include\widgets\dreamserver.h"
#include "include\widgets\MenuWidget.h"

#include <QtWidgets/QApplication>
#include <qfile.h>
#include <qsqltablemodel.h>
#include <qtextstream.h>
#include <fstream>

//#define TEST_REPO
#define MAIN
//#define TEST_MENU

std::ofstream logging_file("log.txt");

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString txt;
	switch (type) {
	case QtDebugMsg:
		txt = QString("Debug: %1").arg(msg);
		break;
	case QtWarningMsg:
		txt = QString("Warning: %1").arg(msg);
		break;
	case QtCriticalMsg:
		txt = QString("Critical: %1").arg(msg);
		break;
	case QtFatalMsg:
		txt = QString("Fatal: %1").arg(msg);
		abort();
	}
	logging_file << txt.toStdString() << "\r\n";
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//qInstallMessageHandler(myMessageHandler);
	
	QFile css(":/MainSheet.css");
	css.open(QIODevice::ReadOnly);
	if (css.isOpen()) {
		QByteArray ba = css.readAll();
		QString sheet = ba;
		a.setStyleSheet(sheet);
	}

#if defined(MAIN)
	ServerSolution w;

#elif defined(TEST_REPO)
	QSqlDatabase orders_connection = QSqlDatabase::addDatabase("QSQLITE", "OrdersConnection");
	orders_connection.setDatabaseName("test.db");
	orders_connection.open();
	QSqlDatabase menu_connection = QSqlDatabase::addDatabase("QSQLITE", "MenuConnection");
	menu_connection.setDatabaseName("test.db");
	menu_connection.open();

	MenuDatabaseModel menu_db_model(menu_connection);
	OrdersRepo orders_repo(orders_connection, &menu_db_model);
	TestRepoWidget w(&orders_repo, &menu_db_model);
	MenuWidget m(&menu_db_model);
	m.show();

#elif defined(TEST_MENU)
	MenuWidget w;

#endif

	w.show();

	return a.exec();
}