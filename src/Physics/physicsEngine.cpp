#include "physicsEngine.hpp"
#include "Utilities/math.hpp"
#include "Logs/logs.hpp"
#include <memory>

namespace ph {

PhysicsEngine::PhysicsEngine()
	: mStaticBodies(std::function<bool(const std::unique_ptr<CollisionBody>&, const std::unique_ptr<CollisionBody>&)>(
		[](const std::unique_ptr<CollisionBody>& b1, const std::unique_ptr<CollisionBody>& b2) -> bool {
			auto pos1 = b1->getPosition();
			auto pos2 = b2->getPosition();

			if (pos1.x < pos2.x)
				return true;
			else if (pos1.x == pos2.x && pos1.y < pos2.y)
				return true;
			return false;
		}
		))
{
	//mStaticBodiesOld.reserve(300);
}

const CollisionBody& PhysicsEngine::createStaticBodyAndGetTheReference(const sf::FloatRect rect)
{
	auto iter = mStaticBodies.emplace(std::make_unique<CollisionBody>(rect, 0.f));
	return *iter.first->get();

	/*mStaticBodiesOld.emplace_back(std::make_unique<CollisionBody>(rect, 0.f));
	auto& staticBody = *mStaticBodiesOld.back().get();
	mCollisionDebugManager.addStaticBodyCollisionDebugRect(staticBody);
	return staticBody;*/
}

CollisionBody& PhysicsEngine::createKinematicBodyAndGetTheReference(const sf::FloatRect rect, const float mass)
{
	mKinematicBodies.emplace_back(rect, mass);
	auto& kinematicBody = mKinematicBodies.back();
	mCollisionDebugManager.addKinematicBodyCollisionDebugRect(kinematicBody);
	return kinematicBody;
}

void PhysicsEngine::removeStaticBody(const CollisionBody& bodyToDelete)
{
	mStaticBodies.erase(std::make_unique<CollisionBody>(bodyToDelete));

	/*for(auto it = mStaticBodiesOld.begin(); it != mStaticBodiesOld.end(); ++it)
		if(it->get() == std::addressof(bodyToDelete)) {
			mStaticBodiesOld.erase(it);
			break;
		}*/
}

void PhysicsEngine::removeKinematicBody(const CollisionBody& bodyToDelete)
{
	for(auto it = mKinematicBodies.begin(); it != mKinematicBodies.end(); ++it)
		if(std::addressof(*it) == std::addressof(bodyToDelete)) {
			mKinematicBodies.erase(it);
			break;
		}
}

void PhysicsEngine::clear() noexcept
{
	//mStaticBodiesOld.clear();
	mStaticBodies.clear();
	mKinematicBodies.clear();
	mCollisionDebugManager.clear();
}

void PhysicsEngine::update(sf::Time delta)
{
    for(auto &kinematicBody : mKinematicBodies)
    {
		handleKinematicCollisionsFor(kinematicBody);
		kinematicBody.updatePush(delta);
		handleStaticCollisionsFor(kinematicBody);
		kinematicBody.actionsAtTheEndOfPhysicsLoopIteration();
    }
	updatePositionsOfDebugRects();
}

void PhysicsEngine::handleStaticCollisionsFor(CollisionBody& kinematicBody)
{
	/*for (auto& staticBody : mStaticBodiesOld)
		if (isThereCollision(kinematicBody, *staticBody))
			mStaticCollisionHandler(kinematicBody, *staticBody);*/

	auto rect = kinematicBody.getRect();
	rect.left -= 32.f;
	rect.top -= 32.f;

	auto iter = mStaticBodies.lower_bound(std::make_unique<CollisionBody>(rect, 0.f));
	auto end = mStaticBodies.end();

	auto maxPosition = kinematicBody.getPosition().x + 32.f;

	while (iter != end)
	{
		if (iter->get()->getPosition().x > maxPosition)
			break;

		if (isThereCollision(kinematicBody, *iter->get()))
			mStaticCollisionHandler(kinematicBody, *iter->get());

		++iter;
	}
}

void PhysicsEngine::handleKinematicCollisionsFor(CollisionBody& kinematicBody)
{
    for (auto& kinematicBody2 : mKinematicBodies)
    {
		if (std::addressof(kinematicBody) == std::addressof(kinematicBody2))
			continue;

		if (isThereCollision(kinematicBody, kinematicBody2))
            mKinematicCollisionHandler(kinematicBody, kinematicBody2);
    }
}

bool PhysicsEngine::isThereCollision(const CollisionBody& a, const CollisionBody& b) const
{
	return Math::areTheyOverlapping(a.getRect(), b.getRect());
}

void PhysicsEngine::updatePositionsOfDebugRects()
{
	int i = 0;
	for(const auto& kinematicBody : mKinematicBodies) {
		auto pos = kinematicBody.getPosition();
		mCollisionDebugManager.mKinematicBodyCollisionDebugRects[i].setPosition(pos);
		++i;
	}
}

}