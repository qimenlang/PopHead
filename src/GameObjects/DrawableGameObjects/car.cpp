#include "car.hpp"

namespace ph {

Car::Car(Renderer& renderer, const float acceleration, const float slowingDown, const sf::Vector2f direction, sf::Texture& texture)
	:DrawableGameObject(renderer, "car", LayerID::kinematicEntities)
	,mAcceleration(acceleration)
	,mSlowingDown(slowingDown)
	,mDirection(direction)
	,mVelocity(0.f)
	,mShouldSpeedUp(false)
	,mShouldSlowDown(false)
{
	mSprite.setTexture(texture);
}

void Car::update(const sf::Time delta)
{
	if(mShouldSpeedUp)
		mVelocity += mAcceleration * delta.asSeconds();

	if(mVelocity > 0) {
		mVelocity -= 0.1f * delta.asSeconds();

		if(mShouldSlowDown)
			mVelocity -= mSlowingDown * delta.asSeconds();

		move(mVelocity * mDirection * delta.asSeconds());
	}
}

void Car::draw(sf::RenderTarget& renderTarget, const sf::RenderStates states) const
{
	renderTarget.draw(mSprite, states);
}

void Car::move(sf::Vector2f offset, bool recursive)
{
	DrawableGameObject::move(offset);
	mSprite.move(offset);
}

void Car::setPosition(sf::Vector2f position, bool recursive)
{
	DrawableGameObject::setPosition(position);
	mSprite.setPosition(position);
}

}
