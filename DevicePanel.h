// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef DEVICEPANEL_H
#define DEVICEPANEL_H

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QVector>
#include <QtGlobal>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include "qcustomplot.h"
#include "WindowIndicator.h"
#include "DeviceProfileManager.h"
#include "MstarEthBridge.h"
#include "MstarCollect.h"

class CollectParamDialog;
class DataFileWriter;
class QDoubleSpinBox;

// 单通道环形波形缓存
struct ChannelRingBuffer {
    QVector<float> data;      // 环形数据容器（电压值）
    int capacity = 0;         // 缓存容量（采样点数）
    int writePos = 0;         // 写指针 (0 ~ capacity-1)
    qint64 totalWritten = 0;  // 累计写入计数（用于计算全局索引）

    void reset() { writePos = 0; totalWritten = 0; }
    void allocate(int cap) { capacity = cap; data.resize(cap); reset(); }
};

// 单设备自包含面板：采集按钮 + 波形图 + 显示使能 + 所有 per-device 状态
class DevicePanel : public QWidget
{
    Q_OBJECT

public:
    explicit DevicePanel(const QString &ip, MHANDLE hdev,
                         collectinforst info, int netSpeedMbps,
                         QWidget *parent = nullptr);
    ~DevicePanel();

    // 设备标识
    QString deviceMac() const { return m_deviceMac; }
    QString deviceIp() const { return m_deviceIp; }
    QString deviceModel() const;  // 从 collectInfo 推导
    MHANDLE hdev() const { return m_hdev; }
    QString masterIp() const { return m_masterIp; }
    void setDeviceMac(const QString &mac) { m_deviceMac = mac; }
    void setDeviceInfoText(const QString &text) { m_labelDevInfo->setText(text); }

    // 采集控制
    bool hasAnyTrigEnable() const;
    bool hasTrigCapability() const;  // m_trigIn 或 m_trigEnable 任一为 true
    void onInstantCollect();
    void onSingleTriggerCollect();
    void onMultiTriggerCollect();
    void onContinuousCollect();
    void onStopCollect();
    void onParamSettings();
    void onSaveData();

    // 状态查询
    bool isCollecting() const { return m_collecting; }
    bool isConnected() const { return true; }

    // 断开连接（停止采集 + 释放设备）
    void disconnectDevice();

    // 参数配置加载/保存
    void loadDeviceProfile();
    void saveDeviceProfile();

signals:
    void plotReady();
    void disconnected(const QString &mac);

private slots:
    void onPlotReady();
    void onAxisRangeChanged();

private:
    void initUI();
    void initDevice();  // 初始化设备（通道图形、参数等）

    void updateUI();
    void appendLog(const QString &msg);
    void updatePlotFromCache();
    void updateWindowIndicator();
    void startCollectCommon(bool useTrig, bool loop);
    void autoSaveData();

    // === UI 控件 ===
    // 采集控制按钮
    QPushButton *m_btnInstant;
    QPushButton *m_btnSingleTrig;
    QPushButton *m_btnMultiTrig;
    QPushButton *m_btnContinuous;
    QPushButton *m_btnStop;
    QPushButton *m_btnParams;
    QPushButton *m_btnSaveData;

    // 状态标签
    QLabel *m_labelDevInfo;
    QLabel *m_labelStatus;

    // 波形显示
    QCustomPlot *m_plot;
    QCPItemStraightLine *m_trigLine = nullptr;
    WindowIndicator *m_windowIndicator;
    QComboBox *m_indicatorChCombo = nullptr;
    QVBoxLayout *m_chLayout;

    // 采集参数对话框
    CollectParamDialog *m_paramDialog = nullptr;

    // === 采集参数 ===
    int m_sampleFreq = 1000000;
    int m_bufLen = 0;
    int m_hostBufSamples = 0;
    int m_preTrigger = 0;
    uint8_t m_oversample = 0;
    uint32_t m_frameDelay = 0;

    // 同步CLK / 触发信号 IO 使能
    bool m_convClkOut = false;
    bool m_convClkIn  = false;
    bool m_trigOut    = false;
    bool m_trigIn     = false;
    QString m_masterIp;

    QVector<bool> m_chEnable;
    QVector<bool> m_trigEnable;
    QVector<uint32_t> m_trigType;
    QVector<double> m_trigValue;

    // === DLL state ===
    MHANDLE m_hdev = nullptr;
    collectinforst m_collectInfo = {};
    QString m_deviceMac;
    QString m_deviceIp;
    int m_netSpeedMbps = 0;
    int m_collectMode = 0;
    int m_trigPos = -1;
    int m_displayPoints = 1024;

    // === 采集线程 ===
    std::thread m_collectThread;
    std::mutex m_collectMtx;
    bool m_collecting = false;

    // 用户交互追踪
    std::atomic<int64_t> m_lastUserInteraction{0};
    static constexpr int PLOT_INTERACTION_TIMEOUT = 10000;
    static constexpr int PLOT_MIN_UPDATE_INTERVAL = 500;
    bool m_autoScrolling = false;
    double m_fillYMaxAbs = 0;

    // === 环形波形缓存 ===
    std::vector<ChannelRingBuffer> m_ringBuffers;
    int m_ringChannelCount = 0;
    int m_plotNumSamples = 0;

    // 通道波形偏移
    QVector<double> m_chOffsets;
    QVector<float> m_chScales;

    // 设备参数保存
    DeviceProfileManager m_profileMgr;

    // === 实时保存 ===
    bool m_saveEnabled = false;
    QString m_savePath;
    QString m_filePrefix = "MCollectDat";
    DataFileWriter *m_fileWriter = nullptr;

    // 通道偏移输入控件
    QVector<QDoubleSpinBox*> m_chOffsetEdits;
};

#endif // DEVICEPANEL_H
