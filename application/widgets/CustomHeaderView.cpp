#include "CustomHeaderView.h"
#include <QComboBox>
#include <qcheckbox.h>

CustomHeaderView::CustomHeaderView(QWidget *parent) : QHeaderView(Qt::Horizontal, parent)
{
	connect(this, &QHeaderView::sectionResized, this, &CustomHeaderView::handleSectionResized);
	connect(this, &QHeaderView::sectionMoved, this, &CustomHeaderView::handleSectionMoved);
	// setMovable(true);
}

CustomHeaderView::~CustomHeaderView()
{
	allBox->deleteLater();
}

void CustomHeaderView::showEvent(QShowEvent *evt)
{
	fixCheckPosition();
	QHeaderView::showEvent(evt);
}

void CustomHeaderView::ensureCheckBox()
{
	allBox = new QCheckBox(this);
}

void CustomHeaderView::handleSectionResized(int i)
{
	ensureCheckBox();
	int logical = logicalIndex(i);
	if(logical == 0)
	{
		allBox->setGeometry(sectionViewportPosition(logical), 0, sectionSize(logical), height());
	}
}

void CustomHeaderView::handleSectionMoved(int logical, int oldVisualIndex, int newVisualIndex)
{
	for (int i = qMin(oldVisualIndex, newVisualIndex); i < count(); i++)
	{
		int logical = logicalIndex(i);
		if(logical == 0)
		{
			allBox->setGeometry(sectionViewportPosition(logical), 0, sectionSize(logical), height());
			break;
		}
	}
}

void CustomHeaderView::fixCheckPosition()
{
	ensureCheckBox();
	if (isSectionHidden(0))
	{
		allBox->hide();
	}
	else
	{
		allBox->setGeometry(sectionViewportPosition(0), 0, sectionSize(0), height());
		allBox->show();
	}
}
