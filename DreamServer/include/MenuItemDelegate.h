#ifndef _MENU_ITEM_DELEGATE_
#define _MENU_ITEM_DELEGATE_

#include <qstyleditemdelegate.h>

class MenuItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	MenuItemDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}
	void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	void setModelData(QWidget* editor, QAbstractItemModel* model, 
		const QModelIndex& index) const override;
};

#endif