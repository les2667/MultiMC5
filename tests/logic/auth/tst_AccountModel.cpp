#include "ModelTester.h"

#include <QDir>
#include <QTemporaryDir>

#include "logic/auth/AccountModel.h"

#include <auth/AccountStore.h>
#include "logic/minecraft/auth/MojangAccount.h"
#include "logic/FileSystem.h"
#include "logic/Json.h"
#include <screenshots/auth/ImgurAccount.h>
#include "tests/TestUtil.h"

class AccountModelTest : public ModelTester
{
	Q_OBJECT
public:
	std::shared_ptr<QAbstractItemModel> createModel(const int = 0) const override
	{
		auto store = std::make_shared<AccountStore>();
		store->registerType(new MojangAccountType());
		store->registerType(new ImgurAccountType());
		store->registerType(new AsdfAccountType());
		store->setSaveTimeout(INT_MAX);
		return store;
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
			{
				auto prof = first->profiles().first();
				QVERIFY(prof);
				QCOMPARE(prof->profileId(), QString("d716718a0ede7865c8a4a00e9cb1b6f5"));
				QCOMPARE(prof->m_legacy, false);
				QCOMPARE(prof->nickname(), QString("IWantTea"));
			}

			MojangAccount *second = dynamic_cast<MojangAccount *>(model->accountsForType("mojang").at(1));
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
		std::shared_ptr<AccountModel> model = std::dynamic_pointer_cast<AccountModel>(createModel());

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
		model = std::dynamic_pointer_cast<AccountModel>(createModel());
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
	}

	void test_Defaults()
	{
		TestsInternal::setupTestingEnv();

		QFile::copy(QFINDTESTDATA("tests/data/accounts_v3.json"), "accounts.json");
		std::shared_ptr<AccountModel> model = std::dynamic_pointer_cast<AccountModel>(createModel());

		// load old format, ensure we loaded the right thing
		QVERIFY(model->loadNow());

		BaseAccount *arthur = model->getAccount(model->index(0, 0));
		auto arthurProfile = arthur->currentProfile();
		BaseAccount *zaphod = model->getAccount(model->index(1, 0));
		auto zaphodProfile = zaphod->currentProfile();

		BaseProfile *profNull = nullptr;
		QVERIFY(arthur);
		QVERIFY(zaphod);

		auto mojang = model->type("mojang");
		QVERIFY(mojang);

		// first profile of first account is the default... as loaded from file
		QCOMPARE(mojang->getDefault(), arthurProfile);

		// global default
		zaphod->setDefault();
		QCOMPARE(zaphod->operator[](0)->isDefault(), true);
		QCOMPARE(mojang->getDefault(), zaphod->operator[](0));


		// unset by type
		mojang->unsetDefault();
		QCOMPARE(mojang->getDefault(), profNull);
		QCOMPARE(zaphod->isDefault(), false);
		QCOMPARE(zaphod->operator[](0)->isDefault(), false);
	}
};

QTEST_GUILESS_MAIN(AccountModelTest)

#include "tst_AccountModel.moc"
