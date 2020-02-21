#include "terminal.hpp"
#include "game.hpp"
#include "Logs/logs.hpp"
#include "Scenes/sceneManager.hpp"
#include "ECS/System.hpp"
#include "ECS/Components/charactersComponents.hpp"
#include "ECS/Components/physicsComponents.hpp"
#include "ECS/Components/graphicsComponents.hpp"
#include "ECS/Components/itemComponents.hpp"
#include "ECS/Systems/areasDebug.hpp"
#include "Audio/Music/musicPlayer.hpp"
#include "Audio/Sound/soundPlayer.hpp"
#include "Audio/Sound/soundData.hpp"
#include "Renderer/renderer.hpp"
#include "Renderer/MinorRenderers/lightRenderer.hpp"
#include "Renderer/API/font.hpp"
#include "GUI/xmlGuiParser.hpp"
#include "Utilities/cast.hpp"
#include "Utilities/xml.hpp"
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>
#include <entt/entt.hpp>
#include <array>
#include <deque>
#include <unordered_map>
#include <string>
#include <fstream>

namespace ph {

static std::string content;
static std::deque<std::string> lastCommands;
static bool isVisible = false;

static sf::Window* window;
static ph::SceneManager* sceneManager;

static int indexOfCurrentLastCommand = -1;
static std::deque<ph::OutputLine> outputLines;
static const sf::Vector2f vector2ArgumentError = {-1, -1};

typedef void (*ExecuteCommandFunction)(void);
static std::unordered_map<std::string, ExecuteCommandFunction> commandsMap;

static sf::Color errorRedColor = {255, 25, 33};
static sf::Color infoLimeColor = {127, 244, 44};

#ifndef PH_DISTRIBUTION
struct ResetGuiLive
{
	float timeFromReset = 0.f;
	float resetFrequency = 0.2f;
	bool isActive = false;
};
static ResetGuiLive resetGuiLive;
#endif 

static sf::Vector2f getPlayerPosition()
{
	sf::Vector2f playerPos = vector2ArgumentError;
	auto& registry = sceneManager->getScene().getRegistry();
	auto players = registry.view<component::Player, component::BodyRect>();
	players.each([&playerPos](const component::Player, const component::BodyRect body) {
		playerPos = body.rect.getCenter();
	});
	return playerPos;
}

static sf::Vector2f handleGetVector2ArgumentError()
{
	Terminal::pushOutputLine({"Incorrect argument! Argument has to be a number.", errorRedColor});
	return vector2ArgumentError;
}

static sf::Vector2f getVector2Argument()
{
	const std::string numbers("1234567890-");

	if(content.find_first_of(numbers) == std::string::npos)
		return handleGetVector2ArgumentError();

	const size_t xArgumentPositionInCommand = content.find_first_of(numbers);
	const size_t xArgumentEndPositionInCommand = content.find(' ', xArgumentPositionInCommand);
	const size_t xArgumentLength = xArgumentEndPositionInCommand - xArgumentPositionInCommand;
	std::string xArgument = content.substr(xArgumentPositionInCommand, xArgumentLength);
	const float positionX = std::strtof(xArgument.c_str(), nullptr);

	const size_t yArgumentPositionInCommand = content.find_first_of(numbers, xArgumentEndPositionInCommand + 1);
	if(yArgumentPositionInCommand == std::string::npos)
		return handleGetVector2ArgumentError();
	const size_t yArgumentEndPositionInCommand = content.find_first_not_of(numbers, yArgumentPositionInCommand);
	const size_t yArgumentLength = yArgumentEndPositionInCommand - yArgumentPositionInCommand;
	const std::string yArgument = content.substr(yArgumentPositionInCommand, yArgumentLength);
	const float positionY = std::strtof(yArgument.c_str(), nullptr);

	return sf::Vector2f(positionX, positionY);
}

static float getSingleFloatArgument()
{
	const size_t spacePosition = content.find_last_of(' ');
	const size_t valueStartPos = spacePosition + 1;
	const size_t valueLength = content.size() - valueStartPos;
	const std::string value = content.substr(valueStartPos, valueLength);
	return std::strtof(value.c_str(), nullptr);
}

static bool commandContains(const char c)
{
	return content.find(c) != std::string::npos;
}

static bool commandContains(const char* c)
{
	return content.find(c) != std::string::npos;
}

static void executeCommand()
{
	// find command without arguments
	size_t argumentPosition = content.find(' ');
	if(argumentPosition == std::string::npos)
		argumentPosition = content.size();
	const std::string commandWithoutArguments = content.substr(0, argumentPosition);

	// execute command
	auto found = commandsMap.find(commandWithoutArguments);
	if(found != commandsMap.end())
		(*found->second)();
	else
		Terminal::pushOutputLine({"Entered command wasn't recognised. Enter 'help' to see available commands.", sf::Color::Red});
}

static void executeInfoMessage()
{
	Terminal::pushOutputLine({"This is terminal. Enter 'help' to see available commands.", sf::Color(50, 50, 255)});
}

static void executeHistory()
{
	auto& commandsHistory = lastCommands;
	std::deque<std::string>::reverse_iterator it = commandsHistory.rbegin();

	for(; it != commandsHistory.rend(); ++it)
		Terminal::pushOutputLine({"- " + *it, infoLimeColor});
	Terminal::pushOutputLine({"Ten last used commands: ", sf::Color::White});
}

static void executeHelp()
{
	Terminal::pushOutputLine({});
	Terminal::pushOutputLine({"history @C9999 show last commands @CO @S31 currentpos @C9999 output player's position @CO @S32 view @C9999 change player's camera size", infoLimeColor});
	Terminal::pushOutputLine({"veld @C9999 velocity areas debug @CO @S31 pushd @C9999 push areas debug @CO @S32 cold @C9999 collision rects debug", infoLimeColor});
	Terminal::pushOutputLine({"give @C9999 player gets an item @CO @S31 tp @C9999 teleport @CO @S32 m @C9999 move player", infoLimeColor});
	Terminal::pushOutputLine({"setvolume @S31 mute @C9999 mute audio @CO @S32 unmute @C9999 unmute audio", infoLimeColor});
	Terminal::pushOutputLine({"gts @C9999 go to scene @CO @S31 r @C9999 reset scene @CO @S32 clear @C9999 clear terminal output", infoLimeColor});
	Terminal::pushOutputLine({"pause @C9999 pause game @CO @S31 rgui @C9999 reset gui @CO @S32 rguilive @C9999 reset gui all the time", infoLimeColor});
	Terminal::pushOutputLine({"rguilivefreq @C9999 set gui reset frequency @CO @S31 lwd @C9999 light walls debug @CO @S32 light @C9999 light debug", infoLimeColor});
	Terminal::pushOutputLine({"fontd @C9999 font debug @CO @S31 nofocusupdate @S32 dc @C9999 debug camera", infoLimeColor});
	Terminal::pushOutputLine({"@C9509 TO LEARN MORE DETAILS ABOUT THE COMMAND USE @CO? @C9509 For example: @COgts ?", infoLimeColor});
}

static void executeGotoScene()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C2919 Example: @C9609 gts sewage@CO goes to sewage scene of file sewage.xml"});
		Terminal::pushOutputLine({"@C9609 gts@CO takes one argument which is name of the scene file without extension."});
	}
	else
	{
		const int spacePosition = content.find_first_of(' ') + 1;
		const std::string scenePath = "scenes/" + content.substr(spacePosition, content.size()) + ".xml";
		sceneManager->replaceScene(scenePath);
	}
}

static void executeReset()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C9609 r stay@CO reloads the current scene and spawns player in his current position."});
		Terminal::pushOutputLine({"@C9609 r@CO reloads the current scene."});
	}
	else
	{
		if(commandContains("stay"))
		{
			sceneManager->replaceScene(sceneManager->getCurrentSceneFilePath(), getPlayerPosition());
		}
		else
		{
			sceneManager->replaceScene(sceneManager->getCurrentSceneFilePath());
		}
	}
}

static void executePause()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"More precisly it calls system::System::setPause(!commandContains(\"off\"))"});
		Terminal::pushOutputLine({"@C9609 pause off@CO Unpauses the game"});
		Terminal::pushOutputLine({"@C9609 pause@CO Pauses the game"});
	}
	else
	{
		system::System::setPause(!commandContains("off"));
	}
}

static void executeResetGui()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C9609 rgui@CO reloads current scene gui from file, doesn't take arguments"});
	}
	else
	{
		const int spacePosition = content.find_first_of(' ') + 1;
		Xml sceneFile;
		sceneFile.loadFromFile(sceneManager->getCurrentSceneFilePath());
		const auto sceneLinksNode = *sceneFile.getChild("scenelinks");
		if(const auto guiNode = sceneLinksNode.getChild("gui")) {
			const std::string filepath = "scenes/gui/" + guiNode->getAttribute("filename")->toString();
			XmlGuiParser guiParser;
			guiParser.parseGuiXml(filepath);
		}
	}
}

static void executeClear()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C9609clear@CO clears terminal output area, doesn't take arguments"});
	}
	else
	{
		for(int i = 0; i < 20; ++i)
			Terminal::pushOutputLine({""});
	}
}

static void executeTeleport()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C2919 Example: @C9609tp 100"});
		Terminal::pushOutputLine({"@C2919 Example: @C9609tp -100 2000"});
		Terminal::pushOutputLine({"It takes 1 parameter(a, a) or 2 parameters (x, y)"});
		Terminal::pushOutputLine({"If player is not on the scene it doesn't do anything"});
		Terminal::pushOutputLine({"@C9609tp@CO teleports player to absolute coordinate"});
	}
	else
	{
		const sf::Vector2f newPosition = getVector2Argument();
		if(newPosition == vector2ArgumentError)
			return;

		auto& registry = sceneManager->getScene().getRegistry();
		auto view = registry.view<component::Player, component::BodyRect>();
		view.each([newPosition](const component::Player player, component::BodyRect& body) {
			body.rect.left = newPosition.x;
			body.rect.top = newPosition.y;
		});
	}
}

static void executeMove()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C2919 Example: @C9609m 100"});
		Terminal::pushOutputLine({"@C2919 Example: @C9609m -100 2000"});
		Terminal::pushOutputLine({"It takes 1 parameter(a, a) or 2 parameters (x, y)"});
		Terminal::pushOutputLine({"If player is not on the scene it doesn't do anything"});
		Terminal::pushOutputLine({"@C9609m@CO teleports player to relative coordinate"});
	}
	else
	{
		sf::Vector2f moveOffset = getVector2Argument();
		auto& registry = sceneManager->getScene().getRegistry();
		auto view = registry.view<component::Player, component::BodyRect>();
		view.each([moveOffset](const component::Player, component::BodyRect& body) {
			body.rect.left += moveOffset.x;
			body.rect.top += moveOffset.y;
		});
	}
}

static void executeGive()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C2919 Example: @C9609give bullet 100"});
		Terminal::pushOutputLine({"It takes 2 parameters (number of items, item name)"});
		Terminal::pushOutputLine({"@C9609give@CO puts given number of given item in player's inventory"});
	}
	else
	{
		if(commandContains("bullet"))
		{
			int numberOfItems = static_cast<int>(getSingleFloatArgument());
			auto& registry = sceneManager->getScene().getRegistry();
			auto view = registry.view<component::Player, component::Bullets>();
			view.each([numberOfItems](const component::Player, component::Bullets& bullets) {
				bullets.numOfPistolBullets += numberOfItems;
				bullets.numOfShotgunBullets += numberOfItems;
			});
		}
		else
		{
			Terminal::pushOutputLine({"Type of item is unknown!", errorRedColor});
		}
	}
}

static void executeCurrentPos()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C9609currentpos@CO Outputs player's position to terminal"});
	}
	else
	{
		Terminal::pushOutputLine({"player position: " + Cast::toString(getPlayerPosition()), infoLimeColor});
	}
}

static void executeCollisionDebug()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C9609cold off@CO turns off collision rects debug"});
		Terminal::pushOutputLine({"@C9609cold@CO turns on collision rects debug"});
	}
	else
	{
		system::AreasDebug::setIsCollisionDebugActive(!commandContains("off"));
	}
}

static void executeVelocityChangingAreaDebug()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C9609veld off@CO turns off velocity changing areas debug"});
		Terminal::pushOutputLine({"@C9609veld@CO turns on velocity changing areas debug"});
	}
	else
	{
		system::AreasDebug::setIsVelocityChangingAreaDebugActive(!commandContains("off"));
	}
}

static void executePushingAreaDebug()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C9609pushd off@CO turns off pushing areas debug"});
		Terminal::pushOutputLine({"@C9609pushd@CO turns on pushing areas debug"});
	}
	else
	{
		system::AreasDebug::setIsPushingAreaDebugActive(!commandContains("off"));
	}
}

static void executeLightWallsAreaDebug()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({""});
		Terminal::pushOutputLine({"@C9609lwd off@CO turns off light walls debug"});
		Terminal::pushOutputLine({"@C9609lwd@CO turns on light walls debug"});
	}
	else
	{
		system::AreasDebug::setIsLightWallsAreaDebugActive(!commandContains("off"));
	}
}

static void setAudioMuted(bool mute)
{
	if(commandContains("music")) {
		MusicPlayer::setMuted(mute);
	}
	else if(commandContains("sound")) {
		SoundPlayer::setMuted(mute);
	}
	else {
		MusicPlayer::setMuted(mute);
		SoundPlayer::setMuted(mute);
	}
}

static void executeMute()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({});
		Terminal::pushOutputLine({"@C9609mute sound @CO mutes only sound"});
		Terminal::pushOutputLine({"@C9609mute music @CO mutes only music"});
		Terminal::pushOutputLine({"@C9609mute @CO mutes audio"});
	}
	else
	{
		setAudioMuted(true);
	}
}

static void executeUnmute()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({});
		Terminal::pushOutputLine({"@C9609unmute sound @CO unmutes only sound"});
		Terminal::pushOutputLine({"@C9609unmute music @CO unmutes only music"});
		Terminal::pushOutputLine({"@C9609unmute @CO unmutes audio"});
	}
	else
	{
		setAudioMuted(false);
	}
}

static void executeSetVolume()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({});
		Terminal::pushOutputLine({"@C9609 setvolume sound @CO sets sound volume"});
		Terminal::pushOutputLine({"@C9609 setvolume music @CO sets music volume"});
		Terminal::pushOutputLine({"@C9609 setvolume @CO sets audio volume"});
	}
	else
	{
		const float newVolume = getSingleFloatArgument();
		if(!(commandContains('0')) && newVolume == 0 || newVolume > 100) {
			Terminal::pushOutputLine({"Incorrect volume value! Enter value from 0 to 100", sf::Color::Red});
			return;
		}

		if(commandContains("music")) {
			MusicPlayer::setVolume(newVolume);
		}
		else if(commandContains("sound")) {
			SoundPlayer::setVolume(newVolume);
		}
		else {
			MusicPlayer::setVolume(newVolume);
			SoundPlayer::setVolume(newVolume);
		}
	}
}

static void executeLight()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({});
		Terminal::pushOutputLine({"@C9609 light rays off @CO disables rays debug"});
		Terminal::pushOutputLine({"@C9609 light rays @CO enables rays debug"});
		Terminal::pushOutputLine({"@C9609 light off @CO disables lighting"});
		Terminal::pushOutputLine({"@C9609 light @CO enables lighting"});
	}
	else
	{
		bool on = !commandContains("off");

		auto& lightDebug = LightRenderer::getDebug();

		if(commandContains("rays"))
			lightDebug.drawRays = on;
		else
			lightDebug.drawLight = on;
	}
}

static void executeFontDebug()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({});
		Terminal::pushOutputLine({"@C9609 fontd off @CO disables font debug"});
		Terminal::pushOutputLine({"@C9609 fontd @CO enables font debug"});
	}
	else
	{
		if(commandContains("off") && FontDebugRenderer::isActive()) {
			FontDebugRenderer::shutDown();
		}
		else if(!FontDebugRenderer::isActive()) {
			FontDebugRenderer::init("joystixMonospace.ttf", 50);
		}
	}
}

static void executeNoFocusUpdate()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({});
		Terminal::pushOutputLine({"@C9609 nofocusupdate off@CO disables updating game if game's window doesn't have focus"});
		Terminal::pushOutputLine({"@C9609 nofocusupdate@CO enables updating game if game's window doesn't have focus"});
	}
	else
	{
		Game::setNoFocusUpdate(!commandContains("off"));
	}
}

static void executeDebugCamera()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({});
		Terminal::pushOutputLine({"@C9609 dc off@CO disables debug camera"});
		Terminal::pushOutputLine({"@C9609 dc@CO enables debug camera"});
	}
	else
	{
		auto& registry = sceneManager->getScene().getRegistry();

		auto destroyExistingDebugCameras = [&registry]() {
			auto debugCameras = registry.view<component::DebugCamera, component::Camera, component::BodyRect>();
			registry.destroy(debugCameras.begin(), debugCameras.end());
		};

		component::Camera::currentCameraName = "default";
		destroyExistingDebugCameras();
		isVisible = true;

		if(!commandContains("off"))
		{
			// create debug camera
			sf::Vector2f playerPos = getPlayerPosition();
			auto entity = registry.create();
			registry.assign<component::Camera>(entity, Camera(playerPos, {640, 360}), "debug");
			registry.assign<component::DebugCamera>(entity);
			registry.assign<component::BodyRect>(entity, FloatRect(playerPos, {0.f, 0.f}));
			component::Camera::currentCameraName = "debug";

			isVisible = false;
		}
	}
}

#ifndef PH_DISTRIBUTION

static void executeResetGuiLive()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({});
		Terminal::pushOutputLine({"you can change rguilivefreq with @C9609 rguilivefreq @CO command"});
		Terminal::pushOutputLine({"@C9609 rguilive off@CO disables loading gui from file once for rguilivefreq seconds"});
		Terminal::pushOutputLine({"@C9609 rguilive@CO enables loading gui from file once for rguilivefreq seconds"});
	}
	else
	{
		resetGuiLive.isActive = !commandContains("off");
		Game::setNoFocusUpdate(resetGuiLive.isActive);
	}
}

static void executeResetGuiLiveFrequency()
{
	if(commandContains('?'))
	{
		Terminal::pushOutputLine({});
		Terminal::pushOutputLine({"@C2919 Example: @C9609 rguilivefreq 0.5@CO sets rguilive freq to 0.5 seconds"});
		Terminal::pushOutputLine({"@C9609 rguilivefreq@CO takes one floating point argument and sets rguilivefreq"});
	}
	else
	{
		resetGuiLive.resetFrequency = getSingleFloatArgument();
	}
}

#endif 

static void updateCommands(float dt)
{
#ifndef PH_DISTRIBUTION
	if(resetGuiLive.isActive) {
		resetGuiLive.timeFromReset += dt;
		if(resetGuiLive.timeFromReset > resetGuiLive.resetFrequency) {
			executeResetGui();
			resetGuiLive.timeFromReset = 0.f;
		}
	}
#endif
}

namespace Terminal {

void handleEvent(sf::Event e)
{
	if(isVisible && e.type == sf::Event::TextEntered)
	{
		char key = static_cast<char>(e.text.unicode);
		if(!iscntrl(key))
			content += key;
	}

	if(e.type == sf::Event::KeyPressed)
	{
		if(e.key.code == sf::Keyboard::Tab && e.key.control)
		{	
			isVisible = !isVisible;
			window->setKeyRepeatEnabled(isVisible);
			system::System::setPause(isVisible);
		}

		if(!isVisible)
			return;

		switch(e.key.code)
		{
			case sf::Keyboard::BackSpace: {
				if(content.size() > 0)
					content.pop_back();
			} break;

			case sf::Keyboard::Enter: {
				executeCommand();

				indexOfCurrentLastCommand = -1;
				if(content.size() != 0) {
					lastCommands.emplace_front(content);
					if(lastCommands.size() > 10)
						lastCommands.pop_back();
				}

				content.clear();
			} break;

			case sf::Keyboard::Up: {
				if(indexOfCurrentLastCommand + 1 < static_cast<int>(lastCommands.size()))
				{
					++indexOfCurrentLastCommand;
					if(indexOfCurrentLastCommand >= 0)
						content = lastCommands[indexOfCurrentLastCommand];
				}
			} break;

			case sf::Keyboard::Down: {
				if(indexOfCurrentLastCommand > -1)
				{
					--indexOfCurrentLastCommand;
					if(indexOfCurrentLastCommand == -1)
						content.clear();
					else
						content = lastCommands[indexOfCurrentLastCommand];
				}
			} break;
		}
	}
}

void pushOutputLine(const OutputLine& line)
{
	if(outputLines.size() >= 14)
		outputLines.pop_back();
	outputLines.emplace_front(line);
}

void init(sf::Window* w, SceneManager* sm)
{
	window = w;
	sceneManager = sm;

	commandsMap["tp"] = &executeTeleport;
	commandsMap["give"] = &executeGive;
	commandsMap["currentpos"] = &executeCurrentPos;
	commandsMap["cold"] = &executeCollisionDebug;
	commandsMap["veld"] = &executeVelocityChangingAreaDebug;
	commandsMap["pushd"] = &executePushingAreaDebug;
	commandsMap["lwd"] = &executeLightWallsAreaDebug;
	commandsMap["mute"] = &executeMute;
	commandsMap["unmute"] = &executeUnmute;
	commandsMap["setvolume"] = &executeSetVolume;
	commandsMap["history"] = &executeHistory;
	commandsMap["help"] = &executeHelp;
	commandsMap["clear"] = &executeClear;
	commandsMap["gts"] = &executeGotoScene;
	commandsMap["r"] = &executeReset;
	commandsMap["pause"] = &executePause;
	commandsMap["rgui"] = &executeResetGui;
	commandsMap["light"] = &executeLight;
	commandsMap["m"] = &executeMove;
	commandsMap["fontd"] = &executeFontDebug;
	commandsMap["nofocusupdate"] = &executeNoFocusUpdate;
	commandsMap["dc"] = &executeDebugCamera;
	commandsMap[""] = &executeInfoMessage;

#ifndef PH_DISTRIBUTION
	commandsMap["rguilive"] = &executeResetGuiLive;
	commandsMap["rguilivefreq"] = &executeResetGuiLiveFrequency;
#endif

	// read terminalInit.txt file
	std::ifstream file;
	file.open("terminalInit.txt");
	if(file.is_open())
	{
		while(!file.eof())
		{
			getline(file, content);
			executeCommand();
		}
		file.close();
	}
}

void update(float dt)
{
	updateCommands(dt);

	if(isVisible)
	{
		Renderer::submitQuad(nullptr, nullptr, &sf::Color(0, 0, 0, 230), nullptr, {0.f, 660.f}, {1920.f, 420.f}, 5, 0.f, {},
			ProjectionType::gui, false);

		Renderer::submitQuad(nullptr, nullptr, &sf::Color::Black, nullptr, {0.f, 720.f}, {1920.f, 5.f},
			4, 0.f, {}, ProjectionType::gui, false);

		Renderer::submitText(content.c_str(), "LiberationMono-Bold.ttf", {5.f, 660.f},
			50.f, sf::Color::White, 0, ProjectionType::gui, false);

		float posY = 723.f;
		for(size_t i = 0; i < outputLines.size(); ++i, posY += 25.f)
			Renderer::submitTextArea(outputLines[i].text.c_str(), "LiberationMono.ttf", {5.f, posY}, 1920.f, TextAligment::left,
			                         25.f, outputLines[i].color, 0, ProjectionType::gui, false);
	}
}

}}
