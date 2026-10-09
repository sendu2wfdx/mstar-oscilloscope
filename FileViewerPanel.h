// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef FILEVIEWERPANEL_H
#define FILEVIEWERPANEL_H

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QVector>
#include "qcustomplot.h"
#include "WindowIndicator.h"

/**
 * @brief 数据文件浏览面板
 *
 * 专门用于浏览 CSV 数据文件，无采集功能。
 * 特点：
 * - 无采集控制按钮
 * - 显示文件名和采样率信息
 * - 波形显示、缩放、平移
 * - 通道显示使能控制
 */
class FileViewerPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FileViewerPanel(const QString &filePath, QWidget *parent = nullptr);
    ~FileViewerPanel();

    // 设备标识（虚拟）
    QString deviceMac() const { return "FILE_" + m_fileName; }
    QString deviceIp() const { return "FILE"; }
    QString deviceModel() const { return m_fileName; }
    QString masterIp() const { return QString(); }  // 文件查看器无主从关系

    // 文件信息
    QString filePath() const { return m_filePath; }
    QString fileName() const { return m_fileName; }

    // 加载 CSV 文件
    bool loadCsvFile(const QString &filePath);

signals:
    void plotReady();

private slots:
    void onAxisRangeChanged();

private:
    void initUI();
    void updatePlot();
    void updateWindowIndicator();

    // === UI 控件 ===
    QLabel *m_labelFileInfo;
    QLabel *m_labelSampleRate;
    QCustomPlot *m_plot;
    WindowIndicator *m_windowIndicator;
    QComboBox *m_indicatorChCombo = nullptr;
    QVBoxLayout *m_chLayout;

    // === 文件数据 ===
    QString m_filePath;
    QString m_fileName;
    double m_sampleRate = 1000000;  // 默认 1MHz
    int m_channelCount = 0;
    QVector<QVector<double>> m_channelData;  // 各通道数据
    QVector<bool> m_chEnable;
    QVector<double> m_chOffsets;  // 通道波形偏移 (V)
    QVector<QDoubleSpinBox*> m_chOffsetEdits;  // 通道偏移输入控件

    // 用户交互追踪
    int64_t m_lastUserInteraction = 0;
    static constexpr int PLOT_INTERACTION_TIMEOUT = 10000;
};

#endif // FILEVIEWERPANEL_H
