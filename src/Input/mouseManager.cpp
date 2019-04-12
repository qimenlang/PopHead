#include "mouseManager.hpp"
#include "Base/gameData.hpp"
#include "eventLoop.hpp"

using PopHead::Input::MouseManager;


auto MouseManager::getMousePosition() const -> sf::Vector2i
{
    return sf::Mouse::getPosition( mGameData->getRenderer().getWindow() );
}

bool MouseManager::hasMouseJustMoved() const
{
    return EventLoop::hasMouseJustMoved();
}

bool MouseManager::isMouseButtonJustPressed(sf::Mouse::Button button) const
{
    return (EventLoop::isMouseButtonJustPressed() && EventLoop::getMouseButton() == button);
}

bool MouseManager::isMouseButtonJustReleased(sf::Mouse::Button button) const
{
    return (EventLoop::isMouseButtonJustReleased() && EventLoop::getMouseButton() == button);
}
