#ifndef _ORDERS_REPO_VIEW_H_
#define _ORDERS_REPO_VIEW_H_

#include <qtableview.h>

class OrdersRepoModel;

// этот класс необходим для управления моделью
// и настройки представления
// после конструирования представляет собой
// TableView, настроеный на отображение необходимой модели,
// а после уничтожения, уничтожает и модель
class OrdersRepoView : public QTableView
{
	Q_OBJECT

public:
	OrdersRepoView(OrdersRepoModel* model);
	~OrdersRepoView();

public slots:
	void on_action_not_prepare();
	void on_action_prepare();
	void on_action_serve();
	void on_context_menu_requested(const QPoint& pos);

private:
	OrdersRepoModel* m_model;
	QAction* m_action_not_prepare;
	QAction* m_action_prepare;
	QAction* m_action_serve;
	QMenu* m_context_menu;
	QPoint m_cursor_pos;
};

#endif
