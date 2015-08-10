#pragma once

class BaseItem
{
public:
	enum Kind
	{
		Account,
		Profile,
		Both
	};
	virtual ~BaseItem()
	{
	}
	virtual void setDefault() = 0;
	virtual void unsetDefault() = 0;
	virtual bool isDefault() const = 0;
	virtual Kind getKind() = 0;
};
