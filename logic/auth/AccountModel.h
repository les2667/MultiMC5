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
#include <memory>
#include <map>
#include "BaseConfigObject.h"
#include "BaseAccount.h"
#include "BaseAccountType.h"

class Container;
class AccountTypesModel;
class BaseInstance;
using InstancePtr = std::shared_ptr<BaseInstance>;

class AccountModel : public QAbstractListModel, public BaseConfigObject
{
	Q_OBJECT

	enum Columns
	{
		DefaultColumn,
		NameColumn,
		TypeColumn
	};

public:
	explicit AccountModel();
	~AccountModel();

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	void registerType(BaseAccountType * type);

	BaseAccountType *type(const QString & storageId) const
	{
		auto iter = m_types.find(storageId);
		if(iter == m_types.end())
			return nullptr;
		return *iter;
	}

	//FIXME: won't work through generic sort&filter proxy
	BaseAccount *getAccount(const QModelIndex &index) const;

	/// The latest account is the account that was most recently changed
	BaseAccount *latest() const
	{
		return m_latest;
	}

	QList<BaseAccount *> accountsForType(BaseAccountType *type) const;
	QList<BaseAccount *> accountsForType(const QString& storageId) const;

	bool hasAny(BaseAccountType *type) const;
	bool hasAny(const QString & storageId) const;

	QAbstractItemModel *typesModel() const;

signals:
	void listChanged();
	void latestChanged();

public slots:
	void registerAccount(BaseAccount *account);
	void unregisterAccount(BaseAccount *account);

private slots:
	void accountChanged();
	void defaultChanged(BaseAccount *oldDef, BaseAccount *newDef);

protected:
	bool doLoad(const QByteArray &data) override;
	QByteArray doSave() const override;

private:
	void emitRowChanged(int row);

	BaseAccount *m_latest = nullptr;

	// mappings between type name and type
	QMap<QString, BaseAccountType *> m_types;

	// stored account types
	AccountTypesModel *m_typesModel;

	QList<BaseAccount *> m_accounts;
};
