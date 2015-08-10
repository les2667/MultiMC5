#include "BaseAccountType.h"
#include "BaseAccount.h"
#include "BaseProfile.h"

bool BaseAccountType::notifyDefaultAccount(BaseAccount *account)
{
	if(account == m_defaultAccount)
	{
		return false;
	}
	auto keep = m_defaultAccount;
	if(m_defaultAccount)
	{
		m_defaultAccount->notifyDefault();
	}
	m_defaultAccount = account;
	if(m_defaultAccount)
	{
		m_defaultAccount->notifyDefault();
	}
	emit defaultAccountChanged(keep, account);
	return true;
}

bool BaseAccountType::notifyDefaultProfile(BaseProfile *profile)
{
	if(profile == m_defaultProfile)
	{
		return false;
	}
	notifyDefaultAccount(profile? profile->parent(): nullptr);
	if(m_defaultProfile)
	{
		m_defaultProfile->notifyDefault();
	}
	auto keep = m_defaultProfile;
	m_defaultProfile = profile;
	if(m_defaultProfile)
	{
		m_defaultProfile->notifyDefault();
	}
	emit defaultProfileChanged(keep, profile);
	return true;
}

bool BaseAccountType::isDefault(const BaseAccount *account) const
{
	return m_defaultAccount == account;
}

bool BaseAccountType::isDefault(const BaseProfile *profile) const
{
	return m_defaultProfile == profile;
}
