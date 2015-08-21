#pragma once
#include <QHeaderView>

class QCheckBox;
class QShowEvent;
class CustomHeaderView: public QHeaderView
{
public:
	// This is always horizontal
	CustomHeaderView(QWidget *parent = 0);
	virtual ~CustomHeaderView();

	virtual void showEvent(QShowEvent *);

public:
	void fixCheckPosition();

private slots:
	void handleSectionResized(int i);
	void handleSectionMoved(int logical, int oldVisualIndex, int newVisualIndex);

private: /* methods */
	void ensureCheckBox();

private: /* data */
	QCheckBox * allBox;
};