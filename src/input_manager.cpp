#include "game/input_manager.hpp"

void InputManager::pressKey(int key) {
    if (!downKeys.contains(key)) {
        pressedKeys.insert(key);
    }

    downKeys.insert(key);
}

void InputManager::releaseKey(int key) {
    downKeys.remove(key);
}

bool InputManager::isKeyDown(int key) const {
    return downKeys.contains(key);
}

bool InputManager::isKeyPressed(int key) const {
    return pressedKeys.contains(key);
}

void InputManager::endFrame() {
    pressedKeys.clear();
}

void InputManager::clear() {
    downKeys.clear();
    pressedKeys.clear();
}
