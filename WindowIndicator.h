// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef WINDOWINDICATOR_H
#define WINDOWINDICATOR_H

#include <QWidget>
#include <QVector>

// 窗口位置指示器 - 类似实体示波器的窗口位置示意图
class WindowIndicator : public QWidget
{
    Q_OBJECT
public:
    explicit WindowIndicator(QWidget *parent = nullptr);

    // 设置全数据范围
    void setFullRange(double min, double max);

    // 设置当前可视窗口范围
    void setVisibleRange(double min, double max);

    // 设置波形数据（用于绘制迷你波形）
    void setWaveformData(const QVector<double> &xData, const QVector<double> &yData);

    // 清空所有数据
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_fullMin = 0;
    double m_fullMax = 1;
    double m_visMin = 0;
    double m_visMax = 1;
    QVector<double> m_xData;
    QVector<double> m_yData;
    bool m_hasData = false;
};

#endif // WINDOWINDICATOR_H
