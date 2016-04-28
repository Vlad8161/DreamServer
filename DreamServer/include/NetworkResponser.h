#ifndef _NETWORK_CLIENT_H
#define _NETWORK_CLIENT_H

#include <qobject.h>
#include <qjsonarray.h>

class QTcpSocket;
class MenuDatabaseModel;
class OrdersRepo;


enum ActionCodes {
	Authorize = 1,
	CheckSync = 2,
	RequestMenu = 3,
	MakeOrder = 4,
	CheckConnection = 5
};

enum ResponseCodes {
	AuthorizeSuccess = 1,
	SyncSuccess = 2,
	Menu = 3,
	OrderMade = 4,
	ErrorInvalidRequest = 5,
	ErrorInvalidPassword = 6,
	ErrorInvalidHash = 7,
	ErrorInvalidCourseId = 8,
	ErrorAccessDeniedAuth = 9,
	ErrorAccessDeniedSync = 10,
	IAmHere = 11
};




/*Класс отвечает за обработку запросов клиента.
 *Для более быстрой работы содержимое меню кэшируется в виде
 *JSON объекта (m_cached_menu). Так же кэшируются изображения
 *каждого блюда в виде QByteArray и хэш меню. При обновлении меню
 *эти данные так же обновляются.
 */
class NetworkResponser : public QObject
{
	Q_OBJECT

public:
	enum StatusFlags {
		ClientAuthorized = 0x00000001,
		ClientSynchronizedMenu = 0x00000002,
	};

	NetworkResponser(QTcpSocket* socket, MenuDatabaseModel* menu, OrdersRepo* repo,
		QString* cached_menu, QString* cached_hash);
	~NetworkResponser();
	const QString& name() const { return m_name; }
	void increment_ticks();

public slots:
	// слот, обрабатывающий соответствующий 
    // сигнал от сокета (вызывает функции обработки запросов)
	void on_ready_read();

private:
	// обработка запроса каждого типа (вызывает m_send_response)
	void m_handle_auth_request(const QJsonObject& root);
	void m_handle_check_sync_request(const QJsonObject& root);
	void m_handle_menu_request(const QJsonObject& root);
	void m_handle_make_order_request(const QJsonObject& root);
	void m_handle_check_connection();
	// отправка данных
	void m_send_response(const QJsonObject& root, const QByteArray& appendix = QByteArray());

private:
	QString m_name;
	QTcpSocket* m_socket;
	MenuDatabaseModel* m_menu;
	OrdersRepo* m_repo;
	qint32 m_message_size;
	qint32 m_flags;
	QString* m_cached_menu;
	QString* m_cached_hash;
	quint32 m_unresponsed_ticks;

signals:
	void status_changed();
};

#endif