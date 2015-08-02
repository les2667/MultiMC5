#include "MojangSessionUtils.h"

void MojangSessionUtils::ProfileResponse::parse(QJsonDocument doc)
{
	const QJsonObject root = doc.object();
	/*
	{
		"id": "<profile identifier>",
		"name": "<player name>",
		"properties": [
			{
				"name": "textures",
				"value": "<base64 string>",
				"signature": "<base64 string; signed data using Yggdrasil's private key>" // Only provided if ?unsigned=false is appended to url
			}
		]
	}
	*/
	id = Json::requireString(root.value("id"));
	name = Json::requireString(root.value("name"));
	auto properties = Json::requireArray(root.value("properties"));
	for(auto item: properties)
	{
		auto obj = Json::requireObject(item);
		auto name = Json::requireString(obj, "name");
		if(name == "textures")
		{
			auto bytes = QByteArray::fromBase64(Json::requireString(obj, "value").toLatin1());
			auto docInternal = Json::requireDocument(bytes);
			/*
			{
				"timestamp": "<java time in ms>",
				"profileId": "<profile uuid>",
				"profileName": "<player name>",
				"isPublic": "<true or false>",
				"textures": {
					"SKIN": {
						"url": "<player skin URL>"
					},
					"CAPE": {
						"url": "<player cape URL>"
					}
				}
			}
			*/
			auto textures = Json::requireObject(Json::requireObject(docInternal), "textures");
			auto skinObj = Json::ensureObject(textures, "SKIN");
			if(!skinObj.empty())
			{
				skinUrl = Json::requireString(skinObj, "url");
				auto type = Json::ensureString(Json::ensureObject(skinObj, "metadata"), "model");

				if(type == "slim")
				{
					playerModel = Alex;
				}
				else
				{
					playerModel = Steve;
				}
			}
			else
			{
				playerModel = id.getModel();
				if(playerModel == Alex)
				{
					skinUrl = "https://minecraft.net/images/alex.png";
				}
				else
				{
					skinUrl = "https://minecraft.net/images/char.png";
				}
			}

			auto capeObj = Json::ensureObject(textures, "CAPE");
			if(!capeObj.isEmpty())
			{
				capeUrl = Json::requireString(capeObj, "url");
			}
			break;
		}
	}

}
