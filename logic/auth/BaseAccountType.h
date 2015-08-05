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

#include <QUrl>
#include <QMetaType>
#include <QObject>
#include <memory>

class BaseAccount;
class QString;

class BaseAccountType : public QObject
{
	Q_OBJECT
public:
	virtual ~BaseAccountType()
	{
	}

	enum Type
	{
		OAuth2Pin,
		UsernamePassword
	};

	/**
	 * Create an account based on this account type
	 */
	virtual BaseAccount *create() = 0;

	/**
	 * Internal id of this account type
	 */
	virtual QString id() const = 0;

	/**
	 * Localized name of this account type
	 */
	virtual QString text() const = 0;

	/**
	 * Icon key used for this account type
	 */
	virtual QString icon() const = 0;

	/**
	 * Localized name for the login token
	 */
	virtual QString usernameText() const = 0;

	/**
	 * Localized name for the password token
	 */
	virtual QString passwordText() const = 0;

	/**
	 * Either OAuth2Pin or UsernamePassword type
	 */
	virtual Type type() const = 0;


	/**
	 * The URL for oauth authentication
	 */
	virtual QUrl oauth2PinUrl() const
	{
		return QUrl();
	}

	/**
	 * Determines if this account type can be used.
	 */
	virtual bool isAvailable() const
	{
		return true;
	}

	BaseAccount * getDefault() const
	{
		return m_default;
	}

	void setDefault(BaseAccount * def, bool initial = false)
	{
		if(m_default != def)
		{
			auto keep = m_default;
			m_default = def;
			if(!initial)
			{
				emit defaultChanged(keep, def);
			}
		}
	}

	void unsetDefault()
	{
		if(m_default)
		{
			auto keep = m_default;
			m_default = nullptr;
			emit defaultChanged(keep, m_default);
		}
	}

	bool isDefault(BaseAccount * acct)
	{
		return m_default == acct;
	}

signals:
	void defaultChanged(BaseAccount * oldDef, BaseAccount * newDef);
private:
	BaseAccount * m_default = nullptr;
};

Q_DECLARE_METATYPE(BaseAccountType *)
