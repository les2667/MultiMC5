/* Copyright 2015 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "AccountModel.h"
#include "BaseProfile.h"
#include <resources/ResourceProxyModel.h>
#include <QDebug>

AccountModel::AccountModel(AccountStorePtr store) : QAbstractItemModel()
{
	m_store = store;
	connect(store.get(), &AccountStore::aboutToRemove, this, &AccountModel::startRemoving);
	connect(store.get(), &AccountStore::removalFinished, this, &AccountModel::removingFinished);
	connect(store.get(), &AccountStore::aboutToAdd, this, &AccountModel::startAdding);
	connect(store.get(), &AccountStore::addingFinished, this, &AccountModel::addingFinished);
	connect(store.get(), &AccountStore::aboutToReset, this, &AccountModel::startResetting);
	connect(store.get(), &AccountStore::resetFinished, this, &AccountModel::resettingFinished);
	connect(store.get(), &AccountStore::itemChanged, this, &AccountModel::itemChanged);
}

void AccountModel::addingFinished()
{
	endInsertRows();
}

void AccountModel::startAdding(int acc)
{
	beginInsertRows(QModelIndex(), acc, acc);
}

void AccountModel::startRemoving(int acc)
{
	beginRemoveRows(QModelIndex(), acc, acc);
}

void AccountModel::removingFinished()
{
	endRemoveRows();
}

void AccountModel::startResetting()
{
	beginResetModel();
}

void AccountModel::resettingFinished()
{
	endResetModel();
}

int AccountModel::columnCount(const QModelIndex &parent) const
{
	return 3;
}

QModelIndex AccountModel::parent(const QModelIndex &child) const
{
	if (!child.isValid())
		return QModelIndex();

	auto item = static_cast<AuthElement *>(child.internalPointer());
	if(item->getKind() == AuthElement::Account)
	{
		// account -> root
		return QModelIndex();
	}
	if(item->getKind() == AuthElement::Profile)
	{
		auto prof = (BaseProfile *) item;
		auto acc = prof->parent();
		auto index = m_store->getAccountIndex(acc);
		if(index < 0)
		{
			return QModelIndex();
		}
		return createIndex(index, 0, prof->parent());
	}
	// catch-all
	qWarning() << "INVALID MODEL INDEX!";
	return QModelIndex();
}

QModelIndex AccountModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	// account
	if (!parent.isValid())
	{
		auto acc = m_store->getAccount(row);
		if(acc)
		{
			return createIndex(row, column, acc);
		}
		return QModelIndex();
	}
	// profile
	else
	{
		auto item = static_cast<AuthElement *>(parent.internalPointer());
		if(item->getKind() != AuthElement::Account)
		{
			return QModelIndex();
		}
		auto parentAcc = (BaseAccount *) item;
		auto prof = parentAcc->operator[](row);
		if(prof)
		{
			return createIndex(row, column, prof);
		}
		return QModelIndex();
	}
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
	{
		return m_store->numAccounts();
	}
	auto item = static_cast<AuthElement *>(parent.internalPointer());
	if(item->getKind() == AuthElement::Account)
	{
		auto acc = (BaseAccount *)item;
		if(acc->size() > 1)
			return acc->size();
	}
	return 0;
}

QVariant AccountModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role != Qt::DisplayRole)
	{
		return QVariant();
	}
	switch(section)
	{
		case DefaultColumn:
			return tr("Default");
		case NameColumn:
			return tr("Username");
		case TypeColumn:
			return tr("Type");
		default:
			return QVariant();
	}
}

Qt::ItemFlags AccountModel::flags(const QModelIndex &index) const
{
	if (index.row() < 0 || index.row() >= rowCount(parent(index)) || !index.isValid())
	{
		return Qt::NoItemFlags;
	}

	return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool AccountModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.row() < 0 || index.row() >= rowCount(parent(index)) || !index.isValid())
	{
		return false;
	}

	auto item = static_cast<AuthElement *>(index.internalPointer());
	if(role == Qt::CheckStateRole)
	{
		if(value == Qt::Checked)
		{
			item->setDefault();
		}
		else
		{
			item->unsetDefault();
		}
		return true;
	}
	return false;
}

QVariant AccountModel::accountData(BaseAccount *account, int column, int role) const
{
	// FIXME: nasty hack
	if(role == Qt::UserRole)
	{
		return QVariant::fromValue((void *)account);
	}
	switch(column)
	{
		case DefaultColumn:
		{
			switch(role)
			{
				case Qt::CheckStateRole:
					return account->isDefault() ? Qt::Checked : Qt::Unchecked;
				default:
					return QVariant();
			}
		}
		case NameColumn:
		{
			switch(role)
			{
				case Qt::DecorationRole:
					return account->avatar();
				case Qt::DisplayRole:
					return account->username();
				case ResourceProxyModel::PlaceholderRole:
					return "icon:hourglass";
				default:
					return QVariant();
			}
		}
		case TypeColumn:
		{
			switch(role)
			{
				case Qt::DecorationRole:
					return account->type()->icon();
				case Qt::DisplayRole:
					return account->type()->text();
				case ResourceProxyModel::PlaceholderRole:
					return "icon:hourglass";
				default:
					return QVariant();
			}
		}
		default:
			return QVariant();
	}
}

QVariant AccountModel::profileData(BaseProfile *profile, int column, int role) const
{
	// FIXME: nasty hack
	if(role == Qt::UserRole)
	{
		return QVariant::fromValue(profile->parent());
	}
	switch(column)
	{
		case DefaultColumn:
		{
			switch(role)
			{
				case Qt::CheckStateRole:
					return profile->isDefault() ? Qt::Checked : Qt::Unchecked;
				default:
					return QVariant();
			}
		}
		case NameColumn:
		{
			switch(role)
			{
				case Qt::DecorationRole:
					return profile->avatar();
				case Qt::DisplayRole:
					return profile->nickname();
				case ResourceProxyModel::PlaceholderRole:
					return "icon:hourglass";
				default:
					return QVariant();
			}
		}
		case TypeColumn:
		{
			switch(role)
			{
				case Qt::DecorationRole:
					return profile->typeIcon();
				case Qt::DisplayRole:
					return profile->typeText();
				case ResourceProxyModel::PlaceholderRole:
					return "icon:hourglass";
				default:
					return QVariant();
			}
		}
		default:
			return QVariant();
	}
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}
	auto row = index.row();
	if(row < 0 || row >= rowCount(parent(index)))
	{
		return QVariant();
	}
	int column = index.column();
	auto item = static_cast<AuthElement *>(index.internalPointer());
	if(item->getKind() == AuthElement::Account)
	{
		auto account = (BaseAccount*) item;
		if(account->size() == 1)
		{
			return profileData(account->operator[](0), column, role);
		}
		return accountData(account, column, role);
	}
	else if(item->getKind() == AuthElement::Profile)
	{
		return profileData((BaseProfile*) item, column, role);
	}

	return QVariant();
}

void AccountModel::itemChanged(int acc, int prof)
{
	auto changedAccount = index(acc, 0);
	auto lastColumn = columnCount(QModelIndex()) - 1;
	emit dataChanged(changedAccount, index(acc, lastColumn));

	auto item = static_cast<BaseAccount *>(changedAccount.internalPointer());
	if(item->size() > 1)
	{
		auto changedProfile = index(prof, 0, changedAccount);
		auto changedProfileB = index(prof, lastColumn, changedAccount);
		emit dataChanged(changedProfile, changedProfileB);
	}
}
