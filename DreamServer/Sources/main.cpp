#include "include\widgets\dreamserver.h"
#include "include\widgets\MenuWidget.h"
#include <QtWidgets/QApplication>
#include <qfile.h>
#include <qsqltablemodel.h>
#include <qtextstream.h>
#include <qlocalsocket.h>
#include <qlocalserver.h>
#include <fstream>

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
    QLocalSocket s;

	//qInstallMessageHandler(myMessageHandler);

    s.connectToServer("dpiki_dreamserver");
    if (!s.waitForConnected(2)) {
        QFile css(":/MainSheet.css");
        css.open(QIODevice::ReadOnly);
        if (css.isOpen()) {
            QByteArray ba = css.readAll();
            QString sheet = ba;
            a.setStyleSheet(sheet);
        }

        ServerSolution w;
        QLocalServer new_app_inst_listener;
        QObject::connect(&new_app_inst_listener, &QLocalServer::newConnection,
            [&w]() { w.show(), w.activateWindow(); });
        new_app_inst_listener.listen("dpiki_dreamserver");

        w.show();

        return a.exec();
    }
}