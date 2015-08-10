// Licensed under the Apache-2.0 license. See README.md for details.

#include "AccountsWidget.h"
#include "ui_AccountsWidget.h"

#include <QInputDialog>
#include <QMessageBox>

#include "dialogs/AccountLoginDialog.h"
#include "dialogs/ProgressDialog.h"
#include "auth/BaseAccount.h"
#include "auth/BaseAccountType.h"
#include "auth/AccountModel.h"
#include <auth/BaseProfile.h>
#include "tasks/Task.h"
#include "BaseInstance.h"
#include "Env.h"
#include "resources/ResourceProxyModel.h"
#include "resources/Resource.h"
#include "MultiMC.h"

AccountsWidget::AccountsWidget(BaseAccountType *type, InstancePtr instance, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::AccountsWidget),
	m_instance(instance),
	m_requestedType(type)
{
	ui->setupUi(this);

	ui->useBtn->setVisible(!!m_instance && type);
	ui->progressWidget->setVisible(false);
	ui->cancelBtn->setText(m_instance ? tr("Cancel") : tr("Close"));
	ui->offlineBtn->setVisible(false);

	ui->view->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->view->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->view->setAllColumnsShowFocus(true);
	auto model = ResourceProxyModel::mixin<QIcon>(new AccountModel(MMC->accountsStore()));
	ui->view->setModel(model);

	connect(ui->view->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountsWidget::currentChanged);
	currentChanged(ui->view->currentIndex(), QModelIndex());

	connect(ui->cancelBtn, &QPushButton::clicked, this, &AccountsWidget::rejected);

	auto head = ui->view->header();
	head->setStretchLastSection(false);
	head->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	head->setSectionResizeMode(1, QHeaderView::Stretch);
	for(int i = 2; i < head->count(); i++)
		head->setSectionResizeMode(i, QHeaderView::ResizeToContents);

	BaseAccount *def;
	if (m_requestedType && (def = m_requestedType->getDefaultAccount()))
	{
		for(int i = 0; i < model->rowCount(); i++)
		{
			QVariant value = model->data(model->index(i, 0), Qt::UserRole);
			auto acct = (BaseAccount *)value.value<void *>();
			if(acct == def)
			{
				ui->view->setCurrentIndex(model->index(i, 0));
			}
		}
		launch(def);
	}
}

AccountsWidget::~AccountsWidget()
{
	delete ui;
}

void AccountsWidget::setSession(SessionPtr session)
{
	m_session = session;
}
void AccountsWidget::setCancelEnabled(const bool enableCancel)
{
	ui->cancelBtn->setVisible(enableCancel);
}
void AccountsWidget::setOfflineEnabled(const bool enabled, const QString &text)
{
	ui->offlineBtn->setVisible(m_offlineEnabled = enabled);
	ui->offlineBtn->setText(text);
}

BaseAccount *AccountsWidget::account() const
{
	// FIXME: complete bullshit.
	return MMC->accountsStore()->getAccount(ui->view->currentIndex().row());
}

void AccountsWidget::on_addBtn_clicked()
{
	if (!m_requestedType)
	{
		AccountLoginDialog dlg(this);
		if (dlg.exec() == QDialog::Accepted)
		{
			MMC->accountsStore()->registerAccount(dlg.account());
		}
	}
	else
	{
		AccountLoginDialog dlg(m_requestedType, this);
		if (dlg.exec() == QDialog::Accepted)
		{
			MMC->accountsStore()->registerAccount(dlg.account());
		}
	}
}

void AccountsWidget::on_removeBtn_clicked()
{
	bool remove = false;
	BaseAccount *account = MMC->accountsStore()->getAccount(ui->view->currentIndex().row());
	if (!account)
		return;

	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Remove account",
		tr("Are you sure you want to remove account %1?").arg(account->username()),
		QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		Task *task = account->createLogoutTask(m_session);
		if (task)
		{
			ProgressDialog(this).exec(task);
		}
		MMC->accountsStore()->unregisterAccount(account);
	}
}

void AccountsWidget::on_globalDefaultBtn_clicked(bool checked)
{
	BaseAccount *account = MMC->accountsStore()->getAccount(ui->view->currentIndex().row());
	if (account)
	{
		if (checked)
		{
			account->setDefault();
		}
		else
		{
			account->unsetDefault();
		}
	}
}

void AccountsWidget::launch(BaseAccount *account)
{
	ui->groupBox->setEnabled(false);
	ui->useBtn->setEnabled(false);
	ui->view->setEnabled(false);
	ui->offlineBtn->setEnabled(false);
	ui->progressWidget->setVisible(true);
	std::shared_ptr<Task> task = std::shared_ptr<Task>(account->createCheckTask(m_session));
	if(task)
	{
		ui->progressWidget->exec(task);
		ui->progressWidget->setVisible(false);

		if (task->successful())
		{
			emit accepted();
			return;
		}
	}
	AccountLoginDialog dlg(account, this);
	if (dlg.exec() == AccountLoginDialog::Accepted)
	{
		emit accepted();
	}
	else
	{
		ui->groupBox->setEnabled(true);
		ui->useBtn->setEnabled(true);
		ui->view->setEnabled(true);
		ui->offlineBtn->setEnabled(m_offlineEnabled);
	}
}

void AccountsWidget::on_useBtn_clicked()
{
	BaseAccount *account = MMC->accountsStore()->getAccount(ui->view->currentIndex().row());
	if (account)
	{
		launch(account);
	}
}

void AccountsWidget::on_offlineBtn_clicked()
{
	bool ok = false;
	const QString name = QInputDialog::getText(this, tr("Player name"),
											   tr("Choose your offline mode player name."),
											   QLineEdit::Normal, m_session->defaultPlayerName(), &ok);
	if (!ok)
	{
		return;
	}
	m_session->makeOffline(name.isEmpty() ? m_session->defaultPlayerName() : name);
	emit accepted();
}

void AccountsWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (!current.isValid())
	{
		ui->groupBox->setEnabled(false);
		ui->avatarLbl->setPixmap(QPixmap());
		ui->usernameLbl->setText("");
		ui->globalDefaultBtn->setChecked(false);
		ui->useBtn->setEnabled(false);
	}
	else
	{
		BaseAccount *account = MMC->accountsStore()->getAccount(current.row());
		ui->groupBox->setEnabled(true);
		Resource::create(account->bigAvatar(), Resource::create("icon:hourglass"))->applyTo(ui->avatarLbl);
		ui->usernameLbl->setText(account->username());
		ui->globalDefaultBtn->setChecked(account->isDefault());
		ui->useBtn->setEnabled(m_requestedType == account->type());
	}
}
