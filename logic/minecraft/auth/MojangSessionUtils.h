#include <QMap>
#include <QUuid>
#include <Json.h>
namespace MojangSessionUtils
{
	enum PlayerModel
	{
		Steve,
		Alex
	};
	class Uuid
	{
	public:
		Uuid()
		{
			data = QByteArray(16,'\0');
		}
		Uuid(QString hex)
		{
			operator=(hex);
		}
		Uuid &operator=(QString hex)
		{
			hex.remove('-');
			data = QByteArray::fromHex(hex.toLatin1());
			return *this;
		}
		PlayerModel getModel()
		{
			if ((data[3] ^ data[7] ^ data[11] ^ data[15]) & 1)
			{
				return Alex;
			}
			return Steve;
		}
		QByteArray data;
	};
	class ProfileResponse
	{
	public:
		Uuid id;
		QString name;
		QString skinUrl;
		QString capeUrl;
		PlayerModel playerModel = Steve;

		ProfileResponse(QByteArray value)
		{
			auto doc = Json::requireDocument(value);
			parse(doc);
		}

		ProfileResponse(QString fileName)
		{
			auto doc = Json::requireDocument(fileName);
			parse(doc);
		}

	private:

		void parse(QJsonDocument doc);
	};

}
Q_DECLARE_METATYPE(MojangSessionUtils::PlayerModel);
