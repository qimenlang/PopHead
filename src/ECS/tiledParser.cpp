#include "pch.hpp"
#include "tiledParser.hpp"

#include "Components/physicsComponents.hpp"
#include "Components/charactersComponents.hpp"
#include "Components/graphicsComponents.hpp"
#include "Components/objectsComponents.hpp"
#include "Components/itemComponents.hpp"
#include "Components/particleComponents.hpp"
#include "Components/debugComponents.hpp"

#include "entitiesTemplateStorage.hpp"
#include "Scenes/sceneManager.hpp"
#include "Resources/textureHolder.hpp"
#include "Renderer/API/shader.hpp"

namespace ph {

static void loadEntity(const Xml& entityNode, EntitiesTemplateStorage& templates, 
                       entt::registry& registry, SceneManager& sceneManager, bool* isPlayerOnScene)
{
	auto type = entityNode.getAttribute("type")->toString();

	// NOTE: entity is initialized inside block after if statement. It is declared here for convenience 
	//       so we can capture it with lambda and we don't have to pass it to lambda as argument.
	entt::entity entity;

	auto createDebugName = [&]()
	{
		#ifndef PH_DISTRIBUTION
		auto& debugName = registry.assign<component::DebugName>(entity);
		auto name = entityNode.getAttribute("type")->toString();
		memcpy(debugName.name, name.c_str(), name.length()); 
		#endif
	};

	auto getPositionAttribute = [&]()
	{
		auto x = entityNode.getAttribute("x");
		auto y = entityNode.getAttribute("y");
		PH_ASSERT_UNEXPECTED_SITUATION(x && y, "entity of type " + entityNode.getAttribute("type")->toString() + " doesn't have position");
		return sf::Vector2f(x->toFloat(), y->toFloat());
	};

	auto getSizeAttribute = [&]()
	{
		auto w = entityNode.getAttribute("width");
		auto h = entityNode.getAttribute("height");
		PH_ASSERT_UNEXPECTED_SITUATION(w && h, "entity of type " + entityNode.getAttribute("type")->toString() + " doesn't have size");
		return sf::Vector2f(w->toFloat(), h->toFloat());
	};

	auto getOptionalSizeAttribute = [&]() -> std::optional<sf::Vector2f>
	{
		auto width = entityNode.getAttribute("width");
		auto height = entityNode.getAttribute("height");
		if(width && height)
			return sf::Vector2f(width->toFloat(), height->toFloat());
		else
			return std::nullopt;
	};

	auto loadPosition = [&]()
	{
		auto& bodyRect = registry.get<component::BodyRect>(entity);
		bodyRect.rect.setPosition(getPositionAttribute());
	};

	auto loadSize = [&]()
	{
		auto& bodyRect = registry.get<component::BodyRect>(entity);
		bodyRect.rect.setSize(getSizeAttribute());
	};

	auto loadPositionAndSize = [&]()
	{
		auto& bodyRect = registry.get<component::BodyRect>(entity);
		bodyRect.rect.setPosition(getPositionAttribute());
		bodyRect.rect.setSize(getSizeAttribute());
	};

	auto loadPositionAndOptionalSize = [&]()
	{
		auto& bodyRect = registry.get<component::BodyRect>(entity);
		bodyRect.rect.setPosition(getPositionAttribute());
		if(auto size = getOptionalSizeAttribute())
			bodyRect.rect.setSize(*size);
	};

	auto getProperty = [&](const std::string& propertyName)
	{
		if(const auto propertiesNode = entityNode.getChild("properties")) 
		{
			const std::vector<Xml> properties = propertiesNode->getChildren("property");
			for(const auto& property : properties)
			{
				if(property.getAttribute("name")->toString() == propertyName)
				{
					return *property.getAttribute("value");
				}
			}
		}

		Xml objectTypesFile;
		PH_ASSERT_CRITICAL(objectTypesFile.loadFromFile("scenes/map/objecttypes.xml"),
			"Tiled object type file \"scenes/map/objecttypes.xml\" wasn't loaded correctly!");
		const auto objectTypesNode = objectTypesFile.getChild("objecttypes");
		std::vector<Xml> objectNodes = objectTypesNode->getChildren("objecttype");
		for(const auto& objectNode : objectNodes)
		{
			if(objectNode.getAttribute("name")->toString() == type) 
			{
				std::vector<Xml> propertiesNodes = objectNode.getChildren("property");
				for(const auto& propertyNode : propertiesNodes)
				{
					if(propertyNode.getAttribute("name")->toString() == propertyName)
					{
						return *propertyNode.getAttribute("default");
					}
				}
			}
		}

		PH_UNEXPECTED_SITUATION("property " + propertyName + " for entity type " + type + " wasn't found!");
		return Xml();
	};

	auto loadHealthComponent = [&]()
	{
		auto& healthComponent = registry.get<component::Health>(entity);
		healthComponent.healthPoints = getProperty("hp").toUnsigned();
		healthComponent.maxHealthPoints = getProperty("maxHp").toUnsigned();
	};

	if(type == "Zombie" || type == "SlowZombie")
	{
		entity = templates.createCopy(type, registry);
		loadPosition();
		loadHealthComponent();
		createDebugName();
	}
	else if(type == "Player") 
	{
		*isPlayerOnScene = true;

		entity = templates.createCopy("Player", registry);
		auto& playerBody = registry.get<component::BodyRect>(entity);
		auto& playerCamera = registry.get<component::Camera>(entity);
		
		playerBody.rect.setPosition(sceneManager.hasPlayerPositionForNextScene() ?
			sceneManager.getPlayerPositionForNextScene() : getPositionAttribute());

		playerCamera.camera = Camera(playerBody.rect.getCenter(), sf::Vector2f(640, 360));

		templates.createCopy("Pistol", registry);
		auto shotgun = templates.createCopy("Shotgun", registry);
		auto melee = templates.createCopy("BaseballBat", registry);
		registry.assign<component::CurrentMeleeWeapon>(melee);

		if(getProperty("hasFlashlight").toBool()) 
		{
			component::LightSource flashlight;
			flashlight.offset = {10.f, 10.f};
			flashlight.color = sf::Color(255, 255, 255, 90);
			flashlight.startAngle = 40.f;
			flashlight.endAngle = 40.f;
			flashlight.attenuationAddition = 0.1f;
			flashlight.attenuationFactor = 3.f;
			flashlight.attenuationSquareFactor = 1.5f;
			registry.assign_or_replace<component::LightSource>(entity, flashlight);
		}	

		createDebugName();
	}
	else if(type == "Camera") 
	{
		if(getProperty("isValid").toBool()) 
		{
			entity = templates.createCopy("Camera", registry);
			auto& camera = registry.get<component::Camera>(entity);
			sf::Vector2f pos = getPositionAttribute();
			sf::Vector2f size = getSizeAttribute();
			sf::Vector2f center(pos + (size / 2.f));
			camera.camera = Camera(center, size);
			camera.name = getProperty("name").toString();
			createDebugName();
		}
	}
	else if(type == "BulletBox") 
	{
		entity = templates.createCopy("BulletBox", registry);
		loadPosition();
		auto& bullets = registry.get<component::Bullets>(entity);
		bullets.numOfPistolBullets = getProperty("numOfPistolBullets").toInt();
		bullets.numOfShotgunBullets = getProperty("numOfShotgunBullets").toInt();
		createDebugName();
	}
	else if(type == "Medkit") 
	{
		entity = templates.createCopy("Medkit", registry);
		loadPosition();
		createDebugName();
	}
	else if(type == "VelocityChangingArea")
	{
		entity = templates.createCopy("VelocityChangingArea", registry);
		loadPosition();
		loadSize();
		float& areaSpeedMultiplier = registry.get<component::AreaVelocityChangingEffect>(entity).areaSpeedMultiplier;
		areaSpeedMultiplier = getProperty("velocityMultiplier").toFloat();
		createDebugName();
	}
	else if(type == "PushingArea")
	{
		entity = templates.createCopy("PushingArea", registry);
		loadPosition();
		loadSize();
		auto& pushDirection = registry.get<component::PushingArea>(entity);
		pushDirection.pushForce.x = getProperty("pushForceX").toFloat();
		pushDirection.pushForce.y = getProperty("pushForceY").toFloat();
		createDebugName();
	}
	else if(type == "HintArea")
	{
		entity = templates.createCopy("HintArea", registry);
		loadPosition();
		loadSize();
		auto& hint = registry.get<component::Hint>(entity);
		hint.hintName = getProperty("hintName").toString();
		hint.keyboardContent = getProperty("hintKeyboardContent").toString();
		hint.joystickContent = getProperty("hintJoystickContent").toString();
		createDebugName();
	}
	else if(type == "Gate")
	{
		entity = templates.createCopy("Gate", registry);
		loadPosition();
		createDebugName();
	}
	else if(type == "Lever") 
	{
		entity = templates.createCopy("Lever", registry);
		loadPosition();
		createDebugName();
	}
	else if(type == "Sprite")
	{
		// create sprite entity
		entity = templates.createCopy("Sprite", registry);
		auto& [rq, body] = registry.get<component::RenderQuad, component::BodyRect>(entity);

		// load texture
		const std::string texturePath = getProperty("texturePath").toString();
		if(texturePath != "none") {
			if(loadTexture(texturePath))
				rq.texture = &getTexture(texturePath);
			else
				PH_EXIT_GAME("TiledParser::loadSprite() wasn't able to load texture \"" + texturePath + "\"");
		}

		// load texture rect
		if(getProperty("activeTextureRect").toBool()) {
			registry.assign_or_replace<component::TextureRect>(
				entity,
				IntRect(
					getProperty("textureRectLeft").toInt(),
					getProperty("textureRectTop").toInt(),
					getProperty("textureRectWidth").toInt(),
					getProperty("textureRectHeight").toInt()
				)
			);
		}

		// load hidden forrenderer
		if(getProperty("hiddenForRenderer").toBool())
			registry.assign_or_replace<component::HiddenForRenderer>(entity);

		// load shader
		rq.shader = nullptr;
		// TODO: Enable custom shaders
		/*const std::string shaderName = getProperty(spriteNode, "shaderName").toString();
		if(shaderName != "none") {
			const std::string vertexShaderFilepath = getProperty(spriteNode, "vertexShaderFilepath").toString();
			PH_ASSERT_CRITICAL(vertexShaderFilepath != "none", "TiledParser::loadSprite(): Sprite has 'shaderName' but doesn't have 'vertexShaderFilepath'!");
			const std::string fragmentShaderFilepath = getProperty(spriteNode, "vertexShaderFilepath").toString();
			PH_ASSERT_CRITICAL(fragmentShaderFilepath != "none", "TiledParser::loadSprite(): Sprite has 'shaderName' but doesn't have 'fragmentShaderFilepath'!");

			auto& sl = ShaderLibrary::getInstance();
			if(sl.loadFromFile(shaderName, vertexShaderFilepath.c_str(), fragmentShaderFilepath.c_str()))
				rq.shader = sl.get(shaderName);
			else
				PH_EXIT_GAME("EntitiesParser::parseRenderQuad() wasn't able to load shader!");
		}*/

		// load rotation and rotation origin
		rq.rotation = getProperty("rotation").toFloat();
		rq.rotationOrigin.x = getProperty("rotationOriginX").toFloat();
		rq.rotationOrigin.y = getProperty("rotationOriginY").toFloat();

		// load z
		rq.z = getProperty("z").toUnsignedChar();

		// TODO: Load color
		rq.color = sf::Color::White;

		// load body rect
		loadPositionAndSize();

		createDebugName();
	}
	else if(type == "Torch") 
	{
		entity = templates.createCopy("Torch", registry);
		loadPosition();
		createDebugName();
	}
	else if(type == "LightWall")
	{
		// TODO: Delete this one
		entity = templates.createCopy("LightWall", registry);
		loadPositionAndSize();
		createDebugName();
	}
	else if(type == "FlowingRiver")
	{
		entity = templates.createCopy("FlowingRiver", registry);
		auto& [pushingArea, particleEmitter, body] = registry.get<component::PushingArea, component::ParticleEmitter, component::BodyRect>(entity);
		body.rect.setPosition(getPositionAttribute());
		const sf::Vector2f size = getSizeAttribute();
		body.rect.setSize(size);
		const sf::Vector2f pushForce(getProperty("pushForceX").toFloat(), getProperty("pushForceY").toFloat());
		PH_ASSERT_CRITICAL(pushForce.x == 0.f || pushForce.y == 0.f, "We don't support diagonal flowing rivers! - Either pushForceX or pushForceY must be zero.");
		pushingArea.pushForce = pushForce;
		const float particleAmountMultiplier = getProperty("particleAmountMultiplier").toFloat();
		particleEmitter.amountOfParticles = static_cast<unsigned>(particleAmountMultiplier * size.x * size.y / 100.f);
		particleEmitter.parInitialVelocity = pushForce;
		particleEmitter.parInitialVelocityRandom = pushForce;
		if(pushForce.y > 0.f) {
			particleEmitter.spawnPositionOffset = {0.f, 0.f};
			particleEmitter.randomSpawnAreaSize = {size.x, 0.f};
		}
		else if(pushForce.y < 0.f) {
			particleEmitter.spawnPositionOffset = {0.f, size.y};
			particleEmitter.randomSpawnAreaSize = {size.x, 0.f};
		}
		else if(pushForce.x > 0.f) {
			particleEmitter.spawnPositionOffset = {0.f, 0.f};
			particleEmitter.randomSpawnAreaSize = {0.f, size.y};
		}
		else if(pushForce.x < 0.f) {
			particleEmitter.spawnPositionOffset = {size.x, 0.f};
			particleEmitter.randomSpawnAreaSize = {0.f, size.y};
		}
		particleEmitter.parWholeLifetime = pushForce.x == 0.f ? std::abs(size.y / pushForce.y) : std::abs(size.x / pushForce.x);
		createDebugName();
	}
	else if(type == "IndoorOutdoorBlendArea")
	{
		using component::IndoorOutdoorBlendArea;
		IndoorOutdoorBlendArea area;
		if(getProperty("exit_left").toBool())
		{
			area.exit = IndoorOutdoorBlendArea::Left;
		}
		else if(getProperty("exit_right").toBool())
		{
			area.exit = IndoorOutdoorBlendArea::Right;
		}
		else if(getProperty("exit_top").toBool())
		{
			area.exit = IndoorOutdoorBlendArea::Top;
		}
		else if(getProperty("exit_down").toBool())
		{
			area.exit = IndoorOutdoorBlendArea::Down;
		}

		entity = registry.create();
		registry.assign<component::IndoorOutdoorBlendArea>(entity, area);
		registry.assign<component::BodyRect>(entity);
		loadPositionAndSize();
		createDebugName();
	}
	else if(type == "PuzzleBoulder")
	{
		entity = templates.createCopy("PuzzleBoulder", registry);
		loadPosition();
		createDebugName();
	}
	else 
	{
		PH_LOG_ERROR("The type of object in map file (" + entityNode.getAttribute("type")->toString() + ") is unknown!");
	}
}

void loadEntitiesFromMapFile(const Xml& mapNode, EntitiesTemplateStorage& templates, entt::registry& registry,
							 SceneManager& sceneManager, bool* isPlayerOnScene)
{
	// TODO: Change map layer name in tiled to entities
	std::vector<Xml> objectGroupNodes = mapNode.getChildren("objectgroup");
	for(auto& objectGroupNode : objectGroupNodes)
	{
		if((objectGroupNode.getAttribute("name")->toString() == "gameObjects"))
		{
			std::vector<Xml> objects = objectGroupNode.getChildren("object");
			for(const auto& entityNode : objects)
			{
				loadEntity(entityNode, templates, registry, sceneManager, isPlayerOnScene);
			}
			return;
		}
	}
}

} 
