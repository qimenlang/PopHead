#pragma once

#include "interface.hpp"
#include "Events/event.hpp"
#include <vector>

namespace ph {

class GUI
{
public:
	void handleEvent(const ph::Event&);
	void update(float dt);

	Interface* addInterface(const char* name);
	Interface* getInterface(const char* name);
	bool hasInterface(const char* name);
	void deleteInterface(const char* name);
	void showInterface(const char* name);
	void hideInterface(const char* name);
	void clearGUI();

private:
	std::vector<Interface> mInterfaces;
};

}
