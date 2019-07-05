#pragma once

#include "Utilities/xml.hpp"
#include "World/Entity/entity.hpp"
#include <string>
#include <vector>

namespace ph{

class SceneParser
{
public:
	SceneParser(GameData* const, Entity& root, const std::string fileName);
private:
	void getResources(const Xml& sceneSourceCode);
	void loadTextures(const Xml& loadingNode);

	void loadScene(const Xml& sceneSourceCode);
	void loadMap(const Xml& rootNode);
	void loadPlayer(const Xml& rootNode);
	void loadGroups(const Xml& rootNode);
	void loadNpcGroup(const Xml& npcGroupNode);
	void loadEnemiesGroup(const Xml& enemyGroupNode);
	void loadZombies(const std::vector<Xml>& zombieNodes);
	void loadSpawners(const Xml& spawnersGroupNode);

private:
	Entity& mRoot;
	GameData* const mGameData;
};

}
