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

#include "Json.h"
#include "BaseInstance.h"
#include "BaseAccount.h"
#include "BaseAccountType.h"
#include "FileSystem.h"
#include "resources/ResourceProxyModel.h"
#include "pathutils.h"

#include "minecraft/auth/MojangAccount.h"
#include "screenshots/auth/ImgurAccount.h"

#define ACCOUNT_LIST_FORMAT_VERSION 3

class AccountTypesModel : public QAbstractListModel
{
public:
	virtual ~AccountTypesModel()
	{
		// FIXME: dangerous
		qDeleteAll(m_content);
	}

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
	{
		if(!index.isValid())
		{
			return QVariant();
		}
		auto row = index.row();
		if(row < 0 || row >= m_content.size())
		{
			return QVariant();
		}
		auto accountType = m_content[row];
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

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override
	{
		return m_content.size();
	}

	void add(BaseAccountType * accountType)
	{
		auto at = m_content.size();
		beginInsertRows(QModelIndex(), at, at);
		m_content.append(accountType);
		endInsertRows();
	}

private:
	QList<BaseAccountType *> m_content;
};

AccountModel::AccountModel()
	: QAbstractListModel(), BaseConfigObject("accounts.json")
{
	m_typesModel = new AccountTypesModel;

	registerType("mojang", new MojangAccountType());
	registerType("imgur", new ImgurAccountType());

	connect(this, &AccountModel::modelReset, this, &AccountModel::listChanged);
	connect(this, &AccountModel::rowsInserted, this, &AccountModel::listChanged);
	connect(this, &AccountModel::rowsRemoved, this, &AccountModel::listChanged);
	connect(this, &AccountModel::dataChanged, this, &AccountModel::listChanged);
}

int AccountModel::columnCount(const QModelIndex &parent) const
{
	return 3;
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
	return m_accounts.size();
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
	if (index.row() < 0 || index.row() >= rowCount(index) || !index.isValid())
	{
		return Qt::NoItemFlags;
	}

	return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool AccountModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.row() < 0 || index.row() >= rowCount(index) || !index.isValid())
	{
		return false;
	}

	auto account = m_accounts[index.row()];
	if(role == Qt::CheckStateRole)
	{
		if(value == Qt::Checked)
		{
			setDefault(account);
		}
		else
		{
			unsetDefault(account->type());
		}
		return true;
	}
	return false;
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
	{
		return QVariant();
	}
	auto row = index.row();
	if(row < 0 || row >= m_accounts.size())
	{
		return QVariant();
	}
	int column = index.column();
	auto account = m_accounts[row];
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
					return isDefault(account) ? Qt::Checked : Qt::Unchecked;
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

AccountModel::~AccountModel()
{
	saveNow();
	// FIXME: dangerous
	qDeleteAll(m_accounts);
	delete m_typesModel;
}

void AccountModel::registerType(const QString &storageId, BaseAccountType * type)
{
	m_types.insert(storageId, type);
	m_typeStorageIds.insert(type, storageId);
	m_typesModel->add(type);
}

BaseAccount *AccountModel::getAccount(const QModelIndex &index) const
{
	return index.isValid() ? m_accounts[index.row()] : nullptr;
}

BaseAccount *AccountModel::getDefault(BaseAccountType *type) const
{
	if(!m_defaults.contains(type))
		return nullptr;
	else return m_defaults[type];
}

BaseAccount *AccountModel::getDefault(const QString &storageId) const
{
	auto t = type(storageId);
	if(!t)
		return nullptr;
	return getDefault(t);
}

void AccountModel::setDefault(BaseAccount *account)
{
	auto currentDefault = m_defaults[account->type()];

	if(!account || currentDefault == account)
		return;


	// this will no longer be default
	if(currentDefault)
	{
		emitRowChanged(m_accounts.indexOf(currentDefault));
	}

	m_defaults[account->type()] = account;
	emitRowChanged(m_accounts.indexOf(currentDefault));

	m_latest = account;
	emit latestChanged();

	scheduleSave();
}

void AccountModel::unsetDefault(BaseAccountType *type)
{
	if(!m_defaults.contains(type))
		return;

	int row = m_accounts.indexOf(m_defaults[type]);
	emitRowChanged(row);
	m_defaults.remove(type);
	scheduleSave();
}

void AccountModel::unsetDefault(const QString &storageId)
{
	auto t = type(storageId);
	if(!t)
		return;
	return unsetDefault(t);
}

bool AccountModel::isDefault(BaseAccount *account) const
{
	return getDefault(account->type()) == account;
}

QList<BaseAccount *> AccountModel::accountsForType(BaseAccountType *type) const
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

QList<BaseAccount *> AccountModel::accountsForType(const QString &storageId) const
{
	auto t = type(storageId);
	if(!t)
		return {};
	return accountsForType(t);
}


bool AccountModel::hasAny(BaseAccountType *type) const
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

bool AccountModel::hasAny(const QString &storageId) const
{
	auto accountType = m_typeStorageIds.key(storageId);
	return hasAny(accountType);
}

QAbstractItemModel *AccountModel::typesModel() const
{
	return m_typesModel;
}

void AccountModel::registerAccount(BaseAccount *account)
{
	auto index = m_accounts.size();
	beginInsertRows(QModelIndex(), index, index);
	m_accounts.append(account);
	endInsertRows();
	connect(account, &BaseAccount::changed, this, &AccountModel::accountChanged);
	m_latest = account;
	emit latestChanged();

	scheduleSave();
}

void AccountModel::unregisterAccount(BaseAccount *account)
{
	auto index = m_accounts.indexOf(account);
	Q_ASSERT(index > -1);

	beginRemoveRows(QModelIndex(), index, index);
	disconnect(account, &BaseAccount::changed, this, &AccountModel::accountChanged);

	// FIXME: nonsense
	m_defaults.remove(account->type());
	//FIXME: dangerous!
	delete account;
	m_accounts.removeAt(index);
	endRemoveRows();

	m_latest = nullptr;
	emit latestChanged();

	scheduleSave();
}

void AccountModel::emitRowChanged(int row)
{
	emit dataChanged(index(row, 0), index(row, columnCount(QModelIndex()) - 1));
}

void AccountModel::accountChanged()
{
	BaseAccount *account = qobject_cast<BaseAccount *>(sender());
	const int row = m_accounts.indexOf(account);
	emitRowChanged(row);

	m_latest = account;
	emit latestChanged();

	scheduleSave();
}

bool AccountModel::doLoad(const QByteArray &data)
{
	using namespace Json;
	const QJsonObject root = requireObject(requireDocument(data));

	QList<BaseAccount *> accs;
	QMap<BaseAccountType *, BaseAccount *> defs;

	const int formatVersion = ensureInteger(root, "formatVersion", 0);
	if (formatVersion == 2) // old, pre-multiauth format
	{
		qDebug() << "Old accounts.json file detected. Before migration:" << requireArray(root, "accounts").size() << "accounts";

		const QString active = ensureString(root, "activeAccount", "");
		for (const QJsonObject &account : requireIsArrayOf<QJsonObject>(root, "accounts"))
		{
			// BaseAccount *acc = createAccount<MojangAccount>();
			BaseAccount *acc = m_types["mojang"]->create();
			acc->load(formatVersion, account);
			accs.append(acc);

			if (!active.isEmpty() && !acc->username().isEmpty() && acc->username() == active)
			{
				defs[acc->type()] = acc;
				m_latest = acc;
			}
		}

		qDebug() << "Loaded" << accs.size() << "old accounts";

		// back up the old file
		QFile::copy(fileName(), fileName() + ".backup");

		// resave now so we save using the new format
		saveNow();
	}
	else if (formatVersion != ACCOUNT_LIST_FORMAT_VERSION)
	{
		const QString newName = fileName() + ".old";
		qWarning() << "Format version mismatch when loading account list. Existing one will be renamed to " << newName;
		QFile file(fileName());
		if (!file.rename(newName))
		{
			throw Exception(tr("Unable to move to %1: %2").arg(newName, file.errorString()));
		}
	}
	else
	{
		const auto accounts = requireIsArrayOf<QJsonObject>(root, "accounts");
		for (const auto account : accounts)
		{
			const QString type = requireString(account, "type");
			if (!m_typeStorageIds.values().contains(type))
			{
				qWarning() << "Unable to load account of type" << type << "(unknown factory)";
			}
			else
			{
				BaseAccount *acc = m_types[type]->create();
				acc->load(formatVersion, account);
				accs.append(acc);
			}
		}

		const auto defaults = requireIsArrayOf<QJsonObject>(root, "defaults");
		for (const auto def : defaults)
		{
			const int index = requireInteger(def, "account");
			if (index >= 0 && index < accs.size())
			{
				defs[m_typeStorageIds.key(requireString(def, "type"))] = accs.at(index);
			}
		}
	}

	m_defaults = defs;
	for (BaseAccount *acc : accs)
	{
		connect(acc, &BaseAccount::changed, this, &AccountModel::accountChanged);
	}
	beginResetModel();
	m_accounts = accs;
	endResetModel();

	return true;
}

QByteArray AccountModel::doSave() const
{
	using namespace Json;
	QJsonArray accounts;
	for (const auto account : m_accounts)
	{
		QJsonObject obj = account->save();
		obj.insert("type", m_typeStorageIds[account->type()]);
		accounts.append(obj);
	}
	QJsonArray defaults;
	for (auto it = m_defaults.constBegin(); it != m_defaults.constEnd(); ++it)
	{
		QJsonObject obj;
		obj.insert("type", m_typeStorageIds[it.key()]);
		obj.insert("account", m_accounts.indexOf(it.value()));
		defaults.append(obj);
	}

	QJsonObject root;
	root.insert("formatVersion", ACCOUNT_LIST_FORMAT_VERSION);
	root.insert("accounts", accounts);
	root.insert("defaults", defaults);
	return toText(root);
}
