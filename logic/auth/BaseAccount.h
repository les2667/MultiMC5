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

#include <QObject>
#include <QString>
#include <QMap>
#include <QMetaType>
#include <memory>
#include "BaseSession.h"
#include "AccountFileFormat.h"
#include "BaseAccountType.h"
#include "AuthElement.h"

class BaseProfile;
class Task;
class QJsonObject;


class BaseAccount : public AuthElement
{
	friend class BaseProfile;
public:
	explicit BaseAccount(BaseAccountType *type);
	virtual ~BaseAccount()
	{
	}

	BaseAccountType *type() const
	{
		return m_type;
	}

	virtual QString avatar() const
	{
		return QString();
	}

	virtual QString bigAvatar() const
	{
		return avatar();
	}

	virtual Task *createLoginTask(const QString &username, const QString &password,
								  SessionPtr session = nullptr) = 0;
	virtual Task *createCheckTask(SessionPtr session = nullptr) = 0;
	virtual Task *createLogoutTask(SessionPtr session = nullptr) = 0;

	QString username() const
	{
		return m_username;
	}

	void setUsername(const QString &username);

	bool hasToken(const QString &key) const
	{
		return m_tokens.contains(key);
	}

	QString token(const QString &key) const
	{
		return m_tokens[key];
	}

	void setToken(const QString &key, const QString &token);

	virtual void load(AccountFileFormat formatVersion, const QJsonObject &obj);
	virtual QJsonObject save() const;

	virtual void setDefault() override;
	virtual void unsetDefault() override;
	virtual bool isDefault() const override;

	// called by base account type
	void notifyDefault();

	virtual std::size_t size() const = 0;
	virtual BaseProfile *operator[](std::size_t index) = 0;
	virtual BaseProfile *currentProfile() = 0;

	Type getKind() final override
	{
		return Account;
	}

protected:
	QString m_username;
	QMap<QString, QString> m_tokens;
	BaseAccountType *m_type;
};
Q_DECLARE_METATYPE(BaseAccount *);
