// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include <QtGlobal>
#include <QVector>
#include <QColor>
#include <cstdint>

/**
 * @brief 波形显示控件
 *
 * 用于实时显示多通道采集数据的波形。黑色背景，带网格，
 * 每个通道用不同颜色绘制，支持自动缩放。
 */
class WaveformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformWidget(QWidget *parent = nullptr);

    /// 设置通道数量，自动分配颜色
    void setChannelCount(int count);

    /// 设置指定通道的波形数据（电压值数组）
    void setChannelData(int channel, const QVector<float> &data);

    /// 获取指定通道的当前波形数据
    QVector<float> channelData(int channel) const;

    /// 清空所有通道数据
    void clearData();

    /// 设置Y轴电压范围（自动模式下无效）
    void setVoltageRange(float vmin, float vmax);

    /// 设置X轴显示采样点数
    void setDisplayPoints(qsizetype points);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawGrid(QPainter &painter, const QRect &area);
    void drawWaveform(QPainter &painter, const QRect &area,
                      const QVector<float> &data, const QColor &color);
    void drawAxisLabels(QPainter &painter, const QRect &area);

    struct ChannelData {
        QVector<float> data;
        QColor color;
        bool visible = true;
    };

    QVector<ChannelData> m_channels;
    float m_vmin = -5.0f;
    float m_vmax = 5.0f;
    qsizetype m_displayPoints = 1024;
    bool m_autoRange = true;

    static const QColor s_channelColors[];
    static const int s_maxChannels;
};

#endif // WAVEFORMWIDGET_H
