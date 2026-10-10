// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "DevicePanel.h"
#include "CollectParamDialog.h"
#include "DataFileWriter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QMetaObject>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDoubleSpinBox>
#include <cstring>
#include <memory>
#include "Translation.h"

DevicePanel::DevicePanel(const QString &ip, MHANDLE hdev,
                         collectinforst info, int netSpeedMbps,
                         QWidget *parent)
    : QWidget(parent)
    , m_deviceIp(ip)
    , m_hdev(hdev)
    , m_collectInfo(info)
    , m_netSpeedMbps(netSpeedMbps)
{
    initUI();
    initDevice();
}

DevicePanel::~DevicePanel()
{
    disconnectDevice();
}

void DevicePanel::initUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    // --- 采集控制行 ---
    auto *collectGroup = new QGroupBox("Collect");
    auto *collectLayout = new QHBoxLayout(collectGroup);
    m_btnInstant = new QPushButton(T("瞬时采集"));
    m_btnSingleTrig = new QPushButton(T("单次触发"));
    m_btnMultiTrig = new QPushButton(T("连续触发"));
    m_btnContinuous = new QPushButton(T("连续采集"));
    m_btnStop = new QPushButton("Stop");
    m_btnStop->setEnabled(false);
    collectLayout->addWidget(m_btnInstant);
    collectLayout->addWidget(m_btnSingleTrig);
    collectLayout->addWidget(m_btnMultiTrig);
    collectLayout->addWidget(m_btnContinuous);
    collectLayout->addWidget(m_btnStop);
    m_btnParams = new QPushButton(T("参数设置"));
    collectLayout->addWidget(m_btnParams);
    m_btnSaveData = new QPushButton(T("保存数据"));
    m_btnSaveData->setEnabled(false);
    collectLayout->addWidget(m_btnSaveData);
    mainLayout->addWidget(collectGroup);

    // --- Info labels ---
    m_labelDevInfo = new QLabel;
    m_labelStatus = new QLabel(T("Ready"));
    auto *infoLayout = new QHBoxLayout;
    infoLayout->addWidget(m_labelDevInfo);
    infoLayout->addStretch();
    infoLayout->addWidget(m_labelStatus);
    mainLayout->addLayout(infoLayout);

    // --- QCustomPlot 波形显示 ---
    m_plot = new QCustomPlot;
    m_plot->setBackground(QColor(0, 0, 0));
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
    m_plot->xAxis->setLabel(T("Time (s)"));
    m_plot->yAxis->setLabel(T("Voltage (V)"));
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

    // 触发位置光标
    m_trigLine = new QCPItemStraightLine(m_plot);
    m_trigLine->point1->setCoords(0, -1e9);
    m_trigLine->point2->setCoords(0, 1e9);
    m_trigLine->setPen(QPen(QColor(255, 80, 80), 2, Qt::DashLine));
    m_trigLine->setVisible(false);

    // 窗口位置指示器
    m_windowIndicator = new WindowIndicator;
    m_indicatorChCombo = new QComboBox;
    m_indicatorChCombo->setFixedWidth(60);
    m_indicatorChCombo->setToolTip(T("指示器显示通道"));
    auto *indicatorLayout = new QHBoxLayout;
    indicatorLayout->setContentsMargins(0, 0, 0, 0);
    indicatorLayout->addWidget(m_indicatorChCombo);
    indicatorLayout->addWidget(m_windowIndicator, 1);

    auto *waveGroup = new QGroupBox(T("波形显示"));
    auto *waveVLayout = new QVBoxLayout(waveGroup);
    waveVLayout->addLayout(indicatorLayout);
    waveVLayout->addWidget(m_plot, 1);

    // 显示使能
    auto *chGroup = new QGroupBox(T("显示使能"));
    m_chLayout = new QVBoxLayout(chGroup);
    m_chLayout->addStretch();

    auto *plotHLayout = new QHBoxLayout;
    plotHLayout->addWidget(waveGroup, 1);
    plotHLayout->addWidget(chGroup);
    mainLayout->addLayout(plotHLayout, 1);

    // --- Signals ---
    connect(m_btnInstant, &QPushButton::clicked, this, &DevicePanel::onInstantCollect);
    connect(m_btnSingleTrig, &QPushButton::clicked, this, &DevicePanel::onSingleTriggerCollect);
    connect(m_btnMultiTrig, &QPushButton::clicked, this, &DevicePanel::onMultiTriggerCollect);
    connect(m_btnContinuous, &QPushButton::clicked, this, &DevicePanel::onContinuousCollect);
    connect(m_btnStop, &QPushButton::clicked, this, &DevicePanel::onStopCollect);
    connect(m_btnParams, &QPushButton::clicked, this, &DevicePanel::onParamSettings);
    connect(m_btnSaveData, &QPushButton::clicked, this, &DevicePanel::onSaveData);
    connect(this, &DevicePanel::plotReady, this, &DevicePanel::onPlotReady);
    connect(m_indicatorChCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        std::lock_guard<std::mutex> lock(m_collectMtx);
        if (m_ringChannelCount > 0 && !m_ringBuffers.empty() && m_ringBuffers[0].totalWritten > 0)
            updateWindowIndicator();
    });
}

void DevicePanel::initDevice()
{
    m_deviceMac = "";  // 需要从 EthBridgeInfo 获取，在 MainWindow 中设置
    // 此处使用 collectInfo 中的信息
    int bytesPerSample = (m_collectInfo.nbits + 7) / 8;
    m_displayPoints = m_collectInfo.databuflen / m_collectInfo.nchannel / bytesPerSample;

    // 主机缓存默认值（与设备缓存相同，用户可在参数对话框中修改）
    if (m_hostBufSamples <= 0) {
        m_hostBufSamples = m_displayPoints;
    }

    // 通道颜色表
    static const QColor chColors[] = {
        QColor(0, 200, 255), QColor(255, 100, 100),
        QColor(100, 255, 100), QColor(255, 255, 0),
        QColor(255, 0, 255), QColor(255, 165, 0),
        QColor(0, 255, 255), QColor(200, 200, 255),
    };

    m_plot->clearGraphs();
    while (m_chLayout->count() > 0) {
        QLayoutItem *child = m_chLayout->takeAt(0);
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    m_indicatorChCombo->clear();
    for (int ch = 0; ch < m_collectInfo.nchannel; ++ch)
        m_indicatorChCombo->addItem(QString("CH%1").arg(ch));
    m_indicatorChCombo->setCurrentIndex(0);

    m_chOffsets.resize(m_collectInfo.nchannel);
    m_chOffsets.fill(0.0);
    m_chOffsetEdits.clear();

    for (int ch = 0; ch < m_collectInfo.nchannel; ++ch) {
        m_plot->addGraph();
        m_plot->graph(ch)->setPen(QPen(chColors[ch % 8]));
        m_plot->graph(ch)->setName(QString("CH%1").arg(ch));

        auto *chRowLayout = new QHBoxLayout;
        chRowLayout->setContentsMargins(0, 0, 0, 0);
        chRowLayout->setSpacing(2);

        auto *cb = new QCheckBox(QString("CH%1").arg(ch));
        cb->setChecked(true);
        cb->setStyleSheet(QString("color: %1;").arg(chColors[ch % 8].name()));
        m_plot->graph(ch)->setVisible(cb->isChecked());
        connect(cb, &QCheckBox::toggled, [this, ch](bool visible) {
            m_plot->graph(ch)->setVisible(visible);
            m_plot->replot();
        });
        chRowLayout->addWidget(cb);

        auto *spinOffset = new QDoubleSpinBox;
        spinOffset->setRange(-100.0, 100.0);
        spinOffset->setDecimals(2);
        spinOffset->setSingleStep(0.1);
        spinOffset->setValue(0.0);
        spinOffset->setFixedWidth(80);
        spinOffset->setToolTip(T("CH%1 波形偏移 (V)").arg(ch));
        connect(spinOffset, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this, ch](double value) {
            m_chOffsets[ch] = value;
            std::lock_guard<std::mutex> lock(m_collectMtx);
            if (m_ringChannelCount > 0 && !m_ringBuffers.empty() && m_ringBuffers[0].totalWritten > 0)
                updatePlotFromCache();
        });
        chRowLayout->addWidget(spinOffset);
        m_chOffsetEdits.append(spinOffset);

        auto *chRowWidget = new QWidget;
        chRowWidget->setLayout(chRowLayout);
        m_chLayout->addWidget(chRowWidget);
    }
    m_chLayout->addStretch();
    m_plot->replot();

    // 设置采样频率
    uint32_t freq = set_collect_samplefreq(m_hdev, m_sampleFreq);
    if (freq > 0) m_sampleFreq = freq;  // 返回值无效时保持原值

    // 设置量程
    QVector<uint8_t> ranges(m_collectInfo.nchannel, AD_AIN_RANGE_SPN5V);
    set_collect_range(m_hdev, ranges.data(), ranges.size());

    // 初始化参数对话框
    m_paramDialog = new CollectParamDialog(this);
    m_paramDialog->initFromDevice(m_hdev, m_collectInfo);
    {
        QVector<int16_t> zeroOffsets(m_collectInfo.nchannel, 0);
        set_collect_offset(m_hdev, zeroOffsets.data(), zeroOffsets.size());
    }
    m_paramDialog->setSampleFreq(m_sampleFreq);
    if (m_bufLen == 0) m_bufLen = m_collectInfo.databuflen;
    m_paramDialog->setBufLen(m_bufLen);
    m_paramDialog->setPreTrigger(m_preTrigger);
    m_paramDialog->setOversample(m_oversample);
    set_collect_oversample(m_hdev, m_oversample);

    // 初始化通道使能默认值
    for (int ch = 0; ch < m_collectInfo.nchannel; ++ch)
        m_chEnable.append(true);
    for (int ch = 0; ch < m_collectInfo.nchannel; ++ch)
        m_trigEnable.append(false);
    for (int ch = 0; ch < m_collectInfo.nchannel; ++ch)
        m_trigType.append(TRIG_TYPE_NORMAL);
    for (int ch = 0; ch < m_collectInfo.nchannel; ++ch)
        m_trigValue.append(0);

    // 注意：loadDeviceProfile() 在 MainWindow 设置 MAC 后调用

    m_labelStatus->setText(T("状态：空闲（%1CH %2bit %3Msps）")
        .arg(m_collectInfo.nchannel)
        .arg(m_collectInfo.nbits)
        .arg(m_collectInfo.samplefreq_max / 1000000.0, 0, 'f', 2));

    updateUI();
}

void DevicePanel::disconnectDevice()
{
    saveDeviceProfile();

    {
        std::lock_guard<std::mutex> lock(m_collectMtx);
        if (m_collecting) m_collecting = false;
    }
    if (m_collectThread.joinable()) m_collectThread.join();

    if (m_hdev) {
        device_handle_release(m_hdev);
        m_hdev = nullptr;
    }
}

QString DevicePanel::deviceModel() const
{
    // 优先使用设备固件返回的型号字符串（如 MS0816A-1000）
    QString model = QString::fromUtf8(m_collectInfo.devmodel).trimmed();
    if (!model.isEmpty()) return model;

    // 回退：根据参数动态生成（格式: MS{通道数}{AD精度}-{采样率KHz}）
    int ch = m_collectInfo.nchannel;
    int bits = m_collectInfo.nbits;
    uint32_t freq = m_collectInfo.samplefreq_max;
    QString freqStr;
    if (freq >= 1000000) {
        freqStr = QString("%1M").arg(freq / 1000000);
    } else if (freq >= 1000) {
        freqStr = QString("%1K").arg(freq / 1000);
    } else {
        freqStr = QString::number(freq);
    }
    return QString("MS%1%2-%3").arg(ch, 2, 10, QChar('0')).arg(bits, 2, 10, QChar('0')).arg(freqStr);
}

bool DevicePanel::hasAnyTrigEnable() const
{
    for (bool en : m_trigEnable) {
        if (en) return true;
    }
    return false;
}

bool DevicePanel::hasTrigCapability() const
{
    if (m_trigIn) return true;
    return hasAnyTrigEnable();
}

void DevicePanel::onInstantCollect()
{
    if (!m_hdev || m_collecting) return;
    if (m_collectThread.joinable()) m_collectThread.join();
    m_collectMode = 0;
    startCollectCommon(false, false);
}

void DevicePanel::onSingleTriggerCollect()
{
    if (!m_hdev || m_collecting) return;
    if (m_collectThread.joinable()) m_collectThread.join();
    m_collectMode = 1;
    startCollectCommon(true, false);
}

void DevicePanel::onMultiTriggerCollect()
{
    if (!m_hdev || m_collecting) return;
    if (m_collectThread.joinable()) m_collectThread.join();
    m_collectMode = 2;
    startCollectCommon(true, true);
}

void DevicePanel::onContinuousCollect()
{
    if (!m_hdev || m_collecting) return;
    if (m_collectThread.joinable()) m_collectThread.join();

    // 检查主机缓存是否已配置
    if (m_hostBufSamples <= 0) {
        QMessageBox::warning(this, T("提示"),
            T("请先打开参数设置对话框配置主机缓存大小。"));
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(T("连续采集"));
    msgBox.setText(T("确认开始连续采集？"));
    QCheckBox chk(T("同时保存数据到文件"));
    chk.setChecked(m_saveEnabled);
    msgBox.setCheckBox(&chk);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    if (msgBox.exec() != QMessageBox::Ok) return;

    m_saveEnabled = chk.isChecked();
    if (m_saveEnabled && m_paramDialog) {
        m_savePath = m_paramDialog->savePath();
        m_filePrefix = m_paramDialog->filePrefix();
    }
    m_collectMode = 3;
    startCollectCommon(false, true);
}

void DevicePanel::startCollectCommon(bool useTrig, bool loop)
{
    int chnum = m_collectInfo.nchannel;
    int bytesPerSample = (m_collectInfo.nbits + 7) / 8;
    int buflen = m_bufLen;

    if (m_collectMode == 3) {
        int dataRate = m_sampleFreq * chnum * bytesPerSample;
        if (m_netSpeedMbps > 0) {
            int maxDataRate = m_netSpeedMbps * 25000;
            if (dataRate > maxDataRate) {
                QMessageBox::warning(this, T("带宽超限"),
                    T("数据速率 %1 MB/s 超过网口有效带宽 %2 MB/s（40%利用率）。\n"
                       "请降低采样率或通道数。")
                    .arg(dataRate / 1000000.0, 0, 'f', 2)
                    .arg(maxDataRate / 1000000.0, 0, 'f', 2));
                return;
            }
        }
        int minByTime = dataRate * 0.02*2;
        int minAbs=4096*2;
        int minBuf = qMax(minAbs, minByTime);
        minBuf = (minBuf + 4095) & ~4095;
        buflen = qMin((int)m_collectInfo.databuflen, minBuf);
    }

    for (int ch = 0; ch < chnum; ++ch) {
        bool enable = (ch < m_chEnable.size()) ? m_chEnable[ch] : false;
        uint32_t ttype;
        int32_t tval = 0;
        if (m_collectMode == 3) {
            ttype = TRIG_TYPE_CONTINOUS;
        } else if (!useTrig) {
            ttype = TRIG_TYPE_NORMAL;
        } else {
            bool trigEn = (ch < m_trigEnable.size()) ? m_trigEnable[ch] : false;
            ttype = trigEn ? ((ch < m_trigType.size()) ? m_trigType[ch] : TRIG_TYPE_LEVEL)
                           : TRIG_TYPE_CONTINOUS;
            double voltVal = (ch < m_trigValue.size()) ? m_trigValue[ch] : 0.0;
            tval = collect_volt_toaddata(m_hdev, ch, (float)voltVal);
        }
        set_collect_chworkmode(m_hdev, ch, tval, ttype, enable);
    }

    int prebuflen = (m_collectMode == 1 || m_collectMode == 2) ? m_preTrigger : 0;
    int maxPreSamples = buflen / (chnum * bytesPerSample) - 1;
    if (maxPreSamples < 0) maxPreSamples = 0;
    if (prebuflen > maxPreSamples) prebuflen = maxPreSamples;
    int ret = collect_start(m_hdev, 0, buflen, prebuflen);
    if (ret <= 0) {
        QMessageBox::warning(this, T("错误"), T("Collect start failed!"));
        return;
    }

    int sampleSize = chnum * bytesPerSample;
    int devSamples = buflen / sampleSize;
    int hostSamples = (m_collectMode == 3) ? m_hostBufSamples : devSamples;
    {
        std::lock_guard<std::mutex> lock(m_collectMtx);
        m_ringBuffers.resize(chnum);
        m_ringChannelCount = chnum;
        for (int ch = 0; ch < chnum; ++ch)
            m_ringBuffers[ch].allocate(hostSamples);
    }

    m_chScales.resize(chnum);
    for (int ch = 0; ch < chnum; ++ch)
        m_chScales[ch] = collect_get_channelscale(m_hdev, ch);

    m_collecting = true;
    m_lastUserInteraction = 0;
    m_fillYMaxAbs = 0;
    m_btnInstant->setEnabled(false);
    m_btnSingleTrig->setEnabled(false);
    m_btnMultiTrig->setEnabled(false);
    m_btnContinuous->setEnabled(false);
    m_btnStop->setEnabled(m_collectMode != 0);

    QString modeName[] = {T("瞬时采集"), T("单次触发"), T("连续触发"), T("连续采集")};
    m_labelStatus->setText(T("状态：%1 中...").arg(modeName[m_collectMode]));

    if (m_collectMode == 3) {
        {
            double dt = 1.0 / m_sampleFreq;
            m_plot->xAxis->blockSignals(true);
            m_plot->xAxis->setRange(0, hostSamples * dt);
            m_plot->xAxis->blockSignals(false);
        }

        if (m_saveEnabled) {
            QVector<int> enabledChs;
            for (int ch = 0; ch < chnum; ++ch)
                if (ch < m_chEnable.size() && m_chEnable[ch])
                    enabledChs.append(ch);
            m_fileWriter = new DataFileWriter();
            m_fileWriter->setWriteSpeedCallback([this]() {
                QMessageBox::warning(this, T("保存速度不足"),
                    T("磁盘写入速度跟不上采集速度，数据正在丢失。\n"
                       "已保存的数据仍然有效。\n"
                       "建议降低采样率或通道数后重试。"));
                if (m_fileWriter) {
                    m_fileWriter->stop();
                    delete m_fileWriter;
                    m_fileWriter = nullptr;
                }
                m_saveEnabled = false;
                m_labelStatus->setText(T("状态：采集继续（已放弃保存）"));
            });
            m_fileWriter->start(m_savePath, m_filePrefix, m_sampleFreq,
                                enabledChs.size(), enabledChs,
                                m_deviceMac, "Continuous");
        }

        int halfSamples = devSamples / 2;
        m_collectThread = std::thread([this, buflen, chnum, devSamples, hostSamples, halfSamples, bytesPerSample]() {
            int halfBytes = halfSamples * chnum * bytesPerSample;
            auto rawbufA = std::make_unique<uint8_t[]>(buflen);
            auto rawbufB = std::make_unique<uint8_t[]>(buflen);
            uint8_t* rawbufs[2] = { rawbufA.get(), rawbufB.get() };
            int writePos = 0;
            auto lastUpdateTime = std::chrono::steady_clock::now();
            auto lastIndicatorUpdate = std::chrono::steady_clock::now();

            QVector<QVector<float>> batch;
            if (m_fileWriter) {
                batch.resize(chnum);
                for (int ch = 0; ch < chnum; ++ch)
                    batch[ch].resize(halfSamples);
            }

            auto processData = [&](uint8_t* buf) {
                int16_t* pdata = (int16_t*)buf;
                {
                    std::lock_guard<std::mutex> lock(m_collectMtx);
                    for (int i = 0; i < halfSamples; ++i) {
                        for (int ch = 0; ch < chnum; ++ch) {
                            float v = (float)pdata[i * chnum + ch] * m_chScales[ch];
                            m_ringBuffers[ch].data[writePos] = v;
                            if (m_fileWriter) batch[ch][i] = v;
                        }
                        writePos = (writePos + 1) % hostSamples;
                    }
                    for (int ch = 0; ch < chnum; ++ch) {
                        m_ringBuffers[ch].writePos = writePos;
                        m_ringBuffers[ch].totalWritten += halfSamples;
                    }
                    m_plotNumSamples = qMin((qint64)hostSamples, m_ringBuffers[0].totalWritten);
                }
                if (m_fileWriter) m_fileWriter->enqueue(batch);
            };

            auto tryUpdatePlot = [&]() {
                auto now = std::chrono::steady_clock::now();
                auto indicatorElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastIndicatorUpdate).count();
                if (indicatorElapsed >= PLOT_MIN_UPDATE_INTERVAL) {
                    lastIndicatorUpdate = now;
                    QMetaObject::invokeMethod(this, [this]() { updateWindowIndicator(); }, Qt::QueuedConnection);
                }
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime).count();
                if (elapsed < PLOT_MIN_UPDATE_INTERVAL) return;
                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count();
                if (nowMs - m_lastUserInteraction.load() >= PLOT_INTERACTION_TIMEOUT) {
                    lastUpdateTime = now;
                    QMetaObject::invokeMethod(this, [this]() { updatePlotFromCache(); }, Qt::QueuedConnection);
                }
            };

            {
                std::lock_guard<std::mutex> lock(m_collectMtx);
                if (!m_collecting) goto thread_exit;
            }
            if (!wait_dmaFirstHalfOk(m_hdev, 5000)) goto thread_exit;
            {
                std::lock_guard<std::mutex> lock(m_collectMtx);
                if (!m_collecting) goto thread_exit;
            }
            if (axifull_read(m_hdev, 0, rawbufs[0], halfBytes) <= 0) goto thread_exit;

            while (true) {
                if (!wait_dmaSecondHalfOk(m_hdev, 5000)) break;
                { std::lock_guard<std::mutex> lock(m_collectMtx); if (!m_collecting) break; }
                if (axifull_read(m_hdev, halfBytes, rawbufs[1], halfBytes) <= 0) break;
                processData(rawbufs[0]);
                tryUpdatePlot();

                if (!wait_dmaFirstHalfOk(m_hdev, 5000)) break;
                { std::lock_guard<std::mutex> lock(m_collectMtx); if (!m_collecting) break; }
                if (axifull_read(m_hdev, 0, rawbufs[0], halfBytes) <= 0) break;
                processData(rawbufs[1]);
                tryUpdatePlot();
            }

thread_exit:
            collect_send_stopreq(m_hdev);
            collect_waitstop(m_hdev, 5000);
            if (m_fileWriter) {
                m_fileWriter->stop();
                delete m_fileWriter;
                m_fileWriter = nullptr;
            }
            {
                std::lock_guard<std::mutex> lock(m_collectMtx);
                m_collecting = false;
            }
            QMetaObject::invokeMethod(this, [this]() {
                m_labelStatus->setText(T("状态：空闲（%1 采样点）").arg(m_plotNumSamples));
                updateUI();
            }, Qt::QueuedConnection);
        });
    } else {
        int waitTimeout = (m_collectMode == 0) ? 10000 : 0;
        m_collectThread = std::thread([this, buflen, chnum, devSamples, waitTimeout, loop, bytesPerSample, prebuflen]() {
            while (true) {
                {
                    std::lock_guard<std::mutex> lock(m_collectMtx);
                    if (!m_collecting) break;
                }
                if (loop) {
                    int ret = collect_start(m_hdev, 0, buflen, prebuflen);
                    if (ret <= 0) break;
                }
                if (!collect_waitstop(m_hdev, waitTimeout)) {
                    std::lock_guard<std::mutex> lock(m_collectMtx);
                    bool wasCollecting = m_collecting;
                    if (!m_collecting) break;
                    if (wasCollecting) {
                        m_collecting = false;
                        m_plotNumSamples = -1;
                        emit plotReady();
                    }
                    break;
                }
                QMetaObject::invokeMethod(m_labelStatus, "setText", Qt::QueuedConnection,
                                          Q_ARG(QString, T("状态：正在数据传输")));
                auto rawbuf = std::make_unique<uint8_t[]>(buflen);
                int ret = axifull_read(m_hdev, 0, rawbuf.get(), buflen);
                if (ret <= 0) {
                    std::lock_guard<std::mutex> lock(m_collectMtx);
                    if (!m_collecting) break;
                    m_collecting = false;
                    m_plotNumSamples = -2;
                    emit plotReady();
                    break;
                }
                int dataStartSample = 0;
                if (m_collectMode == 1 || m_collectMode == 2) {
                    uint32_t trigCh = 0;
                    get_collect_trig_chnum(m_hdev, &trigCh);
                    uint32_t trigpos = 0, endpos = 0;
                    get_collect_trigpos(m_hdev, trigCh, &trigpos, &endpos);
                    dataStartSample = (endpos + 1) % devSamples;
                    m_trigPos = ((int)trigpos - dataStartSample + devSamples) % devSamples;
                } else {
                    m_trigPos = -1;
                }
                int16_t* pdata = (int16_t*)rawbuf.get();
                {
                    std::lock_guard<std::mutex> lock(m_collectMtx);
                    for (int i = 0; i < devSamples; ++i) {
                        int srcIdx = (dataStartSample + i) % devSamples;
                        for (int ch = 0; ch < chnum; ++ch)
                            m_ringBuffers[ch].data[i] = (float)pdata[srcIdx * chnum + ch] * m_chScales[ch];
                    }
                    for (int ch = 0; ch < chnum; ++ch) {
                        m_ringBuffers[ch].writePos = devSamples;
                        m_ringBuffers[ch].totalWritten = devSamples;
                    }
                    m_plotNumSamples = devSamples;
                }
                emit plotReady();
                if (!loop) {
                    std::lock_guard<std::mutex> lock(m_collectMtx);
                    m_collecting = false;
                    break;
                }
                {
                    std::lock_guard<std::mutex> lock(m_collectMtx);
                    if (!m_collecting) break;
                }
            }
            QMetaObject::invokeMethod(this, [this]() {
                if (!m_collecting) {
                    m_labelStatus->setText(T("状态：空闲（%1 采样点）").arg(m_plotNumSamples));
                    updateUI();
                }
            }, Qt::QueuedConnection);
        });
    }
}

void DevicePanel::onPlotReady()
{
    int numSamples, chnum;
    {
        std::lock_guard<std::mutex> lock(m_collectMtx);
        if (m_plotNumSamples == -1) {
            m_trigLine->setVisible(false);
            m_plot->replot();
            m_labelStatus->setText(T("状态：空闲（DMA 超时）"));
            updateUI();
            return;
        }
        if (m_plotNumSamples == -2) {
            m_trigLine->setVisible(false);
            m_plot->replot();
            m_labelStatus->setText(T("状态：空闲（读取失败）"));
            updateUI();
            return;
        }
        numSamples = m_plotNumSamples;
        chnum = m_ringChannelCount;
    }
    double dt = 1.0 / m_sampleFreq;
    m_plot->xAxis->blockSignals(true);
    m_plot->xAxis->setRange(0, numSamples * dt);
    m_plot->xAxis->blockSignals(false);
    m_windowIndicator->setFullRange(0, numSamples * dt);
    {
        std::lock_guard<std::mutex> lock(m_collectMtx);
        updatePlotFromCache();
    }
    bool stillCollecting;
    {
        std::lock_guard<std::mutex> lock(m_collectMtx);
        stillCollecting = m_collecting;
    }
    if (!stillCollecting) {
        m_labelStatus->setText(T("状态：空闲（%1 采样点）").arg(numSamples));
        updateUI();
        if (m_saveEnabled && m_collectMode == 3) autoSaveData();
    }
}

void DevicePanel::updatePlotFromCache()
{
    if (m_ringChannelCount <= 0 || m_ringBuffers.empty()) return;
    int capacity = m_ringBuffers[0].capacity;
    qint64 totalWritten = m_ringBuffers[0].totalWritten;
    if (totalWritten <= 0) return;
    int writePos = m_ringBuffers[0].writePos;
    double dt = 1.0 / m_sampleFreq;

    int validCount = (int)qMin(totalWritten, (qint64)capacity);
    double startTime, endTime;
    if (totalWritten <= capacity) {
        startTime = 0;
        endTime = capacity * dt;
    } else {
        startTime = (totalWritten - capacity) * dt;
        endTime = totalWritten * dt;
    }

    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    if (!m_autoScrolling && nowMs - m_lastUserInteraction.load() >= PLOT_INTERACTION_TIMEOUT) {
        m_autoScrolling = true;
        if (totalWritten > capacity)
            m_plot->xAxis->setRange(startTime, endTime);
        m_autoScrolling = false;
    }

    double xMin = m_plot->xAxis->range().lower;
    double xMax = m_plot->xAxis->range().upper;
    int chnum = m_ringChannelCount;
    const int MAX_DISPLAY_POINTS = 10000;
    int startSample = qMax(0, (int)(xMin / dt));
    int endSample = (int)(xMax / dt) + 1;
    endSample = qMin(endSample, (int)totalWritten);
    if (totalWritten > capacity) startSample = qMax(startSample, (int)(totalWritten - capacity));
    int visiblePoints = endSample - startSample;
    if (visiblePoints <= 0) return;
    // 填充阶段：步长基于固定容量，避免抖动；滚动阶段：步长基于可视范围
    int step;
    if (totalWritten <= capacity) {
        step = (capacity + MAX_DISPLAY_POINTS - 1) / MAX_DISPLAY_POINTS;
    } else {
        step = (visiblePoints + MAX_DISPLAY_POINTS - 1) / MAX_DISPLAY_POINTS;
    }
    if (step < 1) step = 1;

    int dataEnd = (totalWritten <= capacity) ? (int)(xMax / dt) + 1 : endSample;
    int nDispPoints = (dataEnd - startSample + step - 1) / step;
    if (nDispPoints <= 0) return;

    QVector<double> xDisp, yDisp;
    xDisp.reserve(nDispPoints);
    yDisp.reserve(nDispPoints);

    for (int ch = 0; ch < chnum; ++ch) {
        if (!m_plot->graph(ch)->visible()) continue;
        xDisp.clear();
        yDisp.clear();
        const float* buf = m_ringBuffers[ch].data.constData();
        double offset = (ch < m_chOffsets.size()) ? m_chOffsets[ch] : 0.0;
        int s = startSample;
        for (int i = 0; i < nDispPoints; ++i, s += step) {
            if (s < endSample) {
                int physIdx = s % capacity;
                xDisp.append(s * dt);
                yDisp.append((double)buf[physIdx] + offset);
            } else {
                xDisp.append(s * dt);
                yDisp.append(0.0);
            }
        }
        m_plot->graph(ch)->setData(xDisp, yDisp);
    }

    if (totalWritten <= capacity) {
        if (m_fillYMaxAbs <= 0) {
            double maxAbs = 0;
            for (int ch = 0; ch < chnum; ++ch) {
                if (!m_plot->graph(ch)->visible()) continue;
                const float* buf = m_ringBuffers[ch].data.constData();
                double offset = (ch < m_chOffsets.size()) ? m_chOffsets[ch] : 0.0;
                for (int s = startSample; s < endSample; s += step) {
                    double a = qAbs((double)buf[s % capacity] + offset);
                    if (a > maxAbs) maxAbs = a;
                }
            }
            if (maxAbs < 1e-9) maxAbs = 1.0;
            m_fillYMaxAbs = maxAbs;
            m_plot->yAxis->setRange(-maxAbs, maxAbs);
        }
    } else {
        double maxAbs = 0;
        for (int ch = 0; ch < chnum; ++ch) {
            if (!m_plot->graph(ch)->visible()) continue;
            const float* buf = m_ringBuffers[ch].data.constData();
            double offset = (ch < m_chOffsets.size()) ? m_chOffsets[ch] : 0.0;
            for (int s = startSample; s < endSample; s += step) {
                double a = qAbs((double)buf[s % capacity] + offset);
                if (a > maxAbs) maxAbs = a;
            }
        }
        if (maxAbs < 1e-9) maxAbs = 1.0;
        m_plot->yAxis->setRange(-maxAbs, maxAbs);
    }

    updateWindowIndicator();
    if (m_trigPos >= 0) {
        double trigTime = m_trigPos * dt;
        m_trigLine->point1->setCoords(trigTime, -1e9);
        m_trigLine->point2->setCoords(trigTime, 1e9);
        m_trigLine->setVisible(true);
    } else {
        m_trigLine->setVisible(false);
    }
    m_plot->replot();
}

void DevicePanel::updateWindowIndicator()
{
    if (m_ringChannelCount <= 0 || m_ringBuffers.empty()) return;
    int capacity = m_ringBuffers[0].capacity;
    qint64 totalWritten = m_ringBuffers[0].totalWritten;
    if (totalWritten <= 0) return;
    int writePos = m_ringBuffers[0].writePos;
    int indicatorCh = m_indicatorChCombo ? m_indicatorChCombo->currentIndex() : 0;
    if (indicatorCh < 0 || indicatorCh >= (int)m_ringBuffers.size()) indicatorCh = 0;
    double dt = 1.0 / m_sampleFreq;
    m_windowIndicator->setFullRange(0, capacity * dt);
    double xMin = m_plot->xAxis->range().lower;
    double xMax = m_plot->xAxis->range().upper;
    if (totalWritten > capacity) {
        double offset = (totalWritten - capacity) * dt;
        m_windowIndicator->setVisibleRange(xMin - offset, xMax - offset);
    } else {
        m_windowIndicator->setVisibleRange(xMin, xMax);
    }
    const float* buf = m_ringBuffers[indicatorCh].data.constData();
    int indicatorStep = qMax(1, capacity / 2000);
    int nIndicatorPoints = (capacity + indicatorStep - 1) / indicatorStep;
    QVector<double> xInd, yInd;
    xInd.reserve(nIndicatorPoints);
    yInd.reserve(nIndicatorPoints);
    if (totalWritten <= capacity) {
        int validCount = (int)qMin(totalWritten, (qint64)capacity);
        for (int i = 0; i < nIndicatorPoints; ++i) {
            int d = i * indicatorStep;
            xInd.append(d * dt);
            yInd.append(d < validCount ? (double)buf[d] : 0.0);
        }
    } else {
        int oldestPhysIdx = writePos;
        for (int i = 0; i < nIndicatorPoints; ++i) {
            int d = i * indicatorStep;
            if (d < capacity) {
                xInd.append(d * dt);
                yInd.append((double)buf[(oldestPhysIdx + d) % capacity]);
            }
        }
    }
    m_windowIndicator->setWaveformData(xInd, yInd);
}

void DevicePanel::onAxisRangeChanged()
{
    if (m_autoScrolling) return;
    m_lastUserInteraction = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    std::lock_guard<std::mutex> lock(m_collectMtx);
    if (m_ringChannelCount <= 0 || m_ringBuffers.empty()) return;
    if (m_ringBuffers[0].totalWritten <= 0) return;
    int capacity = m_ringBuffers[0].capacity;
    qint64 totalWritten = m_ringBuffers[0].totalWritten;
    double xMin = m_plot->xAxis->range().lower;
    double xMax = m_plot->xAxis->range().upper;
    double visibleRange = xMax - xMin;
    double dt = 1.0 / m_sampleFreq;
    double dataStart = (totalWritten <= capacity) ? 0 : (totalWritten - capacity) * dt;
    double dataEnd = (totalWritten <= capacity) ? capacity * dt : totalWritten * dt;
    double fullRange = dataEnd - dataStart;
    if (visibleRange > fullRange) {
        m_plot->xAxis->blockSignals(true);
        m_plot->xAxis->setRange(dataStart, dataEnd);
        m_plot->xAxis->blockSignals(false);
    } else if (visibleRange < 0.001) {
        double center = (xMin + xMax) / 2.0;
        m_plot->xAxis->blockSignals(true);
        m_plot->xAxis->setRange(center - 0.0005, center + 0.0005);
        m_plot->xAxis->blockSignals(false);
    }
    updatePlotFromCache();
}

void DevicePanel::onStopCollect()
{
    {
        std::lock_guard<std::mutex> lock(m_collectMtx);
        if (!m_collecting) return;
        m_collecting = false;
    }
    collect_send_stopreq(m_hdev);
    m_btnStop->setEnabled(false);
    m_labelStatus->setText("Stopped");
}

void DevicePanel::updateUI()
{
    bool canCollect = !m_collecting;
    m_btnInstant->setEnabled(canCollect);
    m_btnContinuous->setEnabled(canCollect);
    bool hasTrig = hasTrigCapability();
    m_btnSingleTrig->setEnabled(canCollect && hasTrig);
    m_btnMultiTrig->setEnabled(canCollect && hasTrig);
    m_btnStop->setEnabled(m_collecting);
    m_btnParams->setEnabled(!m_collecting);
    bool hasData = !m_collecting && !m_ringBuffers.empty() && m_ringBuffers[0].totalWritten > 0;
    m_btnSaveData->setEnabled(hasData);
}

void DevicePanel::appendLog(const QString &msg)
{
    m_labelStatus->setText(msg);
}

void DevicePanel::onSaveData()
{
    if (m_ringBuffers.empty() || m_ringBuffers[0].totalWritten <= 0) return;
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString defaultName = QString("%1_%2.csv").arg(m_filePrefix).arg(timestamp);
    QString filePath = QFileDialog::getSaveFileName(this, T("保存采集数据"),
        QDir::homePath() + "/" + defaultName, T("CSV 文件 (*.csv)"));
    if (filePath.isEmpty()) return;

    int chnum = 0;
    qint64 totalSamples = 0;
    int sampleRate = m_sampleFreq;
    QVector<QVector<float>> channelSnapshots;
    QVector<int> enabledChs;
    {
        std::lock_guard<std::mutex> lock(m_collectMtx);
        chnum = m_ringChannelCount;
        if (chnum <= 0 || m_ringBuffers.empty()) return;
        totalSamples = m_ringBuffers[0].totalWritten;
        for (int ch = 0; ch < chnum; ++ch)
            if (ch < m_chEnable.size() && m_chEnable[ch]) enabledChs.append(ch);
        if (enabledChs.isEmpty()) return;
        qint64 startSample = 0;
        int capacity = m_ringBuffers[0].capacity;
        if (m_collectMode == 3 && totalSamples > capacity) startSample = totalSamples - capacity;
        int saveSamples = (int)qMin(totalSamples - startSample, (qint64)capacity);
        channelSnapshots.resize(enabledChs.size());
        for (int ei = 0; ei < enabledChs.size(); ++ei) {
            int ch = enabledChs[ei];
            channelSnapshots[ei].resize(saveSamples);
            int oldestPhysIdx = (m_collectMode == 3 && totalSamples > capacity) ? m_ringBuffers[ch].writePos : 0;
            for (int i = 0; i < saveSamples; ++i)
                channelSnapshots[ei][i] = m_ringBuffers[ch].data[(oldestPhysIdx + i) % capacity];
        }
        totalSamples = saveSamples;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, T("错误"), T("无法创建文件: %1").arg(filePath));
        return;
    }
    QTextStream out(&file);
    out.setRealNumberPrecision(6);
    QString modeNames[] = {"Instant", "SingleTrig", "MultiTrig", "Continuous"};
    out << "# MStar Oscilloscope Data" << "\n";
    out << "# MAC = " << m_deviceMac << "\n";
    out << "# Version = V" << m_collectInfo.main_version << "." << m_collectInfo.minor_version << "\n";
    out << "# SampleRate = " << sampleRate << "\n";
    out << "# Channels = " << channelSnapshots.size() << "\n";
    out << "# Samples = " << totalSamples << "\n";
    out << "# CollectMode = " << modeNames[m_collectMode] << "\n";
    out << "# Date = " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    for (int ei = 0; ei < enabledChs.size(); ++ei) {
        if (ei > 0) out << ",";
        out << "CH" << (enabledChs[ei] + 1) << "(V)";
    }
    out << "\n";
    for (int i = 0; i < (int)totalSamples; ++i) {
        for (int ei = 0; ei < channelSnapshots.size(); ++ei) {
            if (ei > 0) out << ",";
            out << channelSnapshots[ei][i];
        }
        out << "\n";
    }
    file.close();
    appendLog(T("数据已保存: %1 (%2 采样点)").arg(filePath).arg(totalSamples));
}

void DevicePanel::autoSaveData()
{
    if (m_ringBuffers.empty() || m_ringBuffers[0].totalWritten <= 0) return;
    if (m_savePath.isEmpty() || m_filePrefix.isEmpty()) return;
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString fileName = QString("%1_%2.csv").arg(m_filePrefix).arg(timestamp);
    QString filePath = m_savePath + "/" + fileName;
    int chnum = 0;
    qint64 totalSamples = 0;
    int sampleRate = m_sampleFreq;
    QVector<QVector<float>> channelSnapshots;
    QVector<int> enabledChs;
    {
        std::lock_guard<std::mutex> lock(m_collectMtx);
        chnum = m_ringChannelCount;
        if (chnum <= 0 || m_ringBuffers.empty()) return;
        totalSamples = m_ringBuffers[0].totalWritten;
        for (int ch = 0; ch < chnum; ++ch)
            if (ch < m_chEnable.size() && m_chEnable[ch]) enabledChs.append(ch);
        if (enabledChs.isEmpty()) return;
        qint64 startSample = 0;
        int capacity = m_ringBuffers[0].capacity;
        if (m_collectMode == 3 && totalSamples > capacity) startSample = totalSamples - capacity;
        int saveSamples = (int)qMin(totalSamples - startSample, (qint64)capacity);
        channelSnapshots.resize(enabledChs.size());
        for (int ei = 0; ei < enabledChs.size(); ++ei) {
            int ch = enabledChs[ei];
            channelSnapshots[ei].resize(saveSamples);
            int oldestPhysIdx = (m_collectMode == 3 && totalSamples > capacity) ? m_ringBuffers[ch].writePos : 0;
            for (int i = 0; i < saveSamples; ++i)
                channelSnapshots[ei][i] = m_ringBuffers[ch].data[(oldestPhysIdx + i) % capacity];
        }
        totalSamples = saveSamples;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        appendLog(T("自动保存失败: 无法创建文件 %1").arg(filePath));
        m_saveEnabled = false;
        return;
    }
    QTextStream out(&file);
    out.setRealNumberPrecision(6);
    QString modeNames[] = {"Instant", "SingleTrig", "MultiTrig", "Continuous"};
    out << "# MStar Oscilloscope Data" << "\n";
    out << "# MAC = " << m_deviceMac << "\n";
    out << "# Version = V" << m_collectInfo.main_version << "." << m_collectInfo.minor_version << "\n";
    out << "# SampleRate = " << sampleRate << "\n";
    out << "# Channels = " << channelSnapshots.size() << "\n";
    out << "# Samples = " << totalSamples << "\n";
    out << "# CollectMode = " << modeNames[m_collectMode] << "\n";
    out << "# Date = " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    for (int ei = 0; ei < enabledChs.size(); ++ei) {
        if (ei > 0) out << ",";
        out << "CH" << (enabledChs[ei] + 1) << "(V)";
    }
    out << "\n";
    for (int i = 0; i < (int)totalSamples; ++i) {
        for (int ei = 0; ei < channelSnapshots.size(); ++ei) {
            if (ei > 0) out << ",";
            out << channelSnapshots[ei][i];
        }
        out << "\n";
    }
    file.close();
    m_saveEnabled = false;
    appendLog(T("数据已自动保存: %1 (%2 采样点)").arg(filePath).arg(totalSamples));
}

void DevicePanel::onParamSettings()
{
    if (!m_paramDialog || !m_hdev) return;
    if (m_collecting) {
        QMessageBox::warning(this, T("提示"), T("采集进行中，无法设置参数。请先停止采集。"));
        return;
    }
    m_paramDialog->setSampleFreq(m_sampleFreq);
    m_paramDialog->setBufLen(m_bufLen);
    m_paramDialog->setPreTrigger(m_preTrigger);
    m_paramDialog->setOversample(m_oversample);
    m_paramDialog->setConvClkOut(m_convClkOut);
    m_paramDialog->setConvClkIn(m_convClkIn);
    m_paramDialog->setTrigOut(m_trigOut);
    m_paramDialog->setTrigIn(m_trigIn);
    m_paramDialog->setMasterIp(m_masterIp);
    if (m_sampleFreq > 0) {
        int seconds = m_hostBufSamples / m_sampleFreq;
        m_paramDialog->setHostBufSeconds(seconds);
    }
    if (!m_chEnable.isEmpty())  m_paramDialog->setChannelEnable(m_chEnable);
    if (!m_trigEnable.isEmpty()) m_paramDialog->setTrigEnable(m_trigEnable);
    if (!m_trigType.isEmpty())  m_paramDialog->setTrigType(m_trigType);
    if (!m_trigValue.isEmpty()) m_paramDialog->setTrigValue(m_trigValue);
    m_paramDialog->setSaveEnable(m_saveEnabled);
    m_paramDialog->setSavePath(m_savePath);
    m_paramDialog->setFilePrefix(m_filePrefix);
    m_paramDialog->setFrameDelay(m_frameDelay);

    if (m_paramDialog->exec() == QDialog::Accepted) {
        m_sampleFreq = m_paramDialog->sampleFreq();
        m_bufLen = m_paramDialog->bufLen();
        m_hostBufSamples = m_paramDialog->hostBufSamples();
        m_preTrigger = m_paramDialog->preTrigger();
        m_oversample = m_paramDialog->oversample();
        m_convClkOut = m_paramDialog->convClkOut();
        m_convClkIn  = m_paramDialog->convClkIn();
        m_trigOut    = m_paramDialog->trigOut();
        m_trigIn     = m_paramDialog->trigIn();
        m_masterIp  = m_paramDialog->masterIp();
        m_chEnable = m_paramDialog->channelEnable();
        m_trigEnable = m_paramDialog->trigEnable();
        m_trigType = m_paramDialog->trigType();
        m_trigValue = m_paramDialog->trigValue();
        m_saveEnabled = m_paramDialog->saveEnable();
        m_savePath = m_paramDialog->savePath();
        m_filePrefix = m_paramDialog->filePrefix();
        m_frameDelay = m_paramDialog->frameDelay();
        uint32_t freq = set_collect_samplefreq(m_hdev, m_sampleFreq);
        m_sampleFreq = freq;
        m_paramDialog->setSampleFreq(m_sampleFreq);
        QVector<int16_t> offsets = m_paramDialog->channelOffset();
        if (!offsets.isEmpty()) set_collect_offset(m_hdev, offsets.data(), offsets.size());
        // 设置量程后回读实际值（不同AD芯片实际值可能与设置值有差异）
        QVector<uint8_t> ranges = m_paramDialog->channelRange();
        if (!ranges.isEmpty()) {
            set_collect_range(m_hdev, ranges.data(), ranges.size());
            QVector<uint8_t> readBackRanges(ranges.size());
            if (get_collect_range(m_hdev, readBackRanges.data(), readBackRanges.size()) > 0) {
                m_paramDialog->setChannelRange(readBackRanges);
            }
        }
        // 设置带宽后回读实际值（不同AD芯片实际值可能与设置值有差异）
        QVector<uint8_t> bw = m_paramDialog->channelBandwidth();
        if (!bw.isEmpty()) {
            set_bandwidth(m_hdev, bw.data(), bw.size());
            QVector<uint8_t> readBackBw(bw.size());
            if (get_bandwidth(m_hdev, readBackBw.data(), readBackBw.size()) > 0) {
                m_paramDialog->setChannelBandwidth(readBackBw);
            }
        }
        set_collect_oversample(m_hdev, m_oversample);
        set_collect_convclkout(m_hdev, m_convClkOut);
        set_collect_convclkin(m_hdev, m_convClkIn);
        set_collect_trigout(m_hdev, m_trigOut);
        set_collect_trigin(m_hdev, m_trigIn);
        set_framdelay(m_hdev, m_frameDelay);
        updateUI();
    }
}

void DevicePanel::saveDeviceProfile()
{
    if (m_deviceMac.isEmpty() || !m_paramDialog) return;
    QVariantMap profile;
    // 保存前校验：采样频率必须 > 0
    profile["sampleFreq"]     = (m_sampleFreq > 0) ? m_sampleFreq : 1000000;
    profile["bufLen"]         = m_bufLen;
    profile["hostBufSeconds"] = m_paramDialog->hostBufSeconds();
    profile["preTrigger"]     = m_preTrigger;
    profile["oversample"]     = (int)m_oversample;
    profile["convClkOut"] = m_convClkOut;
    profile["convClkIn"]  = m_convClkIn;
    profile["trigOut"]    = m_trigOut;
    profile["trigIn"]     = m_trigIn;
    profile["masterIp"]  = m_masterIp;
    auto toVL = [](const QVector<bool> &v) { QVariantList l; for (bool b : v) l.append(b); return l; };
    auto toVLU32 = [](const QVector<uint32_t> &v) { QVariantList l; for (uint32_t u : v) l.append((int)u); return l; };
    auto toVLD = [](const QVector<double> &v) { QVariantList l; for (double d : v) l.append(d); return l; };
    auto toVLU8 = [](const QVector<uint8_t> &v) { QVariantList l; for (uint8_t u : v) l.append((int)u); return l; };
    auto toVLI16 = [](const QVector<int16_t> &v) { QVariantList l; for (int16_t s : v) l.append((int)s); return l; };
    profile["channelEnable"] = toVL(m_chEnable);
    profile["trigEnable"]    = toVL(m_trigEnable);
    profile["trigType"]      = toVLU32(m_trigType);
    profile["trigValue"]     = toVLD(m_trigValue);
    profile["channelOffset"] = toVLD(m_chOffsets);
    profile["calibOffset"] = toVLI16(m_paramDialog->channelOffsetAD());
    profile["channelRange"] = toVLU8(m_paramDialog->channelRange());
    profile["channelBandwidth"] = toVLU8(m_paramDialog->channelBandwidth());
    profile["saveEnable"]  = m_saveEnabled;
    profile["savePath"]    = m_savePath;
    profile["filePrefix"]  = m_filePrefix;
    profile["frameDelay"]  = (int)m_frameDelay;
    m_profileMgr.saveProfile(m_deviceMac, profile);
}

void DevicePanel::loadDeviceProfile()
{
    if (m_deviceMac.isEmpty() || !m_paramDialog) return;
    QVariantMap profile;
    if (!m_profileMgr.loadProfile(m_deviceMac, profile)) return;
    int nch = m_collectInfo.nchannel;
    m_sampleFreq     = profile.value("sampleFreq", m_sampleFreq).toInt();
    // 参数有效性校验：采样频率必须 > 0
    if (m_sampleFreq <= 0) m_sampleFreq = 1000000;
    m_bufLen         = profile.value("bufLen", m_bufLen).toInt();
    int hostSec      = profile.value("hostBufSeconds", 10).toInt();
    m_preTrigger     = profile.value("preTrigger", m_preTrigger).toInt();
    m_oversample     = (uint8_t)profile.value("oversample", (int)m_oversample).toInt();
    m_convClkOut = profile.value("convClkOut", false).toBool();
    m_convClkIn  = profile.value("convClkIn", false).toBool();
    m_trigOut    = profile.value("trigOut", false).toBool();
    m_trigIn     = profile.value("trigIn", false).toBool();
    m_masterIp  = profile.value("masterIp", QString()).toString();
    m_paramDialog->setSampleFreq(m_sampleFreq);
    m_paramDialog->setBufLen(m_bufLen);
    m_paramDialog->setHostBufSeconds(hostSec);
    m_paramDialog->setPreTrigger(m_preTrigger);
    m_paramDialog->setOversample(m_oversample);
    m_paramDialog->setConvClkOut(m_convClkOut);
    m_paramDialog->setConvClkIn(m_convClkIn);
    m_paramDialog->setTrigOut(m_trigOut);
    m_paramDialog->setTrigIn(m_trigIn);
    m_paramDialog->setMasterIp(m_masterIp);
    m_hostBufSamples = m_paramDialog->hostBufSamples();
    auto loadBV = [&](const QString &k, QVector<bool> &v) { auto l = profile.value(k).toList(); if (l.isEmpty()) return; v.clear(); for (int i = 0; i < qMin(l.size(), nch); ++i) v.append(l[i].toBool()); };
    auto loadU32V = [&](const QString &k, QVector<uint32_t> &v) { auto l = profile.value(k).toList(); if (l.isEmpty()) return; v.clear(); for (int i = 0; i < qMin(l.size(), nch); ++i) v.append(l[i].toUInt()); };
    auto loadDV = [&](const QString &k) -> QVector<double> { QVector<double> v; auto l = profile.value(k).toList(); for (int i = 0; i < qMin(l.size(), nch); ++i) v.append(l[i].toDouble()); return v; };
    auto loadU8V = [&](const QString &k) -> QVector<uint8_t> { QVector<uint8_t> v; auto l = profile.value(k).toList(); for (int i = 0; i < qMin(l.size(), nch); ++i) v.append((uint8_t)l[i].toUInt()); return v; };
    auto loadI16V = [&](const QString &k) -> QVector<int16_t> { QVector<int16_t> v; auto l = profile.value(k).toList(); for (int i = 0; i < qMin(l.size(), nch); ++i) v.append((int16_t)l[i].toInt()); return v; };
    loadBV("channelEnable", m_chEnable);
    loadBV("trigEnable", m_trigEnable);
    loadU32V("trigType", m_trigType);
    { auto l = profile.value("trigValue").toList(); if (!l.isEmpty()) { m_trigValue.clear(); for (int i = 0; i < qMin(l.size(), nch); ++i) m_trigValue.append(l[i].toDouble()); } }
    QVector<double> savedOffsets = loadDV("channelOffset");
    for (int ch = 0; ch < qMin(savedOffsets.size(), m_chOffsetEdits.size()); ++ch) {
        m_chOffsets[ch] = savedOffsets[ch];
        m_chOffsetEdits[ch]->blockSignals(true);
        m_chOffsetEdits[ch]->setValue(savedOffsets[ch]);
        m_chOffsetEdits[ch]->blockSignals(false);
    }
    QVector<int16_t> savedCalib = loadI16V("calibOffset");
    if (!savedCalib.isEmpty()) m_paramDialog->setChannelOffsetAD(savedCalib);
    QVector<uint8_t> savedRanges = loadU8V("channelRange");
    if (!savedRanges.isEmpty()) {
        set_collect_range(m_hdev, savedRanges.data(), savedRanges.size());
        // 回读实际量程值（不同AD芯片实际值可能与设置值有差异）
        QVector<uint8_t> readBackRanges(savedRanges.size());
        if (get_collect_range(m_hdev, readBackRanges.data(), readBackRanges.size()) > 0) {
            m_paramDialog->setChannelRange(readBackRanges);
        } else {
            m_paramDialog->setChannelRange(savedRanges);
        }
    }
    QVector<uint8_t> savedBw = loadU8V("channelBandwidth");
    if (!savedBw.isEmpty()) {
        set_bandwidth(m_hdev, savedBw.data(), savedBw.size());
        // 回读实际带宽值（不同AD芯片实际值可能与设置值有差异）
        QVector<uint8_t> readBackBw(savedBw.size());
        if (get_bandwidth(m_hdev, readBackBw.data(), readBackBw.size()) > 0) {
            m_paramDialog->setChannelBandwidth(readBackBw);
        } else {
            m_paramDialog->setChannelBandwidth(savedBw);
        }
    }
    uint32_t freq = set_collect_samplefreq(m_hdev, m_sampleFreq);
    if (freq > 0) m_sampleFreq = freq;  // 返回值无效时保持原值
    m_paramDialog->setSampleFreq(m_sampleFreq);
    set_collect_oversample(m_hdev, m_oversample);
    QVector<int16_t> calibRaw = m_paramDialog->channelOffset();
    if (!calibRaw.isEmpty()) set_collect_offset(m_hdev, calibRaw.data(), calibRaw.size());
    for (int ch = 0; ch < m_chEnable.size() && ch < m_plot->graphCount(); ++ch)
        m_plot->graph(ch)->setVisible(m_chEnable[ch]);
    m_plot->replot();
    m_saveEnabled  = profile.value("saveEnable", m_saveEnabled).toBool();
    m_savePath     = profile.value("savePath", m_savePath).toString();
    m_filePrefix   = profile.value("filePrefix", m_filePrefix).toString();
    m_frameDelay   = (uint32_t)profile.value("frameDelay", (int)m_frameDelay).toInt();
    if (m_paramDialog) {
        m_paramDialog->setSaveEnable(m_saveEnabled);
        m_paramDialog->setSavePath(m_savePath);
        m_paramDialog->setFilePrefix(m_filePrefix);
        m_paramDialog->setFrameDelay(m_frameDelay);
    }
    if (m_hdev) set_framdelay(m_hdev, m_frameDelay);
    updateUI();
}
