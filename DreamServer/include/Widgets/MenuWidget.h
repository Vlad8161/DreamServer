#ifndef _MENU_WIDGET_H_
#define _MENU_WIDGET_H_

#include <QtWidgets/QDialog>
#include "ui_menu_widget.h"
#include "include/MenuDatabaseModel.h"

class MenuWidget : public QDialog
{
	Q_OBJECT

public:
	MenuWidget(MenuDatabaseModel* menu, QWidget *parent = 0);
	~MenuWidget();

public slots:
	void on_add_item_action();
	void on_remove_item_action();
	void on_add_category_action();
	void on_remove_category_action();
	void on_change_img_action();
	void on_context_menu_requested(const QPoint& p);

private:
	void create_actions();

private:
	QAction* m_add_item_action;
	QAction* m_remove_item_action;
	QAction* m_add_category_action;
	QAction* m_remove_category_action;
	QAction* m_change_img_action;
	QMenu* m_category_menu;
	QMenu* m_item_menu;
	QMenu* m_tree_menu;
	Ui::menu_widget ui;
	MenuDatabaseModel* m_menu;
};

#endif // MENU_WIDGET_H_