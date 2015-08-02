#include <QTest>
#include "TestUtil.h"

#include "logic/minecraft/auth/MojangSessionUtils.h"

class MojangSessionUtilsTest : public QObject
{
	using Model = MojangSessionUtils::PlayerModel;

	Q_OBJECT
private
slots:
	void test_Steve()
	{
		QString testData = "{\"id\":\"db6c3fe809694ec4bebb269e0e673f1f\",\"name\":\"peterix\",\"properties\":[{\"name\":\"textures\",\"value\":\"eyJ0aW1lc3RhbXAiOjE0Mzg0Nzc3Mjc3OTgsInByb2ZpbGVJZCI6ImRiNmMzZmU4MDk2OTRlYzRiZWJiMjY5ZTBlNjczZjFmIiwicHJvZmlsZU5hbWUiOiJwZXRlcml4IiwidGV4dHVyZXMiOnsiU0tJTiI6eyJ1cmwiOiJodHRwOi8vdGV4dHVyZXMubWluZWNyYWZ0Lm5ldC90ZXh0dXJlLzY3OWU4M2Y3YWViMDMzNGM2NjhhYmY0NGExMTFhYWFjZjcxOWRjYmM3ZTU4ZGQyMzE1N2VlOWZiZDZiNjgxIn19fQ==\"}]}";
		auto resp = MojangSessionUtils::ProfileResponse(testData.toLatin1());
		QCOMPARE(resp.playerModel, MojangSessionUtils::Steve);
		qDebug() << "Skin" << resp.skinUrl;
		qDebug() << "Cape" << resp.capeUrl;
	}
	void test_Alex()
	{
		QString testData = "{\"id\":\"4363286331474710a1c360f43fbf380a\",\"name\":\"_Peterix_\",\"properties\":[{\"name\":\"textures\",\"value\":\"eyJ0aW1lc3RhbXAiOjE0Mzg0Nzc1NzU0ODgsInByb2ZpbGVJZCI6IjQzNjMyODYzMzE0NzQ3MTBhMWMzNjBmNDNmYmYzODBhIiwicHJvZmlsZU5hbWUiOiJfUGV0ZXJpeF8iLCJ0ZXh0dXJlcyI6e319\"}]}";
		//QString testData = "{\"id\":\"4566e69fc90748ee8d71d7ba5aa00d20\",\"name\":\"Thinkofdeath\",\"properties\":[{\"name\":\"textures\",\"value\":\"eyJ0aW1lc3RhbXAiOjE0Mzg0NzY4MzMyMjMsInByb2ZpbGVJZCI6IjQ1NjZlNjlmYzkwNzQ4ZWU4ZDcxZDdiYTVhYTAwZDIwIiwicHJvZmlsZU5hbWUiOiJUaGlua29mZGVhdGgiLCJ0ZXh0dXJlcyI6eyJTS0lOIjp7InVybCI6Imh0dHA6Ly90ZXh0dXJlcy5taW5lY3JhZnQubmV0L3RleHR1cmUvMTNlODFiOWUxOWFiMWVmMTdhOTBjMGFhNGUxMDg1ZmMxM2NkNDdjZWQ1YTdhMWE0OTI4MDNiMzU2MWU0YTE1YiJ9LCJDQVBFIjp7InVybCI6Imh0dHA6Ly90ZXh0dXJlcy5taW5lY3JhZnQubmV0L3RleHR1cmUvYTc4OTMyNzBmN2RmNjdhMjBlYTljOGFiNTJkN2FmZjIzMjcwZmM0M2ExMWZiM2RhZTYyMjkxNjM0YWY0MjQifX19\"}]}";
		auto resp = MojangSessionUtils::ProfileResponse(testData.toLatin1());
		QCOMPARE(resp.playerModel, MojangSessionUtils::Alex);
		qDebug() << "Skin" << resp.skinUrl;
		qDebug() << "Cape" << resp.capeUrl;
	}

	void test_Other()
	{
		QString testData = "{\"id\":\"4566e69fc90748ee8d71d7ba5aa00d20\",\"name\":\"Thinkofdeath\",\"properties\":[{\"name\":\"textures\",\"value\":\"eyJ0aW1lc3RhbXAiOjE0Mzg0NzY4MzMyMjMsInByb2ZpbGVJZCI6IjQ1NjZlNjlmYzkwNzQ4ZWU4ZDcxZDdiYTVhYTAwZDIwIiwicHJvZmlsZU5hbWUiOiJUaGlua29mZGVhdGgiLCJ0ZXh0dXJlcyI6eyJTS0lOIjp7InVybCI6Imh0dHA6Ly90ZXh0dXJlcy5taW5lY3JhZnQubmV0L3RleHR1cmUvMTNlODFiOWUxOWFiMWVmMTdhOTBjMGFhNGUxMDg1ZmMxM2NkNDdjZWQ1YTdhMWE0OTI4MDNiMzU2MWU0YTE1YiJ9LCJDQVBFIjp7InVybCI6Imh0dHA6Ly90ZXh0dXJlcy5taW5lY3JhZnQubmV0L3RleHR1cmUvYTc4OTMyNzBmN2RmNjdhMjBlYTljOGFiNTJkN2FmZjIzMjcwZmM0M2ExMWZiM2RhZTYyMjkxNjM0YWY0MjQifX19\"}]}";
		auto resp = MojangSessionUtils::ProfileResponse(testData.toLatin1());
		QCOMPARE(resp.playerModel, MojangSessionUtils::Steve);
		qDebug() << "Skin" << resp.skinUrl;
		qDebug() << "Cape" << resp.capeUrl;
	}

	void test_Uuid_data()
	{
		QTest::addColumn<QString>("uuid");
		QTest::addColumn<Model>("model");

		// Steve UUIDs
		QTest::newRow("steve1") << "fffffff0-ffff-fff0-ffff-fff0fffffff0" << Model::Steve;
		QTest::newRow("steve2") << "fffffff0-ffff-fff0-ffff-fff1fffffff1" << Model::Steve;
		QTest::newRow("steve3") << "fffffff0-ffff-fff1-ffff-fff0fffffff1" << Model::Steve;
		QTest::newRow("steve4") << "fffffff0-ffff-fff1-ffff-fff1fffffff0" << Model::Steve;
		QTest::newRow("steve5") << "fffffff1-ffff-fff0-ffff-fff0fffffff1" << Model::Steve;
		QTest::newRow("steve6") << "fffffff1-ffff-fff0-ffff-fff1fffffff0" << Model::Steve;
		QTest::newRow("steve7") << "fffffff1-ffff-fff1-ffff-fff0fffffff0" << Model::Steve;
		QTest::newRow("steve8") << "fffffff1-ffff-fff1-ffff-fff1fffffff1" << Model::Steve;

		// Alex UUIDs
		QTest::newRow("alex1") << "fffffff0-ffff-fff0-ffff-fff0fffffff1" << Model::Alex;
		QTest::newRow("alex2") << "fffffff0-ffff-fff0-ffff-fff1fffffff0" << Model::Alex;
		QTest::newRow("alex3") << "fffffff0-ffff-fff1-ffff-fff0fffffff0" << Model::Alex;
		QTest::newRow("alex4") << "fffffff0-ffff-fff1-ffff-fff1fffffff1" << Model::Alex;
		QTest::newRow("alex5") << "fffffff1-ffff-fff0-ffff-fff0fffffff0" << Model::Alex;
		QTest::newRow("alex6") << "fffffff1-ffff-fff0-ffff-fff1fffffff1" << Model::Alex;
		QTest::newRow("alex7") << "fffffff1-ffff-fff1-ffff-fff0fffffff1" << Model::Alex;
		QTest::newRow("alex8") << "fffffff1-ffff-fff1-ffff-fff1fffffff0" << Model::Alex;
	}
	void test_Uuid()
	{
		QFETCH(QString, uuid);
		QFETCH(Model, model);

		MojangSessionUtils::Uuid _uuid(uuid);

		QCOMPARE(_uuid.getModel(), model);
	}
};

QTEST_GUILESS_MAIN(MojangSessionUtilsTest)

#include "tst_MojangSessionUtils.moc"
