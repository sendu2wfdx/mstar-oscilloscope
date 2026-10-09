// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "WaveformWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QFont>
#include <algorithm>
#include <cmath>

const QColor WaveformWidget::s_channelColors[] = {
    QColor(0, 255, 0),     // green
    QColor(255, 255, 0),   // yellow
    QColor(0, 200, 255),   // cyan
    QColor(255, 128, 0),   // orange
    QColor(255, 0, 255),   // magenta
    QColor(255, 100, 100), // light red
    QColor(128, 255, 128), // light green
    QColor(200, 200, 255), // light blue
};

const int WaveformWidget::s_maxChannels = sizeof(WaveformWidget::s_channelColors) / sizeof(QColor);

WaveformWidget::WaveformWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(20, 20, 20));
    setPalette(pal);
}

void WaveformWidget::setChannelCount(int count)
{
    m_channels.resize(count);
    for (int i = 0; i < count; ++i) {
        m_channels[i].color = s_channelColors[i % s_maxChannels];
        m_channels[i].visible = true;
    }
    update();
}

void WaveformWidget::setChannelData(int channel, const QVector<float> &data)
{
    if (channel < 0 || channel >= m_channels.size()) return;
    m_channels[channel].data = data;

    if (m_autoRange) {
        float gmin = 1e9f, gmax = -1e9f;
        for (const auto &ch : m_channels) {
            if (!ch.visible || ch.data.isEmpty()) continue;
            auto [lo, hi] = std::minmax_element(ch.data.begin(), ch.data.end());
            gmin = std::min(gmin, *lo);
            gmax = std::max(gmax, *hi);
        }
        if (gmin < gmax) {
            float margin = (gmax - gmin) * 0.1f;
            if (margin < 0.01f) margin = 0.5f;
            m_vmin = gmin - margin;
            m_vmax = gmax + margin;
        }
    }
    update();
}

QVector<float> WaveformWidget::channelData(int channel) const
{
    if (channel < 0 || channel >= m_channels.size()) return {};
    return m_channels[channel].data;
}

void WaveformWidget::clearData()
{
    for (auto &ch : m_channels) {
        ch.data.clear();
    }
    update();
}

void WaveformWidget::setVoltageRange(float vmin, float vmax)
{
    m_vmin = vmin;
    m_vmax = vmax;
    m_autoRange = false;
    update();
}

void WaveformWidget::setDisplayPoints(qsizetype points)
{
    m_displayPoints = points;
    update();
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int margin = 60;
    QRect area(margin, 10, width() - margin - 20, height() - margin - 20);

    painter.fillRect(area, QColor(30, 30, 30));

    drawGrid(painter, area);
    drawAxisLabels(painter, area);

    for (const auto &ch : m_channels) {
        if (ch.visible && !ch.data.isEmpty()) {
            drawWaveform(painter, area, ch.data, ch.color);
        }
    }

    painter.setPen(QColor(80, 80, 80));
    painter.drawRect(area);
}

void WaveformWidget::drawGrid(QPainter &painter, const QRect &area)
{
    QPen pen(QColor(60, 60, 60));
    painter.setPen(pen);

    const int hDivs = 10;
    const int vDivs = 8;

    for (int i = 1; i < hDivs; ++i) {
        int x = area.left() + i * area.width() / hDivs;
        painter.drawLine(x, area.top(), x, area.bottom());
    }
    for (int i = 1; i < vDivs; ++i) {
        int y = area.top() + i * area.height() / vDivs;
        painter.drawLine(area.left(), y, area.right(), y);
    }
}

void WaveformWidget::drawWaveform(QPainter &painter, const QRect &area,
                                   const QVector<float> &data, const QColor &color)
{
    if (data.size() < 2) return;

    QPen pen(color, 1.5);
    painter.setPen(pen);

    qsizetype points = std::min(data.size(), m_displayPoints);
    float vrange = m_vmax - m_vmin;
    if (vrange < 1e-9f) vrange = 1.0f;

    QPainterPath path;
    float x0 = area.left();
    float y0 = area.bottom() - (data[0] - m_vmin) / vrange * area.height();
    path.moveTo(x0, std::clamp(y0, (float)area.top(), (float)area.bottom()));

    for (int i = 1; i < points; ++i) {
        float x = area.left() + (float)i / (points - 1) * area.width();
        float y = area.bottom() - (data[i] - m_vmin) / vrange * area.height();
        y = std::clamp(y, (float)area.top(), (float)area.bottom());
        path.lineTo(x, y);
    }

    painter.drawPath(path);
}

void WaveformWidget::drawAxisLabels(QPainter &painter, const QRect &area)
{
    painter.setPen(QColor(180, 180, 180));
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    float vrange = m_vmax - m_vmin;
    const int vLabels = 5;
    for (int i = 0; i <= vLabels; ++i) {
        float v = m_vmin + i * vrange / vLabels;
        int y = area.bottom() - i * area.height() / vLabels;
        QString label = QString::number(v, 'f', 2) + "V";
        painter.drawText(0, y - 8, 55, 16, Qt::AlignRight | Qt::AlignVCenter, label);
    }

    qsizetype points = 0;
    for (const auto &ch : m_channels) {
        points = std::max(points, ch.data.size());
    }
    QString xlabel = QString("Points: %1").arg(std::min(points, m_displayPoints));
    painter.drawText(area.left(), area.bottom() + 5, area.width(), 16,
                     Qt::AlignHCenter, xlabel);
}
