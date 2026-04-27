#pragma once

#include <QRectF>

class QPainter;

class Ball {
public:
    float x;
    float y;
    float vx;
    float vy;
    int size;

    Ball();

    void randomVelocity();
    void move();
    void bounceWalls();
    void reset();
    bool isOutLeft() const;
    bool isOutRight() const;
    float distToRect(const QRectF& rect) const;
    void draw(QPainter& painter) const;
};
