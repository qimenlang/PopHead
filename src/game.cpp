#include "game.hpp"

#include <SFML/System.hpp>
#include <Input/eventLoop.hpp>
#include "Logs/logger.hpp"
#include "Resources/loadFonts.hpp"

namespace ph {

Game::Game()
	:mGameData{}
	,mSoundPlayer{new SoundPlayer()}
	,mMusicPlayer{new MusicPlayer()}
	,mTextures{new TextureHolder()}
	,mFonts{new FontHolder()}
	,mShaders{new ShaderHolder()}
	,mSceneMachine{new SceneMachine()}
	,mMap(new Map())
	,mInput{new Input()}
	,mRenderer{new Renderer()}
	,mPhysicsEngine{new PhysicsEngine()}
	,mTerminal{new Terminal()}
	,mEfficiencyRegister{new EfficiencyRegister()}
	,mGui{new GUI()}
{
	mGameData.reset(new GameData(
		mSoundPlayer.get(),
		mMusicPlayer.get(),
		mTextures.get(),
		mFonts.get(),
		mShaders.get(),
		mSceneMachine.get(),
		mMap.get(),
		mInput.get(),
		mRenderer.get(),
		mPhysicsEngine.get(),
		mTerminal.get(),
		mEfficiencyRegister.get(),
		mGui.get()
	));

	loadFonts(mGameData.get());

	mTerminal->init(mGameData.get());

	//logger.setGameData() has to be called after mTerminal.init() - this comment should be replaced by proper unit test
	Logger::getInstance().setGameData(mGameData.get());

	mEfficiencyRegister->init(mGameData.get());

	mMap->setGameData(mGameData.get());

	mSceneMachine->setGameData(mGameData.get());
	mSceneMachine->pushScene("scenes/desertScene.xml");

	mGui->init(mGameData.get());

	EventLoop::init(mGameData.get());
	mInput->setGameData(mGameData.get());

	mRenderer->setGameData(mGameData.get());
}

void Game::run()
{
	sf::Clock clock;
	const sf::Time timePerFrame = sf::seconds(1.f / 60.f);
	sf::Time timeSinceLastUpdate = sf::Time::Zero;

	while(mGameData->getGameCloser().shouldGameBeClosed() == false)
	{
		mSceneMachine->changingScenesProcess();
		input();

		timeSinceLastUpdate += clock.restart();

		while(timeSinceLastUpdate >= timePerFrame) {
			timeSinceLastUpdate -= timePerFrame;

			update(timePerFrame);
			mRenderer->draw();
		}
	}

	mRenderer->getWindow().close();
}

void Game::input()
{
	EventLoop::eventLoop(mGameData.get());
	mSceneMachine->input();
	mInput->getGlobalKeyboardShortcutes().handleShortcuts();
	mTerminal->input();
	mEfficiencyRegister->input();
}

void Game::update(sf::Time delta)
{
	mSceneMachine->update(delta);
	mPhysicsEngine->update(delta);
	mRenderer->update(delta);
	mGui->update(delta);
	mEfficiencyRegister->update();
}

}