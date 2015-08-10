#include "AccountTypesModel.h"

AccountTypesModel::AccountTypesModel(AccountStorePtr store) : QAbstractListModel()
{
	m_store = store;
}

AccountTypesModel::~AccountTypesModel()
{
}
QVariant AccountTypesModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}
	auto row = index.row();
	if(row < 0 || row >= rowCount())
	{
		return QVariant();
	}
	auto accountType = m_store->getAccountType(row);
	switch(role)
	{
		case Qt::UserRole:
		{
			return QVariant::fromValue<BaseAccountType *>(accountType);
		}
		case Qt::DisplayRole:
		{
			return accountType->text();
		}
		case Qt::DecorationRole:
		{
			return accountType->icon();
		}
		default:
			return QVariant();
	}
}

int AccountTypesModel::rowCount(const QModelIndex &parent) const
{
	return m_store->numAccountTypes();
}
