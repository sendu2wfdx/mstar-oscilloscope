// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef DATAFILEWRITER_H
#define DATAFILEWRITER_H

#include <QString>
#include <QVector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <functional>
#include <cstdio>

/**
 * @brief 实时数据文件写入器
 *
 * 后台线程持续从有界队列中取出数据批次，写入 CSV 文件。
 * 支持自动分片（每文件最大 maxSamplesPerFile 个采样点），
 * 确保 Excel 可秒开、MATLAB 可方便读取。
 *
 * 线程安全：enqueue() 可从采集线程调用，start()/stop() 必须在主线程调用。
 */
class DataFileWriter
{
public:
    /// 写入速度跟不上时主线程回调（从写入线程通过 Qt 事件队列触发）
    using WriteSpeedCallback = std::function<void()>;

    DataFileWriter();
    ~DataFileWriter();

    /// 启动写入：打开第一个文件，启动后台线程
    void start(const QString &savePath, const QString &filePrefix,
               int sampleRate, int channelCount, const QVector<int> &enabledChannels,
               const QString &mac, const QString &collectMode,
               int maxSamplesPerFile = 100000);

    /// 入队一批数据（采集线程调用）
    /// data[ch][i] = 第 ch 个使能通道的第 i 个电压值
    void enqueue(const QVector<QVector<float>> &data);

    /// 停止写入：等待队列排空后关闭文件、join 线程
    void stop();

    /// 已写入的总采样点数
    qint64 totalWritten() const { return m_totalWritten.load(); }

    /// 当前队列中的批次数（调试用）
    int queueSize() const;

    /// 设置写入速度告警回调（队列积压超过阈值时触发，仅触发一次）
    void setWriteSpeedCallback(WriteSpeedCallback cb);

private:
    void writerLoop();
    bool openNextFile();
    void closeCurrentFile();
    void writeBatch(const QVector<QVector<float>> &data);

    // 配置（start 时设置，之后只读）
    QString m_savePath;
    QString m_filePrefix;
    int m_sampleRate = 0;
    int m_channelCount = 0;          // 使能通道数
    QVector<int> m_enabledChannels;  // 使能通道号列表
    QString m_mac;
    QString m_collectMode;
    int m_maxSamplesPerFile = 100000;

    // 有界队列
    static constexpr int MAX_QUEUE_BATCHES = 64;
    std::queue<QVector<QVector<float>>> m_queue;
    std::mutex m_queueMtx;
    std::condition_variable m_queueCv;      // 通知 writer 有新数据或停止
    std::condition_variable m_queueFullCv;  // 通知 enqueue 队列有空间

    // 写入线程
    std::thread m_writerThread;
    std::atomic<bool> m_stopFlag{false};

    // 当前文件（使用 C stdio 高性能 I/O）
    FILE *m_fp = nullptr;
    int m_fileIndex = 0;
    int m_samplesInFile = 0;
    qint64 m_globalSampleOffset = 0;  // 全局采样点偏移（用于时间戳计算）
    std::atomic<qint64> m_totalWritten{0};

    // 性能监控
    WriteSpeedCallback m_speedCallback;
    std::atomic<bool> m_warned{false};   // 已触发告警（仅触发一次）
    int m_writeBatchCount = 0;           // 已写入批次数
    double m_totalWriteMs = 0.0;         // 累计写入耗时 (ms)
};

#endif // DATAFILEWRITER_H
