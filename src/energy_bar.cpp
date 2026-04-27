#include "game/energy_bar.hpp"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPen>
#include <QString>

#include "game/constants.hpp"

namespace {
void drawTextTopLeft(QPainter& painter, int x, int y, const QString& text) {
    const QFontMetrics metrics(painter.font());
    painter.drawText(x, y + metrics.ascent(), text);
}
}

EnergyBar::EnergyBar(int idx, int x, int y)
    : energy(0), barX(x), barY(y), barW(200), barH(10), player(idx) {
    if (idx == 1) {
        fillColor = QColor(60, 140, 255);
        highlightColor = QColor(160, 210, 255);
    }
    else {
        fillColor = QColor(255, 60, 80);
        highlightColor = QColor(255, 160, 160);
    }
}

void EnergyBar::addEnergy(int value) {
    energy += value;
    if (energy > MAX_ENERGY) {
        energy = MAX_ENERGY;
    }
}

void EnergyBar::reset() {
    energy = 0;
}

bool EnergyBar::isFull() const {
    return energy >= MAX_ENERGY;
}

void EnergyBar::draw(QPainter& painter) const {
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(30, 30, 30));
    painter.drawRect(QRect(barX, barY, barW, barH));

    const int fillWidth = energy * barW / MAX_ENERGY;
    if (fillWidth > 0) {
        painter.setBrush(fillColor);
        painter.drawRect(QRect(barX, barY, fillWidth, barH));
        painter.setPen(QPen(highlightColor, 1));
        painter.drawLine(barX, barY, barX + fillWidth, barY);
    }

    QFont font("Consolas");
    font.setPixelSize(14);
    painter.setFont(font);
    painter.setPen(QColor(140, 170, 210));

    const QString label = QStringLiteral("P%1  %2 / %3").arg(player).arg(energy).arg(MAX_ENERGY);
    if (player == 1) {
        drawTextTopLeft(painter, barX, barY - 16, label);
    }
    else {
        const int textWidth = painter.fontMetrics().horizontalAdvance(label);
        drawTextTopLeft(painter, barX + barW - textWidth, barY - 16, label);
    }
    painter.restore();
}
