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

#pragma once

#include <QAbstractListModel>
#include "AccountStore.h"

class AccountModel : public QAbstractItemModel
{
	Q_OBJECT

	enum Columns
	{
		DefaultColumn,
		NameColumn,
		TypeColumn
	};

public:
	explicit AccountModel(AccountStorePtr store);
	virtual ~AccountModel() {};

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QModelIndex parent(const QModelIndex &child) const;
	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

private:
	QVariant accountData(BaseAccount * account, int column, int role) const;
	QVariant profileData(BaseProfile * profile, int column, int role) const;

private slots:
	void itemChanged(int acc, int prof);
	void startRemoving(int acc);
	void removingFinished();
	void startAdding(int acc);
	void addingFinished();
	void startResetting();
	void resettingFinished();

private:
	AccountStorePtr m_store;
};
