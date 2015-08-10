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
class BaseProfile;
class QString;

class BaseAccountType : public QObject
{
	friend class BaseProfile;
	friend class BaseAccount;
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

	bool isDefault(const BaseAccount * account) const;
	bool isDefault(const BaseProfile * profile) const;

	BaseAccount * getDefaultAccount()
	{
		return m_defaultAccount;
	}

	BaseProfile * getDefaultProfile()
	{
		return m_defaultProfile;
	}

signals:
	void defaultProfileChanged(BaseProfile * oldDef, BaseProfile * newDef);
	void defaultAccountChanged(BaseAccount * oldDef, BaseAccount * newDef);

protected:
	bool notifyDefaultAccount(BaseAccount * account);
	bool notifyDefaultProfile(BaseProfile * profile);

private:
	void setDefault(BaseAccount * account, BaseProfile * profile);

private:
	BaseAccount * m_defaultAccount = nullptr;
	BaseProfile * m_defaultProfile = nullptr;
};
