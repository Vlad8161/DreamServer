#include "include/networkresponser.h"
#include "include/menudatabasemodel.h"
#include "include/ordersrepo.h"

#include <qtcpsocket.h>
#include <qdebug.h>
#include <qbuffer.h>
#include <qhostaddress.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>

#include <stdlib.h>


struct PreOrder {
	int id_course;
	int count;
	int t_num;
	QString notes;
};


NetworkResponser::NetworkResponser(QTcpSocket* socket, MenuDatabaseModel* menu, OrdersRepo* repo,
	QString* cached_menu, QString* cached_hash)
{
	m_socket = socket;
	m_menu = menu;
	m_repo = repo;
	m_flags = 0;
	m_message_size = 0;
	m_cached_menu = cached_menu;
	m_cached_hash = cached_hash;
	m_name = socket->peerAddress().toString();

	connect(m_socket, SIGNAL(readyRead()),
		this, SLOT(on_ready_read()), Qt::QueuedConnection);
}


NetworkResponser::~NetworkResponser()
{
	m_socket->disconnectFromHost();
	m_socket->deleteLater();
}



void NetworkResponser::on_ready_read()
{
	while (true) {
		if (m_message_size == 0) {
			int bytes = m_socket->bytesAvailable();
			if (m_socket->bytesAvailable() < 4)
				return;
			m_socket->read((char*)&m_message_size, 4);
		}

		int bytes = m_socket->bytesAvailable();
		int bytes_available = m_socket->bytesAvailable();
		if (m_socket->bytesAvailable() < m_message_size)
			return;

		QByteArray ba = m_socket->read(m_message_size);
		m_message_size = 0;

		QJsonDocument doc = QJsonDocument::fromJson(ba);
		QJsonObject root = doc.object();
		if (root.isEmpty()) {
			QJsonObject response;
			response["response_code"] = ResponseCodes::ErrorInvalidRequest;
			m_send_response(response);
			continue;
		}

		if (!root.contains("action_code")) {
			QJsonObject response;
			response["response_code"] = ResponseCodes::ErrorInvalidRequest;
			m_send_response(response);
			continue;
		}

		int action_code = root["action_code"].toInt();

		if (action_code == ActionCodes::Authorize) {
			m_handle_auth_request(root);
		}
		else if (action_code == ActionCodes::CheckSync) {
			m_handle_check_sync_request(root);
		}
		else if (action_code == ActionCodes::RequestMenu) {
			m_handle_menu_request(root);
		}
		else if (action_code == ActionCodes::MakeOrder) {
			m_handle_make_order_request(root);
		}
		else {
			QJsonObject response;
			response["response_code"] = ResponseCodes::ErrorInvalidRequest;
			m_send_response(response);
			continue;
		}
	}
}



void NetworkResponser::m_handle_auth_request(const QJsonObject& root)
{
	QJsonObject response;
	
	if (!root.contains("password")) {
		response["response_code"] = ResponseCodes::ErrorInvalidRequest;
		m_send_response(response);
		return;
	}
	
	if (root["password"].toString() != "password") {
		response["response_code"] = ResponseCodes::ErrorInvalidPassword;
		m_send_response(response);
		return;
	}

	if (root.contains("name")) {
		m_name = root["name"].toString() + " (" + m_socket->peerAddress().toString() + ")";
		emit status_changed();
	}

	m_flags |= StatusFlags::ClientAuthorized;
	response["response_code"] = ResponseCodes::AuthorizeSuccess;
	m_send_response(response);
}



void NetworkResponser::m_handle_check_sync_request(const QJsonObject& root)
{
	QJsonObject response;
	
	if (!(m_flags & StatusFlags::ClientAuthorized)) {
		response["response_code"] = ResponseCodes::ErrorAccessDeniedAuth;
		m_send_response(response);
		return;
	}

	if (!root.contains("hash")) {
		response["response_code"] = ResponseCodes::ErrorInvalidRequest;
		m_send_response(response);
		return;
	}

	auto s = root["hash"].toString().toUpper();
	if (root["hash"].toString().toUpper() != m_cached_hash->toUpper()) {
		response["response_code"] = ResponseCodes::ErrorInvalidHash;
		m_send_response(response);
		return;
	}

	m_flags |= StatusFlags::ClientSynchronizedMenu;
	response["response_code"] = ResponseCodes::SyncSuccess;
	m_send_response(response);
}



void NetworkResponser::m_handle_menu_request(const QJsonObject& root)
{
	QJsonObject response;
	
	if (!(m_flags & StatusFlags::ClientAuthorized)) {
		response["response_code"] = ResponseCodes::ErrorAccessDeniedAuth;
		m_send_response(response);
		return;
	}

	response["response_code"] = ResponseCodes::Menu;
	response["menu"] = *m_cached_menu;
	m_send_response(response);
}



void NetworkResponser::m_handle_make_order_request(const QJsonObject& root)
{
	QJsonObject response;

	if (!(m_flags & StatusFlags::ClientAuthorized)) {
		response["response_code"] = ResponseCodes::ErrorAccessDeniedAuth;
		m_send_response(response);
		return;
	}

	if (!(m_flags & StatusFlags::ClientSynchronizedMenu)) {
		response["response_code"] = ResponseCodes::ErrorAccessDeniedSync;
		m_send_response(response);
		return;
	}

	if (!root.contains("orders") || !root["orders"].isArray()) {
		response["response_code"] = ResponseCodes::ErrorInvalidRequest;
		m_send_response(response);
		return;
	}

	QJsonArray& items = root["orders"].toArray();
	QVector<PreOrder> pre_orders;

	for (auto& i : items) {
		if (!i.isObject()) {
			response["response_code"] = ResponseCodes::ErrorInvalidRequest;
			m_send_response(response);
			return;
		}

		QJsonObject o = i.toObject();
		if (!o.contains("id_course") ||
			!o.contains("count") ||
			!o.contains("table_num") ||
			!o.contains("notes")) {
			response["response_code"] = ResponseCodes::ErrorInvalidRequest;
			m_send_response(response);
			return;
		}

		PreOrder pre_o;
		pre_o.id_course = o["id_course"].toInt();
		pre_o.count = o["count"].toInt();
		pre_o.t_num = o["table_num"].toInt();
		pre_o.notes = o["notes"].toString();
		if (!m_menu->contains(pre_o.id_course)) {
			response["response_code"] = ResponseCodes::ErrorInvalidCourseId;
			m_send_response(response);
			return;
		}

		pre_orders.push_back(pre_o);
	}

	for (auto& i : pre_orders)
		m_repo->add_order(i.id_course, i.t_num, i.count, i.notes);

	response["response_code"] = ResponseCodes::OrderMade;
	m_send_response(response);
}



// Это основной метод, формирующий пакет данных для передачи
void NetworkResponser::m_send_response(const QJsonObject& root, const QByteArray& appendix)
{
	QJsonDocument doc(root);
	QByteArray response = doc.toJson();
	quint32 json_size = response.size();
	quint32 response_size = json_size + sizeof(json_size) + appendix.size();
	response.insert(0, (char*)&json_size, sizeof(json_size));
	response.insert(0, (char*)&response_size, sizeof(response_size));
	if (appendix.size() != 0)
		response.append(appendix);
	m_socket->write(response);
	m_socket->flush();
}