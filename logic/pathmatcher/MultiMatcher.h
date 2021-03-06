#include "IPathMatcher.h"
#include <SeparatorPrefixTree.h>
#include <QRegularExpression>

class MultiMatcher : public IPathMatcher
{
public:
	virtual ~MultiMatcher() {};
	MultiMatcher()
	{
	}
	MultiMatcher &add(Ptr add)
	{
		m_matchers.append(add);
		return *this;
	}

	virtual bool matches(const QString &string)  override
	{
		for(auto iter: m_matchers)
		{
			if(iter->matches(string))
			{
				return true;
			}
		}
		return false;
	}

	QList<Ptr> m_matchers;
};
