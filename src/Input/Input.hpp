#ifndef POPHEAD_INPUT_INPUT_H_
#define POPHEAD_INPUT_INPUT_H_

#include "ActionManager.hpp"
#include "KeyboardManager.hpp"
#include "MouseManager.hpp"

namespace PopHead {
namespace Input {


class Input
{
public:
    auto getKeyboard() -> const KeyboardManager&;
    auto getMouse()    -> const MouseManager&;
    auto getAction()   -> const MouseManager&;

private:
    ActionManager mAction;
    KeyboardManager mKeyboard;
    MouseManager mMouse;
};


}}

#endif // !POPHEAD_INPUT_INPUT_H_