#pragma once

class AuthElement
{
public:
	enum Type
	{
		Account,
		Profile
	};

	virtual ~AuthElement()
	{
	}
	virtual void setDefault() = 0;
	virtual void unsetDefault() = 0;
	virtual bool isDefault() const = 0;
	virtual Type getKind() = 0;
};
