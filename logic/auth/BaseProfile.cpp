#include "BaseProfile.h"

void BaseProfile::setDefault()
{
	m_parent->type()->notifyDefaultProfile(this);
}

void BaseProfile::unsetDefault()
{
	if(m_parent->type()->isDefault(this))
	{
		m_parent->type()->notifyDefaultProfile(nullptr);
	}
}

bool BaseProfile::isDefault() const
{
	return m_parent->type()->isDefault(this);
}

BaseAccount *BaseProfile::parent()
{
	return m_parent;
}
