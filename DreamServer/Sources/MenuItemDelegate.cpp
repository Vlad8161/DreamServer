#include "include/MenuItemDelegate.h"
#include <qlineedit.h>


void MenuItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	QString curr_val = index.model()->data(index).toString();
	QLineEdit* line_edit = qobject_cast<QLineEdit*>(editor);
	if (!curr_val.isEmpty()) {
		line_edit->setText(curr_val);
		line_edit->setSelection(0, curr_val.size() - 1);
	}
}



void MenuItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
	const QModelIndex& index) const
{
	model->setData(index, qobject_cast<QLineEdit*>(editor)->text());
}