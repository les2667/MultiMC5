/* Copyright 2013-2015 MultiMC Contributors
 *
 * Authors: Orochimarufan <orochimarufan.x3@gmail.com>
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

#include "MojangAccount.h"
#include "tasks/RefreshTask.h"
#include "tasks/AuthenticateTask.h"
#include "tasks/ValidateTask.h"

#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegExp>
#include <QStringList>
#include <QJsonDocument>
#include <QDebug>

#include "Json.h"
#include <auth/AccountFileFormat.h>

void MojangAccount::load(AccountFileFormat formatVersion, const QJsonObject &object)
{
	if (formatVersion == OldMultiMC || formatVersion == MultiAuth)
	{
		using namespace Json;

		// The JSON object must at least have a username for it to be valid.
		if (!object.value("username").isString())
		{
			throw Exception("Can't load Mojang account info from JSON object. Username field is "
							"missing or of the wrong type.");
		}

		BaseAccount::load(formatVersion, object);

		if (formatVersion == OldMultiMC)
		{
			setClientToken(ensureString(object, "clientToken"));
			setAccessToken(ensureString(object, "accessToken"));
		}

		const QJsonArray profileArray = requireArray(object, "profiles");
		if (profileArray.size() < 1)
		{
			throw Exception("Can't load Mojang account with username \"" + username()
							+ "\". No profiles found.");
		}

		for (const QJsonValue &profileVal : profileArray)
		{
			const QJsonObject profileObject = requireObject(profileVal);
			const QString id = ensureString(profileObject, "id", "");
			const QString name = ensureString(profileObject, "name", "");
			const bool legacy = ensureBoolean(profileObject, QStringLiteral("legacy"), false);
			const bool def = ensureBoolean(profileObject, QStringLiteral("default"), false);
			if (id.isEmpty() || name.isEmpty())
			{
				qWarning() << "Unable to load a profile because it was missing an ID or a name.";
				continue;
			}
			auto prof = new MojangProfile{this, id, name, legacy};
			m_profiles.append(prof);
			if(def)
			{
				prof->setDefault();
			}
		}

		if (object.value("user").isObject())
		{
			MojangAuthSession::User u;
			const QJsonObject userStructure = requireObject(object, "user");
			u.id = userStructure.value("id").toString();
			m_user = u;
		}

		// Get the currently selected profile.
		const QString currentProfile = ensureString(object, "activeProfile", "");
		if (!currentProfile.isEmpty())
		{
			setCurrentProfile(currentProfile);
		}
	}
}

QJsonObject MojangAccount::save() const
{
	QJsonObject json = BaseAccount::save();

	QJsonArray profileArray;
	for (MojangProfile *profile : m_profiles)
	{
		QJsonObject profileObj;
		profileObj.insert("id", profile->m_id);
		profileObj.insert("name", profile->m_name);
		profileObj.insert("legacy", profile->m_legacy);
		if(profile->isDefault())
		{
			profileObj.insert("default", true);
		}
		profileArray.append(profileObj);
	}
	json.insert("profiles", profileArray);

	QJsonObject userStructure;
	{
		userStructure.insert("id", m_user.id);
		/*
		QJsonObject userAttrs;
		for(auto key: m_user.properties.keys())
		{
			auto array = QJsonArray::fromStringList(m_user.properties.values(key));
			userAttrs.insert(key, array);
		}
		userStructure.insert("properties", userAttrs);
		*/
	}
	json.insert("user", userStructure);

	if (m_currentProfile != nullptr)
	{
		json.insert("activeProfile", m_currentProfile->profileId());
	}

	return json;
}


Task *MojangAccount::createLoginTask(const QString &username, const QString &password, SessionPtr session)
{
	setUsername(username);
	if (accountStatus() == NotVerified && password.isEmpty())
	{
		MojangAuthSessionPtr authSession = std::dynamic_pointer_cast<MojangAuthSession>(session);
		if (authSession)
		{
			authSession->status = MojangAuthSession::RequiresPassword;
			populateSessionFromThis(authSession);
		}
		return nullptr;
	}
	return new AuthenticateTask(std::dynamic_pointer_cast<MojangAuthSession>(session),
								username, password, this);
}

Task *MojangAccount::createCheckTask(SessionPtr session)
{
	auto mojangSession = std::dynamic_pointer_cast<MojangAuthSession>(session);
	return new RefreshTask(mojangSession, this);
}

Task *MojangAccount::createLogoutTask(SessionPtr session)
{
	return nullptr; // TODO
}

bool MojangAccount::setCurrentProfile(const QString &profileId)
{
	for (int i = 0; i < m_profiles.length(); i++)
	{
		if (m_profiles[i]->profileId() == profileId)
		{
			m_currentProfile = m_profiles[i];
			// changed();
			return true;
		}
	}
	return false;
}

AccountStatus MojangAccount::accountStatus() const
{
	if (accessToken().isEmpty())
		return NotVerified;
	else
		return Verified;
}

QString MojangAccount::avatar() const
{
	return "icon:mojang";
}

QString MojangAccount::bigAvatar() const
{
	return QString();
}

void MojangAccount::populateSessionFromThis(MojangAuthSessionPtr session)
{
	// the user name. you have to have an user name
	session->username = username();
	// volatile auth token
	session->access_token = accessToken();
	// the semi-permanent client token
	session->client_token = clientToken();
	if (m_currentProfile && !m_currentProfile->profileId().isEmpty())
	{
		// profile name
		session->player_name = currentProfile()->nickname();
		// profile ID
		session->uuid = m_currentProfile->profileId();
		// 'legacy' or 'mojang', depending on account type
		session->user_type = m_currentProfile->m_legacy ? "legacy" : "mojang";
		if (!session->access_token.isEmpty())
		{
			session->session = "token:" + accessToken() + ":" + m_currentProfile->profileId();
		}
		else
		{
			session->session = "-";
		}
	}
	else
	{
		session->player_name = "Player";
		session->session = "-";
	}
	session->u = user();
}
