#include "game/paddle.hpp"

#include <cmath>

#include <QColor>
#include <QPainter>

#include "game/constants.hpp"

Paddle::Paddle(int leftX) : x(leftX), y(HEIGHT / 2 - PADDLE_H / 2), width(PADDLE_W), height(PADDLE_H) {
}

void Paddle::moveUp() {
    y -= PADDLE_SPEED;
    if (y < 0) {
        y = 0;
    }
}

void Paddle::moveDown() {
    y += PADDLE_SPEED;
    if (y + height > HEIGHT) {
        y = HEIGHT - height;
    }
}

void Paddle::moveToward(float targetY, float speed) {
    const float center = y + height / 2.0f;
    const float diff = targetY - center;

    if (std::fabs(diff) < speed) {
        y = static_cast<int>(targetY - height / 2.0f);
    }
    else if (diff > 0.0f) {
        y += static_cast<int>(speed);
    }
    else {
        y -= static_cast<int>(speed);
    }

    if (y < 0) {
        y = 0;
    }
    if (y + height > HEIGHT) {
        y = HEIGHT - height;
    }
}

QRectF Paddle::rect() const {
    return QRectF(x, y, width, height);
}

void Paddle::draw(QPainter& painter) const {
    painter.save();
    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(40, 40, 60));
    painter.drawRect(QRectF(x - 8, y - 8, width + 16, height + 16));

    painter.setBrush(QColor(100, 100, 140));
    painter.drawRect(QRectF(x - 4, y - 4, width + 8, height + 8));

    painter.setBrush(QColor(180, 180, 220));
    painter.drawRect(QRectF(x - 2, y - 2, width + 4, height + 4));

    painter.setBrush(Qt::white);
    painter.drawRect(QRectF(x, y, width, height));
    painter.restore();
}
