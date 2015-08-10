

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

#include "AccountStore.h"

#include "Json.h"
#include "BaseInstance.h"
#include "BaseAccount.h"
#include "BaseAccountType.h"
#include "AccountFileFormat.h"
#include "BaseProfile.h"
#include "FileSystem.h"
#include "resources/ResourceProxyModel.h"
#include "pathutils.h"

AccountStore::AccountStore(): BaseConfigObject("accounts.json")
{
}

AccountStore::~AccountStore()
{
	saveNow();
	qDeleteAll(m_accounts);
	qDeleteAll(m_typeStore);
}

void AccountStore::registerType(BaseAccountType * type)
{
	Q_ASSERT(!m_types.contains(type->id()));
	m_types.insert(type->id(), type);
	m_typeStore.append(type);
	connect(type, &BaseAccountType::defaultProfileChanged, this, &AccountStore::defaultChanged);
}

void AccountStore::defaultChanged(BaseProfile *oldDef, BaseProfile *newDef)
{
	/*
	if(oldDef)
	{
		emitRowChanged(m_accounts.indexOf(oldDef));
	}
	if(newDef)
	{
		emitRowChanged(m_accounts.indexOf(newDef));
	}
	scheduleSave();
	*/
}

QList<BaseAccount *> AccountStore::accountsForType(BaseAccountType *type) const
{
	QList<BaseAccount *> out;
	for (BaseAccount *acc : m_accounts)
	{
		if (acc->type() == type)
		{
			out.append(acc);
		}
	}
	return out;
}

QList<BaseAccount *> AccountStore::accountsForType(const QString &storageId) const
{
	auto t = type(storageId);
	if(!t)
		return {};
	return accountsForType(t);
}


bool AccountStore::hasAny(BaseAccountType *type) const
{
	for (const BaseAccount *acc : m_accounts)
	{
		if (acc->type() == type)
		{
			return true;
		}
	}
	return false;
}

bool AccountStore::hasAny(const QString &storageId) const
{
	auto accountType = m_types[storageId];
	return hasAny(accountType);
}

void AccountStore::registerAccount(BaseAccount *account)
{
	auto index = m_accounts.size();
	// beginInsertRows(QModelIndex(), index, index);
	m_accounts.append(account);
	// endInsertRows();
	// connect(account, &BaseAccount::changed, this, &AccountStore::accountChanged);
	scheduleSave();
}

void AccountStore::unregisterAccount(BaseAccount *account)
{
	auto index = m_accounts.indexOf(account);
	Q_ASSERT(index > -1);

	// beginRemoveRows(QModelIndex(), index, index);
	// disconnect(account, &BaseAccount::changed, this, &AccountStore::accountChanged);

	account->unsetDefault();
	delete account;
	m_accounts.removeAt(index);
	// endRemoveRows();
	scheduleSave();
}

/*
void AccountStore::emitRowChanged(int row)
{
	emit dataChanged(index(row, 0), index(row, columnCount(QModelIndex()) - 1));
}
*/

void AccountStore::accountChanged()
{
	// BaseAccount *account = qobject_cast<BaseAccount *>(sender());
	// const int row = m_accounts.indexOf(account);
	// emitRowChanged(row);
	scheduleSave();
}

void AccountStore::loadOldMultiMC(QJsonObject root)
{
	using namespace Json;
	int loaded = 0;

	qDebug() << "Old accounts.json file detected. Before migration:" << requireArray(root, "accounts").size() << "accounts";

	// beginResetModel();

	BaseAccount *def = nullptr;

	const QString active = ensureString(root, "activeAccount", "");
	for (const QJsonObject &account : requireIsArrayOf<QJsonObject>(root, "accounts"))
	{
		BaseAccount *acc = m_types["mojang"]->create();
		acc->load(AccountFileFormat::OldMultiMC, account);
		m_accounts.append(acc);

		if (!active.isEmpty() && !acc->username().isEmpty() && acc->username() == active)
		{
			def = acc;
		}
		loaded++;
	}

	if(def)
	{
		def->currentProfile()->setDefault();
	}

	qDebug() << "Loaded" << loaded << "old accounts";

	// back up the old file
	QFile::copy(fileName(), fileName() + ".backup");

	// resave now so we save using the new format
	saveNow();
	// endResetModel();
}

void AccountStore::loadMultiAuth(QJsonObject root)
{
	using namespace Json;
	const auto accounts = requireIsArrayOf<QJsonObject>(root, "accounts");
	for (const auto account : accounts)
	{
		const QString type = requireString(account, "type");
		if (!m_types.contains(type))
		{
			qWarning() << "Unable to load account of type" << type << "(unknown factory)";
		}
		else
		{
			BaseAccount *acc = m_types[type]->create();
			acc->load(AccountFileFormat::MultiAuth, account);
			m_accounts.append(acc);
		}
	}
}

bool AccountStore::doLoad(const QByteArray &data)
{
	using namespace Json;
	QList<BaseAccount *> accs;
	QMap<BaseAccountType *, BaseAccount *> defs;

	int formatVersion = 0;

	const QJsonObject root = requireObject(requireDocument(data));

	formatVersion = ensureInteger(root, "formatVersion", 0);
	switch(formatVersion)
	{
		case AccountFileFormat::OldMultiMC:
		{
			loadOldMultiMC(root);
			break;
		}
		case AccountFileFormat::MultiAuth:
		{
			loadMultiAuth(root);
			break;
		}
		default:
		{
			const QString newName = fileName() + ".old";
			qWarning() << "Format version mismatch when loading account list. Existing one will be renamed to " << newName;
			QFile file(fileName());
			if (!file.rename(newName))
			{
				throw Exception(tr("Unable to move to %1: %2").arg(newName, file.errorString()));
			}
		}
	}

	for (BaseAccount *acc : accs)
	{
		// connect(acc, &BaseAccount::changed, this, &AccountStore::accountChanged);
	}


	return true;
}

QByteArray AccountStore::doSave() const
{
	using namespace Json;
	QJsonArray accounts;
	for (const auto account : m_accounts)
	{
		QJsonObject obj = account->save();
		obj.insert("type", account->type()->id());
		accounts.append(obj);
	}

	QJsonObject root;
	root.insert("formatVersion", AccountFileFormat::Current);
	root.insert("accounts", accounts);
	return toText(root);
}
