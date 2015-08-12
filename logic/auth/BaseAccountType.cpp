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
	m_defaultAccount = account;
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
	auto keep = m_defaultProfile;
	m_defaultProfile = profile;
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
