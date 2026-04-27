#pragma once

#include <QColor>

class QPainter;

class EnergyBar {
public:
    int energy;
    int barX;
    int barY;
    int barW;
    int barH;
    int player;
    QColor fillColor;
    QColor highlightColor;

    EnergyBar(int idx, int x, int y);

    void addEnergy(int value);
    void reset();
    bool isFull() const;
    void draw(QPainter& painter) const;
};
