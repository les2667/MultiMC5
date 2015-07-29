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

	ui->view->setModel(ResourceProxyModel::mixin<QIcon>(MMC->accountsModel().get()));
	connect(ui->view->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountsWidget::currentChanged);
	currentChanged(ui->view->currentIndex(), QModelIndex());

	connect(ui->cancelBtn, &QPushButton::clicked, this, &AccountsWidget::rejected);

	auto head = ui->view->header();
	head->setStretchLastSection(false);
	head->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	head->setSectionResizeMode(1, QHeaderView::Stretch);
	for(int i = 2; i < head->count(); i++)
		head->setSectionResizeMode(i, QHeaderView::ResizeToContents);

	//FIXME: hacky workaround
	/*
	BaseAccount *def = MMC->accountsModel()->getAccount(m_requestedType);
	if (def)
	{
		ui->view->setCurrentIndex(ui->view->model()->index(MMC->accountsModel()->find(def), 0));
		// need to delay invocation since signals get emitted that we haven't connected to yet
		QMetaObject::invokeMethod(this, "on_useBtn_clicked", Qt::QueuedConnection);
	}
	*/
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
	return MMC->accountsModel()->getAccount(ui->view->currentIndex());
}

void AccountsWidget::on_addBtn_clicked()
{
	if (!m_requestedType)
	{
		AccountLoginDialog dlg(this);
		if (dlg.exec() == QDialog::Accepted)
		{
			MMC->accountsModel()->registerAccount(dlg.account());
		}
	}
	else
	{
		AccountLoginDialog dlg(m_requestedType, this);
		if (dlg.exec() == QDialog::Accepted)
		{
			MMC->accountsModel()->registerAccount(dlg.account());
		}
	}
}

void AccountsWidget::on_removeBtn_clicked()
{
	bool remove = false;
	BaseAccount *account = MMC->accountsModel()->getAccount(ui->view->currentIndex());
	if (!account)
		return;

	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Remove account",
		tr("Are you sure you want to remove account %1?").arg(account->loginUsername()),
		QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		Task *task = account->createLogoutTask(m_session);
		if (task)
		{
			ProgressDialog(this).exec(task);
		}
		MMC->accountsModel()->unregisterAccount(account);
	}
}

void AccountsWidget::on_globalDefaultBtn_clicked(bool checked)
{
	BaseAccount *account = MMC->accountsModel()->getAccount(ui->view->currentIndex());
	if (account)
	{
		if (checked)
		{
			MMC->accountsModel()->setDefault(account);
		}
		else
		{
			MMC->accountsModel()->unsetDefault(account->type());
		}
	}
}

void AccountsWidget::on_useBtn_clicked()
{
	BaseAccount *account = MMC->accountsModel()->getAccount(ui->view->currentIndex());
	if (account)
	{
		ui->groupBox->setEnabled(false);
		ui->useBtn->setEnabled(false);
		ui->view->setEnabled(false);
		ui->offlineBtn->setEnabled(false);
		ui->progressWidget->setVisible(true);
		std::shared_ptr<Task> task = std::shared_ptr<Task>(account->createCheckTask(m_session));
		ui->progressWidget->exec(task);
		ui->progressWidget->setVisible(false);

		if (task->successful())
		{
			emit accepted();
		}
		else
		{
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
		BaseAccount *account = MMC->accountsModel()->getAccount(current);
		ui->groupBox->setEnabled(true);
		Resource::create(account->bigAvatar(), Resource::create("icon:hourglass"))->applyTo(ui->avatarLbl);
		ui->usernameLbl->setText(account->username());
		ui->globalDefaultBtn->setChecked(MMC->accountsModel()->isDefault(account));
		ui->useBtn->setEnabled(m_requestedType == account->type());
	}
}
