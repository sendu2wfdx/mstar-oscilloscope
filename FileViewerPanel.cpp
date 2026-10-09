// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "FileViewerPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QComboBox>
#include <cmath>
#include <algorithm>

FileViewerPanel::FileViewerPanel(const QString &filePath, QWidget *parent)
    : QWidget(parent), m_filePath(filePath)
{
    QFileInfo fi(filePath);
    m_fileName = fi.fileName();
    initUI();
    // 注意：不在构造函数中加载文件，由外部调用 loadCsvFile()
}

FileViewerPanel::~FileViewerPanel()
{
}

void FileViewerPanel::initUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    // --- 文件信息行 ---
    auto *infoLayout = new QHBoxLayout;
    m_labelFileInfo = new QLabel("文件: " + m_fileName);
    m_labelSampleRate = new QLabel("采样率: --");
    infoLayout->addWidget(m_labelFileInfo);
    infoLayout->addStretch();
    infoLayout->addWidget(m_labelSampleRate);
    mainLayout->addLayout(infoLayout);

    // --- QCustomPlot 波形显示（与 DevicePanel 相同样式）---
    m_plot = new QCustomPlot;
    m_plot->setBackground(QColor(0, 0, 0));  // 黑色背景
    m_plot->xAxis->setBasePen(QPen(Qt::darkGray));
    m_plot->yAxis->setBasePen(QPen(Qt::darkGray));
    m_plot->xAxis->setTickPen(QPen(Qt::darkGray));
    m_plot->yAxis->setTickPen(QPen(Qt::darkGray));
    m_plot->xAxis->setSubTickPen(QPen(Qt::darkGray));
    m_plot->yAxis->setSubTickPen(QPen(Qt::darkGray));
    m_plot->xAxis->setTickLabelColor(Qt::lightGray);
    m_plot->yAxis->setTickLabelColor(Qt::lightGray);
    m_plot->xAxis->grid()->setPen(QPen(QColor(40, 40, 40)));
    m_plot->yAxis->grid()->setPen(QPen(QColor(40, 40, 40)));
    m_plot->xAxis->setLabel("Time (s)");
    m_plot->yAxis->setLabel("Voltage (V)");
    m_plot->xAxis->setLabelColor(Qt::lightGray);
    m_plot->yAxis->setLabelColor(Qt::lightGray);
    m_plot->legend->setVisible(false);

    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_plot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    m_plot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    connect(m_plot->xAxis, SIGNAL(rangeChanged(QCPRange)),
            this, SLOT(onAxisRangeChanged()));

    m_plot->xAxis->setRange(0, 1024);
    m_plot->yAxis->setRange(-5, 5);

    // 用户交互检测
    auto trackInteraction = [this]() {
        m_lastUserInteraction = QDateTime::currentMSecsSinceEpoch();
    };
    connect(m_plot, &QCustomPlot::mousePress, this, trackInteraction);
    connect(m_plot, &QCustomPlot::mouseWheel, this, trackInteraction);

    // 窗口位置指示器
    m_windowIndicator = new WindowIndicator;
    m_indicatorChCombo = new QComboBox;
    m_indicatorChCombo->setFixedWidth(60);
    m_indicatorChCombo->setToolTip(tr("指示器显示通道"));
    auto *indicatorLayout = new QHBoxLayout;
    indicatorLayout->setContentsMargins(0, 0, 0, 0);
    indicatorLayout->addWidget(m_indicatorChCombo);
    indicatorLayout->addWidget(m_windowIndicator, 1);

    auto *waveGroup = new QGroupBox(tr("波形显示"));
    auto *waveVLayout = new QVBoxLayout(waveGroup);
    waveVLayout->addLayout(indicatorLayout);
    waveVLayout->addWidget(m_plot, 1);

    // 显示使能
    auto *chGroup = new QGroupBox(tr("显示使能"));
    m_chLayout = new QVBoxLayout(chGroup);
    m_chLayout->addStretch();

    // 水平布局：波形显示 + 显示使能
    auto *plotHLayout = new QHBoxLayout;
    plotHLayout->addWidget(waveGroup, 1);
    plotHLayout->addWidget(chGroup);
    mainLayout->addLayout(plotHLayout, 1);
}

bool FileViewerPanel::loadCsvFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_labelFileInfo->setText("错误: 无法打开文件");
        return false;
    }

    QTextStream in(&file);

    // 解析元数据（# 开头的行）
    m_sampleRate = 1000000;  // 默认
    m_channelCount = 0;
    QString firstNonCommentLine;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (!line.startsWith("#")) {
            firstNonCommentLine = line;  // 保存第一行非注释数据
            break;
        }

        // 解析元数据
        if (line.contains("SampleRate", Qt::CaseInsensitive)) {
            QStringList parts = line.split("=");
            if (parts.size() >= 2) {
                m_sampleRate = parts[1].trimmed().toDouble();
            }
        } else if (line.contains("Channels", Qt::CaseInsensitive)) {
            QStringList parts = line.split("=");
            if (parts.size() >= 2) {
                m_channelCount = parts[1].trimmed().toInt();
            }
        }
    }

    // 读取数据
    m_channelData.clear();
    bool firstDataRow = true;

    // 处理之前保存的第一行非注释数据
    if (!firstNonCommentLine.isEmpty()) {
        QStringList fields = firstNonCommentLine.split(",");
        if (!fields.isEmpty()) {
            // 检查是否是列头
            bool ok;
            fields[0].toDouble(&ok);
            if (!ok) {
                // 是列头
                if (m_channelCount <= 0) {
                    m_channelCount = fields.size();
                }
                m_channelData.resize(m_channelCount);
                firstDataRow = false;
            } else {
                // 是数据行
                if (m_channelCount <= 0) {
                    m_channelCount = fields.size();
                    m_channelData.resize(m_channelCount);
                }
                for (int ch = 0; ch < qMin(m_channelCount, fields.size()); ++ch) {
                    double val = fields[ch].toDouble(&ok);
                    if (ok) m_channelData[ch].append(val);
                }
                firstDataRow = false;
            }
        }
    }

    // 继续读取剩余数据
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        QStringList fields = line.split(",");
        if (fields.isEmpty()) continue;

        // 检查是否是列头行（首字段非数字）
        bool isHeader = false;
        if (firstDataRow && !fields[0].isEmpty()) {
            bool ok;
            fields[0].toDouble(&ok);
            if (!ok) isHeader = true;
        }

        if (isHeader) {
            // 从列头推断通道数
            if (m_channelCount <= 0) {
                m_channelCount = fields.size();
            }
            m_channelData.resize(m_channelCount);
            firstDataRow = false;
            continue;
        }

        // 数据行
        if (m_channelCount <= 0) {
            m_channelCount = fields.size();
            m_channelData.resize(m_channelCount);
        }

        for (int ch = 0; ch < qMin(m_channelCount, fields.size()); ++ch) {
            bool ok;
            double val = fields[ch].toDouble(&ok);
            if (ok) {
                m_channelData[ch].append(val);
            }
        }
        firstDataRow = false;
    }

    file.close();

    // 检查是否成功加载数据
    if (m_channelCount <= 0 || m_channelData.isEmpty()) {
        m_labelFileInfo->setText("错误: 文件中没有有效数据");
        return false;
    }

    // 更新 UI
    m_labelSampleRate->setText(QString("采样率: %1 Hz").arg(m_sampleRate, 0, 'f', 0));

    // 初始化通道偏移
    m_chOffsets.resize(m_channelCount, 0.0);
    m_chOffsetEdits.clear();

    // 初始化通道使能
    m_chEnable.resize(m_channelCount, true);

    // 通道颜色（与 DevicePanel 一致）
    static const QColor chColors[] = {
        Qt::red, Qt::green, Qt::blue, Qt::magenta, Qt::cyan,
        Qt::darkYellow, Qt::darkGreen, Qt::darkMagenta
    };

    m_plot->clearPlottables();
    m_indicatorChCombo->clear();

    // 清除旧的控件
    QLayoutItem *child;
    while ((child = m_chLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    for (int ch = 0; ch < m_channelCount; ++ch) {
        // 创建图形
        auto *graph = m_plot->addGraph();
        graph->setName(QString("CH%1").arg(ch));
        graph->setPen(QPen(chColors[ch % 8]));
        graph->setVisible(m_chEnable[ch]);

        // 每通道一行：复选框 + 偏移输入
        auto *chRowLayout = new QHBoxLayout;
        chRowLayout->setContentsMargins(0, 0, 0, 0);
        chRowLayout->setSpacing(2);

        // 带颜色的复选框
        auto *cb = new QCheckBox(QString("CH%1").arg(ch));
        cb->setChecked(true);
        cb->setStyleSheet(QString("color: %1;").arg(chColors[ch % 8].name()));
        graph->setVisible(cb->isChecked());
        connect(cb, &QCheckBox::toggled, this, [this, ch](bool visible) {
            m_chEnable[ch] = visible;
            if (ch < m_plot->graphCount()) {
                m_plot->graph(ch)->setVisible(visible);
                m_plot->replot();
            }
        });
        chRowLayout->addWidget(cb);

        // 波形偏移输入
        auto *spinOffset = new QDoubleSpinBox;
        spinOffset->setRange(-100.0, 100.0);
        spinOffset->setDecimals(2);
        spinOffset->setSingleStep(0.1);
        spinOffset->setValue(0.0);
        spinOffset->setFixedWidth(80);
        spinOffset->setToolTip(QString("CH%1 波形偏移 (V)").arg(ch));
        connect(spinOffset, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, [this, ch](double value) {
            m_chOffsets[ch] = value;
            updatePlot();
        });
        chRowLayout->addWidget(spinOffset);
        m_chOffsetEdits.append(spinOffset);

        auto *chRowWidget = new QWidget;
        chRowWidget->setLayout(chRowLayout);
        m_chLayout->addWidget(chRowWidget);

        // 指示器通道选项
        m_indicatorChCombo->addItem(QString("CH%1").arg(ch));
    }
    m_chLayout->addStretch();
    m_plot->replot();

    // 设置数据并更新绘图
    updatePlot();

    return true;
}

void FileViewerPanel::updatePlot()
{
    if (m_channelData.isEmpty()) return;

    int sampleCount = 0;
    for (const auto &data : m_channelData) {
        sampleCount = qMax(sampleCount, data.size());
    }
    if (sampleCount == 0) return;

    // 生成时间轴（秒）
    QVector<double> timeAxis(sampleCount);
    double dt = 1.0 / m_sampleRate;  // 秒
    for (int i = 0; i < sampleCount; ++i) {
        timeAxis[i] = i * dt;
    }

    // 设置各通道数据（应用偏移）
    for (int ch = 0; ch < m_channelCount && ch < m_plot->graphCount(); ++ch) {
        if (m_channelData[ch].isEmpty()) continue;
        // 应用通道偏移
        double offset = (ch < m_chOffsets.size()) ? m_chOffsets[ch] : 0.0;
        if (offset != 0.0) {
            QVector<double> offsetData(m_channelData[ch].size());
            for (int i = 0; i < m_channelData[ch].size(); ++i) {
                offsetData[i] = m_channelData[ch][i] + offset;
            }
            m_plot->graph(ch)->setData(timeAxis, offsetData);
        } else {
            m_plot->graph(ch)->setData(timeAxis, m_channelData[ch]);
        }
    }

    // 自动缩放坐标轴
    m_plot->xAxis->setRange(0, timeAxis.last());
    
    // Y 轴自适应（考虑偏移）
    double ymin = 1e9, ymax = -1e9;
    for (int ch = 0; ch < m_channelCount && ch < m_chEnable.size() && ch < m_channelData.size(); ++ch) {
        if (!m_chEnable[ch] || m_channelData[ch].isEmpty()) continue;
        double offset = (ch < m_chOffsets.size()) ? m_chOffsets[ch] : 0.0;
        for (double v : m_channelData[ch]) {
            double val = v + offset;
            ymin = qMin(ymin, val);
            ymax = qMax(ymax, val);
        }
    }
    if (ymin < ymax) {
        double margin = (ymax - ymin) * 0.1;
        m_plot->yAxis->setRange(ymin - margin, ymax + margin);
    }

    m_plot->replot();

    // 更新窗口指示器
    updateWindowIndicator();
}

void FileViewerPanel::updateWindowIndicator()
{
    if (m_channelData.isEmpty()) return;

    int indicatorCh = m_indicatorChCombo->currentIndex();
    if (indicatorCh < 0 || indicatorCh >= m_channelCount) indicatorCh = 0;
    if (m_channelData[indicatorCh].isEmpty()) return;

    int sampleCount = m_channelData[indicatorCh].size();
    double totalTimeSec = sampleCount / m_sampleRate;
    double dt = 1.0 / m_sampleRate;

    m_windowIndicator->setFullRange(0, totalTimeSec);
    double xMin = m_plot->xAxis->range().lower;
    double xMax = m_plot->xAxis->range().upper;
    m_windowIndicator->setVisibleRange(xMin, xMax);

    // 生成迷你波形数据（降采样）
    int indicatorStep = qMax(1, sampleCount / 2000);
    int nPoints = (sampleCount + indicatorStep - 1) / indicatorStep;
    QVector<double> xInd, yInd;
    xInd.reserve(nPoints);
    yInd.reserve(nPoints);

    double offset = (indicatorCh < m_chOffsets.size()) ? m_chOffsets[indicatorCh] : 0.0;
    const auto &data = m_channelData[indicatorCh];
    for (int i = 0; i < nPoints; ++i) {
        int idx = i * indicatorStep;
        xInd.append(idx * dt);
        yInd.append(idx < data.size() ? data[idx] + offset : 0.0);
    }
    m_windowIndicator->setWaveformData(xInd, yInd);
}

void FileViewerPanel::onAxisRangeChanged()
{
    updateWindowIndicator();
}
