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
#include <QJsonObject>
#include <memory>
#include <map>
#include <BaseConfigObject.h>

class BaseProfile;
class BaseAccount;
class BaseAccountType;
class AccountStore;
typedef std::shared_ptr<AccountStore> AccountStorePtr;

class AccountStore : public QObject, public BaseConfigObject
{
	Q_OBJECT

public:
	explicit AccountStore();
	~AccountStore();

	void registerType(BaseAccountType *type);
	void registerAccount(BaseAccount *account);
	void unregisterAccount(BaseAccount *account);

	BaseAccountType *type(const QString & storageId) const
	{
		auto iter = m_types.find(storageId);
		if(iter == m_types.end())
			return nullptr;
		return *iter;
	}

	QList<BaseAccount *> accountsForType(BaseAccountType *type) const;
	QList<BaseAccount *> accountsForType(const QString& storageId) const;

	bool hasAny(BaseAccountType *type) const;
	bool hasAny(const QString & storageId) const;

	std::size_t numAccounts()
	{
		return m_accounts.size();
	}
	BaseAccount * getAccount(size_t index)
	{
		if(index < numAccounts())
		{
			return m_accounts[index];
		}
		return nullptr;
	}
	int getAccountIndex(BaseAccount * acc)
	{
		return m_accounts.indexOf(acc);
	}

	std::size_t numAccountTypes()
	{
		return m_typeStore.size();
	}
	BaseAccountType * getAccountType(size_t index)
	{
		return m_typeStore[index];
	}

signals:
	void aboutToRemove(int accIndex);
	void removalFinished();

	void aboutToAdd(int accIndex);
	void addingFinished();

	void aboutToReset();
	void resetFinished();

	void itemChanged(int accIndex, int profIndex);

private slots:
	void accountChanged();
	void defaultProfileChanged(BaseProfile *oldDef, BaseProfile *newDef);
	void defaultAccountChanged(BaseAccount *oldDef, BaseAccount *newDef);

private:
	void loadOldMultiMC(QJsonObject root);
	void loadMultiAuth(QJsonObject root);

	bool doLoad(const QByteArray &data) override;
	QByteArray doSave() const override;

private:
	// mappings between type name and type
	QMap<QString, BaseAccountType *> m_types;

	QList<BaseAccountType *> m_typeStore;
	QList<BaseAccount *> m_accounts;
};
