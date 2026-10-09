// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "WindowIndicator.h"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <algorithm>

WindowIndicator::WindowIndicator(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(40);
    setMinimumWidth(100);
}

void WindowIndicator::setFullRange(double min, double max)
{
    m_fullMin = min;
    m_fullMax = max;
    update();
}

void WindowIndicator::setVisibleRange(double min, double max)
{
    m_visMin = min;
    m_visMax = max;
    update();
}

void WindowIndicator::setWaveformData(const QVector<double> &xData, const QVector<double> &yData)
{
    m_xData = xData;
    m_yData = yData;
    m_hasData = !xData.isEmpty();
    update();
}

void WindowIndicator::clear()
{
    m_xData.clear();
    m_yData.clear();
    m_hasData = false;
    update();
}

void WindowIndicator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // 背景
    painter.fillRect(rect(), QColor(40, 40, 40));

    // 边框
    painter.setPen(QColor(80, 80, 80));
    painter.drawRect(0, 0, w - 1, h - 1);

    double fullRange = m_fullMax - m_fullMin;
    if (fullRange <= 0) return;

    // 绘制迷你波形（如果有数据）
    if (m_hasData && !m_xData.isEmpty()) {
        // 找到 Y 轴范围
        double yMin = *std::min_element(m_yData.begin(), m_yData.end());
        double yMax = *std::max_element(m_yData.begin(), m_yData.end());
        double yRange = yMax - yMin;
        if (yRange < 1e-9) yRange = 1.0;

        // 绘制波形折线
        painter.setPen(QPen(QColor(100, 180, 100), 1));
        QPainterPath path;
        bool first = true;

        // 降采样到屏幕宽度
        int step = qMax(1, m_xData.size() / w);
        for (int i = 0; i < m_xData.size(); i += step) {
            double x = (m_xData[i] - m_fullMin) / fullRange * w;
            double y = h - (m_yData[i] - yMin) / yRange * (h - 4) - 2;
            if (first) {
                path.moveTo(x, y);
                first = false;
            } else {
                path.lineTo(x, y);
            }
        }
        painter.drawPath(path);
    }

    // 绘制可视窗口高亮区域
    double visStartX = (m_visMin - m_fullMin) / fullRange * w;
    double visEndX = (m_visMax - m_fullMin) / fullRange * w;

    // 限制在控件范围内
    visStartX = qMax(0.0, qMin((double)w, visStartX));
    visEndX = qMax(0.0, qMin((double)w, visEndX));

    // 半透明高亮
    painter.fillRect(QRectF(visStartX, 0, visEndX - visStartX, h),
                     QColor(100, 150, 255, 60));

    // 高亮边框
    painter.setPen(QPen(QColor(100, 150, 255), 2));
    painter.drawRect(QRectF(visStartX, 0, visEndX - visStartX, h));
}
