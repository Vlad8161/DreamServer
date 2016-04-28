#include "include/networkmanager.h"
#include "include/networkconnectionsmodel.h"
#include "include/menudatabasemodel.h"
#include "include/ordersrepo.h"

#include <qtcpsocket.h>
#include <qnetworkinterface.h>
#include <qbuffer.h>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qcryptographichash.h>
#include <qtimer.h>

#include <assert.h>


NetworkManager::NetworkManager(MenuDatabaseModel* menu_model, OrdersRepo* orders_repo)
{
	m_menu = menu_model;
	m_repo = orders_repo;
	m_tcp_server = new QTcpServer(this);
	m_timer = new QTimer(this);

	m_serialize_menu();

	connect(m_menu, SIGNAL(menu_changed()),
		this, SLOT(on_menu_changed()));
	connect(m_repo, SIGNAL(table_changed(int)),
		this, SLOT(on_table_changed(int)));
	connect(m_tcp_server, SIGNAL(newConnection()),
		this, SLOT(on_connect()));
	connect(m_timer, SIGNAL(timeout()),
		this, SLOT(on_tick()));

	m_timer->start(1000);
}



NetworkManager::~NetworkManager()
{
	if (m_tcp_server->isListening())
		stop();
	delete m_tcp_server;
	delete m_timer;
}



bool NetworkManager::start()
{
	if (m_tcp_server->isListening())
		return true;

	if (!m_tcp_server->listen(QHostAddress::Any, m_port))
		return false;

	emit server_started();

	return true;
}



void NetworkManager::stop()
{
	if (m_tcp_server->isListening()) {
		m_tcp_server->close();
		m_clear_clients_list();
		emit server_stopped();
		emit connection_list_changed();
	}
}



void NetworkManager::on_connect()
{
	QTcpSocket* new_socket = m_tcp_server->nextPendingConnection();
	NetworkResponser* new_client = new NetworkResponser(new_socket, m_menu, m_repo,
		&m_cached_menu, &m_cached_hash);

	assert(!m_clients.contains(new_socket));

	m_clients[new_socket] = new_client;

	connect(new_socket, SIGNAL(disconnected()),
		this, SLOT(on_disconnect()));
	connect(new_client, SIGNAL(status_changed()),
		this, SLOT(on_responser_status_changed()));

	emit connection_list_changed();
}



void NetworkManager::on_disconnect()
{
	QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

	assert(m_clients.contains(socket));

	delete m_clients[socket];
	m_clients.remove(socket);

	emit connection_list_changed();
}



void NetworkManager::on_tick()
{
	for (auto& i : m_clients.values()) {
		i->check_connection();
	}
}



void NetworkManager::on_menu_changed()
{
	m_clear_clients_list();
	m_serialize_menu();
}



void NetworkManager::m_clear_clients_list()
{
	for (auto& i : m_clients.keys()) {
		i->disconnectFromHost();
	}
}



QHostAddress NetworkManager::address() const
{
	QHostAddress ret;

	if (!m_tcp_server->isListening())
		return QHostAddress::Null;

	auto addresses = QNetworkInterface::allAddresses();
	for (auto& i : addresses) {
		if (i != QHostAddress::LocalHost && i.toIPv4Address())
			return i;
	}

	return QHostAddress::LocalHost;
}



quint16 NetworkManager::port() const
{
	if (m_tcp_server->isListening()) {
		return m_tcp_server->serverPort();
	}
	else {
		return 0;
	}
}



NetworkConnectionsModel* NetworkManager::create_model() const
{
	NetworkConnectionsModel* ret = new NetworkConnectionsModel(this);

	connect(this, SIGNAL(connection_list_changed()),
		ret, SLOT(on_connection_list_changed()));
	connect(this, SIGNAL(client_status_changed(NetworkResponser*)),
		ret, SLOT(client_status_changed(NetworkResponser*)));

	return ret;
}


void NetworkManager::m_serialize_menu()
{
	m_cached_hash.clear();
	m_cached_menu.clear();

	auto menu = m_menu->query_items();
	for (auto& i : menu) {
		assert(!i->category.contains(';'));
		assert(!i->name.contains(';'));
		m_cached_menu += QString("%1;%2;%3\n")
			.arg(i->id)
			.arg(i->name)
			.arg(i->category);
	}

	auto ba = QCryptographicHash::hash(
		m_cached_menu.toUtf8(),
		QCryptographicHash::Sha256).toHex();
	m_cached_hash = QString(ba);
}



void NetworkManager::on_responser_status_changed()
{
	NetworkResponser* responser = qobject_cast<NetworkResponser*>(sender());
	emit client_status_changed(responser);
}