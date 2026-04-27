#pragma once

#include <QRectF>

class QPainter;

class Paddle {
public:
    int x;
    int y;
    int width;
    int height;

    explicit Paddle(int leftX);

    void moveUp();
    void moveDown();
    void moveToward(float targetY, float speed);
    QRectF rect() const;
    void draw(QPainter& painter) const;
};
