#include "ModelTester.h"

#include <QDir>
#include <QTemporaryDir>

#include "logic/auth/AccountModel.h"
#include "logic/auth/BaseAccountType.h"
#include "logic/auth/BaseAccount.h"
#include "logic/minecraft/auth/MojangAccount.h"
#include "logic/FileSystem.h"
#include "logic/Json.h"
#include "tests/TestUtil.h"

class AsdfAccount : public BaseAccount
{
public:
	explicit AsdfAccount(BaseAccountType *type) : BaseAccount(type) {}

	Task *createLoginTask(const QString &username, const QString &password, SessionPtr session) override { return nullptr; }
	Task *createCheckTask(SessionPtr session) override { return nullptr; }
	Task *createLogoutTask(SessionPtr session) override { return nullptr; }
};
class AsdfAccountType : public BaseAccountType
{
public:
	QString text() const override { return QString(); }
	QString icon() const override { return QString(); }
	QString usernameText() const override { return QString(); }
	QString passwordText() const override { return QString(); }
	Type type() const override { return UsernamePassword; }

	virtual BaseAccount *create()
	{
		return new AsdfAccount(this);
	}
};

class AccountModelTest : public ModelTester
{
	Q_OBJECT
public:
	std::shared_ptr<QAbstractItemModel> createModel(const int = 0) const override
	{
		auto m = std::make_shared<AccountModel>();
		m->registerType("asdf", new AsdfAccountType());
		m->setSaveTimeout(INT_MAX);
		return m;
	}
	void populate(std::shared_ptr<QAbstractItemModel> model, const int = 0) const override
	{
		auto m = std::dynamic_pointer_cast<AccountModel>(model);
		m->registerAccount(m->type("mojang")->create());
	}

private:
	void testFormatRoundtrip(QString originalFilename, bool backupMustBeCreated)
	{
		auto checkModel = [](AccountModel *model)
		{
			QCOMPARE(model->rowCount(QModelIndex()), 2);
			QVERIFY(model->hasAny("mojang"));
			QCOMPARE(model->accountsForType("mojang").size(), 2);

			MojangAccount *first = dynamic_cast<MojangAccount *>(model->accountsForType("mojang").first());
			QVERIFY(first);
			QCOMPARE(first->username(), QString("arthur.philip@dent.co.uk"));
			QCOMPARE(first->clientToken(), QString("f11bc5a96e8428cae87df606c6ed05cb"));
			QCOMPARE(first->accessToken(), QString("214c57e4fe0b58253e3409cdd5e63053"));
			QCOMPARE(first->profiles().size(), 1);
			QCOMPARE(first->profiles().first().id, QString("d716718a0ede7865c8a4a00e9cb1b6f5"));
			QCOMPARE(first->profiles().first().legacy, false);
			QCOMPARE(first->profiles().first().name, QString("IWantTea"));

			MojangAccount *second = dynamic_cast<MojangAccount *>(model->accountsForType("mojang").at(1));
			QVERIFY(second);
			QCOMPARE(second->username(), QString("zaphod.beeblebrox@galaxy.gov"));
			QCOMPARE(second->clientToken(), QString("d03a2bcf2d1cc467042c7b2680ba947d"));
			QCOMPARE(second->accessToken(), QString("204fe2edcee69f8c207c392e6cc25c9c"));
			QCOMPARE(second->profiles().size(), 1);
			QCOMPARE(second->profiles().first().id, QString("40db0352edab1d1afb8443a34680ef10"));
			QCOMPARE(second->profiles().first().legacy, false);
			QCOMPARE(second->profiles().first().name, QString("IAmTheBest"));

			QVERIFY(model->isDefault(first));
		};

		QFile::copy(QFINDTESTDATA(originalFilename), "accounts.json");
		std::shared_ptr<AccountModel> model = std::make_shared<AccountModel>();

		// load old format, ensure we loaded the right thing
		QVERIFY(model->loadNow());
		checkModel(model.get());

		// save new format
		model->saveNow();

		// verify a backup was created
		if(backupMustBeCreated)
		{
			QVERIFY(TestsInternal::compareFiles(QFINDTESTDATA(originalFilename), "accounts.json.backup")); // ensure the backup is created
		}

		// load again, ensure nothing got lost in translation
		model.reset(new AccountModel);
		QVERIFY(model->loadNow());
		checkModel(model.get());
	}

private slots:
	void test_Migrate_V2_to_V3()
	{
		testFormatRoundtrip("tests/data/accounts_v2.json", true);
	}

	void test_RoundTrip()
	{
		testFormatRoundtrip("tests/data/accounts_v3.json", false);
	}

	void test_Types()
	{
		std::shared_ptr<AccountModel> model = std::dynamic_pointer_cast<AccountModel>(createModel());
		QVERIFY(model->typesModel());
		QVERIFY(model->type("mojang"));
	}

	void test_Querying()
	{
		std::shared_ptr<AccountModel> model = std::dynamic_pointer_cast<AccountModel>(createModel());
		populate(model);

		BaseAccount *account = model->getAccount(model->index(0, 0));
		QVERIFY(account);
		QCOMPARE(model->hasAny("mojang"), true);
		QCOMPARE(model->accountsForType("mojang"), QList<BaseAccount *>() << account);
		QCOMPARE(model->latest(), account);
	}

	void test_Defaults()
	{
		TestsInternal::setupTestingEnv();

		std::shared_ptr<AccountModel> model = std::dynamic_pointer_cast<AccountModel>(createModel());
		populate(model);
		populate(model);

		InstancePtr instance1 = TestsInternal::createInstance();
		InstancePtr instance2 = TestsInternal::createInstance();

		BaseAccount *acc1 = model->getAccount(model->index(0, 0));
		BaseAccount *acc2 = model->getAccount(model->index(1, 0));
		BaseAccount *accNull = nullptr;
		QVERIFY(acc1);
		QVERIFY(acc2);

		// no default set
		QCOMPARE(model->getDefault("mojang"), accNull);
		QCOMPARE(model->getDefault("asdf"), accNull);

		model->setDefault(acc2);
		// global default
		QCOMPARE(model->getDefault("mojang"), acc2);
		QCOMPARE(model->getDefault("asdf"), accNull);

		model->unsetDefault("mojang");
		// unsetting global default
		QCOMPARE(model->getDefault("mojang"), accNull);
		QCOMPARE(model->getDefault("asdf"), accNull);
	}
};

QTEST_GUILESS_MAIN(AccountModelTest)

#include "tst_AccountModel.moc"
