#include "game/ball.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include <QColor>
#include <QPainter>
#include <QPointF>

#include "game/constants.hpp"

Ball::Ball()
    : x(static_cast<float>(WIDTH / 2)),
      y(static_cast<float>(HEIGHT / 2)),
      vx(0.0f),
      vy(0.0f),
      size(6) {
    randomVelocity();
}

void Ball::randomVelocity() {
    const int spx = BALL_SPAWN_SPEED_MIN + std::rand() % BALL_SPAWN_SPEED_RANGE;
    const int spy = BALL_SPAWN_SPEED_MIN + std::rand() % BALL_SPAWN_SPEED_RANGE;
    vx = static_cast<float>(std::rand() % 2 == 0 ? spx : -spx);
    vy = static_cast<float>(std::rand() % 2 == 0 ? spy : -spy);
}

void Ball::move() {
    x += vx;
    y += vy;
}

void Ball::bounceWalls() {
    if (y <= size) {
        vy = std::fabs(vy);
    }
    if (y >= HEIGHT - size) {
        vy = -std::fabs(vy);
    }
}

void Ball::reset() {
    x = static_cast<float>(WIDTH / 2);
    y = static_cast<float>(HEIGHT / 2);
    size = 6;
    randomVelocity();
}

bool Ball::isOutLeft() const {
    return x < 0.0f;
}

bool Ball::isOutRight() const {
    return x > static_cast<float>(WIDTH);
}

float Ball::distToRect(const QRectF& rect) const {
    const float nx = std::clamp(x, static_cast<float>(rect.left()), static_cast<float>(rect.right()));
    const float ny = std::clamp(y, static_cast<float>(rect.top()), static_cast<float>(rect.bottom()));
    const float dx = x - nx;
    const float dy = y - ny;
    return std::sqrt(dx * dx + dy * dy);
}

void Ball::draw(QPainter& painter) const {
    painter.save();
    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(30, 60, 120));
    painter.drawEllipse(QPointF(x, y), size * 2.5, size * 2.5);

    painter.setBrush(QColor(80, 140, 255));
    painter.drawEllipse(QPointF(x, y), size * 1.6, size * 1.6);

    painter.setBrush(QColor(160, 210, 255));
    painter.drawEllipse(QPointF(x, y), size, size);

    const int coreRadius = std::max(1, static_cast<int>(size * 0.4f));
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPointF(x, y), coreRadius, coreRadius);
    painter.restore();
}
