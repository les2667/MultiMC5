#include "logic/auth/BaseAccountType.h"
#include "logic/auth/BaseAccount.h"
#include <auth/AccountStore.h>
#include <minecraft/auth/MojangAccount.h>
#include <screenshots/auth/ImgurAccount.h>
#include <TestUtil.h>
#include <memory>

class Task;

class AsdfAccount : public BaseAccount
{
public:
	explicit AsdfAccount(BaseAccountType *type) : BaseAccount(type)
	{
	}

	Task *createLoginTask(const QString &username, const QString &password,
						  SessionPtr session) override
	{
		return nullptr;
	}
	Task *createCheckTask(SessionPtr session) override
	{
		return nullptr;
	}
	Task *createLogoutTask(SessionPtr session) override
	{
		return nullptr;
	}
	virtual BaseProfile *currentProfile()
	{
		return nullptr;
	}
	virtual BaseProfile *operator[](std::size_t index)
	{
		return nullptr;
	}
	virtual std::size_t size() const
	{
		return 0;
	}
};
class AsdfAccountType : public BaseAccountType
{
public:
	QString id() const override
	{
		return "asdf";
	}
	QString text() const override
	{
		return QString();
	}
	QString icon() const override
	{
		return QString();
	}
	QString usernameText() const override
	{
		return QString();
	}
	QString passwordText() const override
	{
		return QString();
	}
	Type type() const override
	{
		return UsernamePassword;
	}

	virtual BaseAccount *create()
	{
		return new AsdfAccount(this);
	}
};

class AccountModelTest : public BaseTest
{
	Q_OBJECT
private:
	std::shared_ptr<AccountStore> createStore() const
	{
		auto store = std::make_shared<AccountStore>();
		store->registerType(new MojangAccountType());
		store->registerType(new ImgurAccountType());
		store->registerType(new AsdfAccountType());
		store->setSaveTimeout(INT_MAX);
		return store;
	}

	void testFormatRoundtrip(QString originalFilename, bool backupMustBeCreated)
	{
		auto checkModel = [](AccountStore *store)
		{
			QCOMPARE(store->numAccounts(), 2UL);
			QVERIFY(store->hasAny("mojang"));
			QCOMPARE(store->accountsForType("mojang").size(), 2);

			MojangAccount *first = dynamic_cast<MojangAccount *>(store->accountsForType("mojang").first());
			QVERIFY(first);
			QCOMPARE(first->username(), QString("arthur.philip@dent.co.uk"));
			QCOMPARE(first->clientToken(), QString("f11bc5a96e8428cae87df606c6ed05cb"));
			QCOMPARE(first->accessToken(), QString("214c57e4fe0b58253e3409cdd5e63053"));
			QCOMPARE(first->profiles().size(), 1);
			{
				auto prof = first->profiles().first();
				QVERIFY(prof);
				QCOMPARE(prof->profileId(), QString("d716718a0ede7865c8a4a00e9cb1b6f5"));
				QCOMPARE(prof->m_legacy, false);
				QCOMPARE(prof->nickname(), QString("IWantTea"));
			}

			MojangAccount *second = dynamic_cast<MojangAccount *>(store->accountsForType("mojang").at(1));
			QVERIFY(second);
			QCOMPARE(second->username(), QString("zaphod.beeblebrox@galaxy.gov"));
			QCOMPARE(second->clientToken(), QString("d03a2bcf2d1cc467042c7b2680ba947d"));
			QCOMPARE(second->accessToken(), QString("204fe2edcee69f8c207c392e6cc25c9c"));
			QCOMPARE(second->profiles().size(), 1);
			{
				auto prof = second->profiles().first();
				QVERIFY(prof);
				QCOMPARE(prof->profileId(), QString("40db0352edab1d1afb8443a34680ef10"));
				QCOMPARE(prof->m_legacy, false);
				QCOMPARE(prof->nickname(), QString("IAmTheBest"));
			}

			QVERIFY(first->isDefault());
		};

		QFile::copy(QFINDTESTDATA(originalFilename), "accounts.json");
		std::shared_ptr<AccountStore> store = createStore();

		// load old format, ensure we loaded the right thing
		QVERIFY(store->loadNow());
		checkModel(store.get());

		// save new format
		store->saveNow();

		// verify a backup was created
		if(backupMustBeCreated)
		{
			QVERIFY(TestsInternal::compareFiles(QFINDTESTDATA(originalFilename), "accounts.json.backup")); // ensure the backup is created
		}

		// load again, ensure nothing got lost in translation
		store = createStore();
		QVERIFY(store->loadNow());
		checkModel(store.get());
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
		auto model = createStore();
		QVERIFY(model->type("mojang"));
	}

	void test_Querying()
	{
		auto store = createStore();
		store->registerAccount(store->type("mojang")->create());

		BaseAccount *account = store->getAccount(0);
		QVERIFY(account);
		QCOMPARE(store->hasAny("mojang"), true);
		QCOMPARE(store->accountsForType("mojang"), QList<BaseAccount *>() << account);
	}

	void test_Defaults()
	{
		TestsInternal::setupTestingEnv();

		QFile::copy(QFINDTESTDATA("tests/data/accounts_v3.json"), "accounts.json");
		auto store = createStore();

		// load old format, ensure we loaded the right thing
		QVERIFY(store->loadNow());

		BaseAccount *arthur = store->getAccount(0);
		auto arthurProfile = arthur->currentProfile();
		BaseAccount *zaphod = store->getAccount(1);
		auto zaphodProfile = zaphod->currentProfile();

		BaseProfile *profNull = nullptr;
		QVERIFY(arthur);
		QVERIFY(zaphod);

		auto mojang = store->type("mojang");
		QVERIFY(mojang);

		// first profile of first account is the default... as loaded from file
		QVERIFY(arthur->isDefault());
		QVERIFY(arthurProfile->isDefault());
		QVERIFY(!zaphod->isDefault());
		QVERIFY(!zaphodProfile->isDefault());


		// set default to zaphod
		zaphod->setDefault();
		QVERIFY(!arthur->isDefault());
		QVERIFY(!arthurProfile->isDefault());
		QVERIFY(zaphod->isDefault());
		QVERIFY(zaphodProfile->isDefault());

		// unset default
		zaphod->unsetDefault();
		QVERIFY(!arthur->isDefault());
		QVERIFY(!arthurProfile->isDefault());
		QVERIFY(!zaphod->isDefault());
		QVERIFY(!zaphodProfile->isDefault());
	}
};

QTEST_GUILESS_MAIN(AccountModelTest)

#include "tst_AccountStore.moc"
