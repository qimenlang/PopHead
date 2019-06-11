#ifndef POPHEAD_WORLD_ENTITY_OBJECTS_MAP_H_
#define POPHEAD_WORLD_ENTITY_OBJECTS_MAP_H_

#include "World/Entity/object.hpp"
#include <string>
#include <vector>

namespace ph{

class Map : public Object
{
public:
    Map(GameData* gameData, std::string name, const std::string& xmlFilename, float scale = 1);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	struct TilesetsData {
		std::vector<std::string> sources;
		std::vector<unsigned> columnsCounts;
		std::vector<unsigned> gid;
		std::vector<unsigned> tileCounts;
	};

	std::vector<sf::Sprite> mTiles;
};


}

#endif // POPHEAD_WORLD_ENTITY_OBJECTS_MAP_H_
