#pragma once
#include <QAbstractListModel>
#include "BaseAccountType.h"
#include "AccountStore.h"

class AccountTypesModel : public QAbstractListModel
{
public:
	explicit AccountTypesModel(AccountStorePtr store);
	virtual ~AccountTypesModel();
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
	AccountStorePtr m_store;
};
