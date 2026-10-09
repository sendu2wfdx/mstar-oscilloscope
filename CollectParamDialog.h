// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef COLLECTPARAMDIALOG_H
#define COLLECTPARAMDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QVector>
#include "MstarEthBridge.h"
#include "MstarCollect.h"

/**
 * @brief 采集参数设置对话框
 *
 * 提供采样频率、缓冲区长度、通道使能、触发模式/值、偏移校准设置，
 * 由主窗口"参数设置"按钮弹出。
 */
class CollectParamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CollectParamDialog(QWidget *parent = nullptr);

    /// 根据设备信息初始化控件选项，并从设备读取当前设置
    void initFromDevice(MHANDLE hdev, const collectinforst &info);

    // --- 采样频率 ---
    int sampleFreq() const;
    void setSampleFreq(int freq);

    // --- 缓存（总字节数，UI显示为单通道采样点数） ---
    int bufLen() const;
    void setBufLen(int buflen);

    // --- 主机缓存（单通道采样点数） ---
    // 由时长换算：seconds × sampleFreq
    int hostBufSamples() const;
    int hostBufSeconds() const;            // 主机缓存时长（秒）
    void setHostBufSeconds(int seconds);   // 设置主机缓存时长（秒）

    // --- 预触发位置（单通道采样点数） ---
    int preTrigger() const;
    void setPreTrigger(int samples);

    // --- 过采样倍数 ---
    uint8_t oversample() const;
    void setOversample(uint8_t val);

    // --- 通道使能 ---
    QVector<bool> channelEnable() const;
    void setChannelEnable(const QVector<bool> &enables);

    // --- 触发（每通道） ---
    QVector<bool> trigEnable() const;
    QVector<uint32_t> trigType() const;
    QVector<double> trigValue() const;
    void setTrigEnable(const QVector<bool> &enables);
    void setTrigType(const QVector<uint32_t> &types);
    void setTrigValue(const QVector<double> &values);

    // --- 同步CLK / 触发信号 IO 使能 ---
    bool convClkOut() const;
    bool convClkIn() const;
    bool trigOut() const;
    bool trigIn() const;
    void setConvClkOut(bool en);
    void setConvClkIn(bool en);
    void setTrigOut(bool en);
    void setTrigIn(bool en);

    // --- 主设备 IP 地址（从设备使用） ---
    QString masterIp() const;
    void setMasterIp(const QString &ip);

    // --- 偏移校准（每通道，AD值） ---
    QVector<int16_t> channelOffset() const;
    QVector<int16_t> channelOffsetAD() const;  // 偏移校准AD值（用于保存配置）
    void setChannelOffsetAD(const QVector<int16_t> &adValues);  // 从配置恢复偏移AD值

    // --- 量程（每通道） ---
    QVector<uint8_t> channelRange() const;
    void setChannelRange(const QVector<uint8_t> &ranges);  // 从配置恢复量程

    // --- 输入带宽（每通道） ---
    QVector<uint8_t> channelBandwidth() const;
    void setChannelBandwidth(const QVector<uint8_t> &bw);

    // --- 实时保存 ---
    bool saveEnable() const;
    void setSaveEnable(bool en);
    QString savePath() const;
    void setSavePath(const QString &path);
    QString filePrefix() const;
    void setFilePrefix(const QString &prefix);

    // --- 数据包发送间隔 ---
    uint32_t frameDelay() const;
    void setFrameDelay(uint32_t delay);

private:
    void buildChannelTable(int nchannel, int nbits);

    // 基本参数
    QSpinBox *m_spinSampleFreq;
    QComboBox *m_comboBufLen;
    QLabel *m_labelBufLen;
    QComboBox *m_comboOversample;
    QComboBox *m_comboHostBuf;   // 主机缓存时长（秒）
    QLabel *m_labelHostBuf;
    QSpinBox *m_spinPreTrig;
    QSlider *m_sliderPreTrig;

    // 同步CLK / 触发IO 使能
    QComboBox *m_comboSyncMode;  // 同步模式：无级联/主设备(CLK输出)/从设备(CLK输入)
    QCheckBox *m_chkTrigOut;
    QCheckBox *m_chkTrigIn;

    // 主设备 IP 地址（从设备时显示）
    QLabel *m_labelMasterIp;
    QLineEdit *m_editMasterIp;

    // 实时保存
    QCheckBox *m_chkSaveEnable;
    QLineEdit *m_editSavePath;
    QLineEdit *m_editFilePrefix;

    // 数据包发送间隔
    QSpinBox *m_spinFrameDelay;

    int m_baseMaxSampleFreq = 100000000;  // 基础最高采样频率（无过采样时）
    int m_bytesPerSample = 2;             // 每采样点字节数
    int m_devMaxPerChSamples = 0;         // 设备最大缓存（单通道采样点数）

    // 通道列表
    int m_nchannel = 0;
    int m_nbits = 0;
    QVector<QCheckBox*> m_chEnableChecks;
    QVector<QCheckBox*> m_chTrigEnableChecks;
    QVector<QComboBox*> m_chTrigModes;
    QVector<QDoubleSpinBox*> m_chTrigValues;
    QVector<QSpinBox*> m_chOffsetSpins;
    QVector<QComboBox*> m_chRangeCombos;
    QVector<QComboBox*> m_chBandwidthCombos;

    MHANDLE m_hdev = nullptr;
};

#endif // COLLECTPARAMDIALOG_H
