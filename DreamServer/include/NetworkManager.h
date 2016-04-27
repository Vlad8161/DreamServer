#ifndef _NETWORK_MANAGER_H_
#define _NETWORK_MANAGER_H_

#include <qtcpserver.h>
#include <qabstractitemmodel.h>
#include "include/NetworkResponser.h"


class MenuDatabaseModel;
class OrdersRepo;
class NetworkConnectionsModel;
class QTimer;

/* Manages incoming connections
 * Creates NetworkResponser for each connection
 * Removes NetworkResponser when needed (lost connection, menu changed
 * Serializes and caches menu
 * Notifies model when connection list changes
 */
class NetworkManager : public QObject
{
	Q_OBJECT

public:
	NetworkManager(MenuDatabaseModel* menu_model, OrdersRepo* orders_repo);
	~NetworkManager();
	NetworkConnectionsModel* create_model() const;
	QHostAddress address() const;
	quint16 port() const;
	QList<NetworkResponser*> clients() const { return m_clients.values(); }
	int n_clients() const { return m_clients.count(); }
	bool start();
	void stop();

private slots:
	void on_connect();
	//void on_socket_error();
	void on_disconnect();
	void on_menu_changed();
	void on_responser_status_changed();
	void on_tick();

private:
	void m_clear_clients_list();
	void m_serialize_menu();

signals:
	void server_started();
	void server_stopped();
	void connection_list_changed();
	void client_status_changed(NetworkResponser*);

private:
	QMap<QTcpSocket*, NetworkResponser*> m_clients;
	QTcpServer* m_tcp_server;
	OrdersRepo* m_repo;
	MenuDatabaseModel* m_menu;
	QTimer* m_timer;
	QString m_cached_menu;
	QString m_cached_hash;

	int m_port = 13563;
};

#endif