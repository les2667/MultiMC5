// Licensed under the Apache-2.0 license. See README.md for details.

#pragma once

#include <QWidget>
#include <memory>

namespace Ui {
class AccountsWidget;
}

class BaseAccount;
class BaseAccountType;
using InstancePtr = std::shared_ptr<class BaseInstance>;
using SessionPtr = std::shared_ptr<class BaseSession>;

class AccountsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit AccountsWidget(BaseAccountType *type, InstancePtr instance = nullptr, QWidget *parent = nullptr);
	~AccountsWidget();

	void setSession(SessionPtr session);
	void setCancelEnabled(const bool enableCancel);
	void setOfflineEnabled(const bool enabled, const QString &text);

	BaseAccount *account() const;

signals:
	void accepted();
	void rejected();

	//FIXME: bad.
public slots:
	void on_useBtn_clicked();

private slots:
	void on_addBtn_clicked();
	void on_removeBtn_clicked();
	void on_globalDefaultBtn_clicked(bool checked);
	void on_offlineBtn_clicked();
	void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
	void useAccount(BaseAccount *account);

private:
	Ui::AccountsWidget *ui;
	InstancePtr m_instance;
	SessionPtr m_session;
	BaseAccountType *m_requestedType;
	bool m_offlineEnabled = false;
};
