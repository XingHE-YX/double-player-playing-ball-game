#pragma once

#include <QSet>

class InputManager {
public:
    void pressKey(int key);
    void releaseKey(int key);
    bool isKeyDown(int key) const;
    bool isKeyPressed(int key) const;
    void endFrame();
    void clear();

private:
    QSet<int> downKeys;
    QSet<int> pressedKeys;
};
