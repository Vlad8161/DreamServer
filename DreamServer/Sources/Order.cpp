#include "..\include\Order.h"


Order::Order() : id(0), count(0), table_num(0), status(NOT_PREPARED)
{
}



Order& Order::operator=(const Order& ord1)
{
	id            = ord1.id;
	name          = ord1.name;
	count         = ord1.count;
	table_num     = ord1.table_num;
	time_got      = ord1.time_got;
	time_prepared = ord1.time_prepared;
	status        = ord1.status;
	notes         = ord1.notes;
	
	return *this;
}



bool operator==(const Order& ord1, const Order& ord2)
{
	if (ord1.id != ord2.id ||
		ord1.name != ord2.name ||
		ord1.count != ord2.count ||
		ord1.table_num != ord2.table_num ||
		ord1.status != ord2.status ||
		ord1.time_got != ord2.time_got ||
		ord1.time_prepared != ord2.time_prepared ||
		ord1.notes != ord2.notes)
		return false;
	else
		return true;
}
