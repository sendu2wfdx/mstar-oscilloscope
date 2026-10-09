// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "DataFileWriter.h"
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <QMetaObject>
#include <chrono>
#include <cstdio>
#include <cstring>

DataFileWriter::DataFileWriter() = default;

DataFileWriter::~DataFileWriter()
{
    stop();
}

void DataFileWriter::start(const QString &savePath, const QString &filePrefix,
                           int sampleRate, int channelCount, const QVector<int> &enabledChannels,
                           const QString &mac, const QString &collectMode,
                           int maxSamplesPerFile)
{
    stop();  // 确保旧状态已清理

    m_savePath = savePath;
    m_filePrefix = filePrefix;
    m_sampleRate = sampleRate;
    m_channelCount = channelCount;
    m_enabledChannels = enabledChannels;
    m_mac = mac;
    m_collectMode = collectMode;
    m_maxSamplesPerFile = maxSamplesPerFile;

    m_fileIndex = 0;
    m_samplesInFile = 0;
    m_globalSampleOffset = 0;
    m_totalWritten.store(0);
    m_stopFlag.store(false);
    m_warned.store(false);
    m_writeBatchCount = 0;
    m_totalWriteMs = 0.0;

    // 清空队列
    {
        std::lock_guard<std::mutex> lock(m_queueMtx);
        while (!m_queue.empty()) m_queue.pop();
    }

    // 确保保存目录存在
    QDir dir(savePath);
    if (!dir.exists()) dir.mkpath(".");

    // 打开第一个文件
    if (!openNextFile()) return;

    // 启动写入线程
    m_writerThread = std::thread(&DataFileWriter::writerLoop, this);
}

void DataFileWriter::enqueue(const QVector<QVector<float>> &data)
{
    if (data.isEmpty() || data[0].isEmpty()) return;

    std::unique_lock<std::mutex> lock(m_queueMtx);
    // 有界队列：满时等待（背压）
    m_queueFullCv.wait(lock, [this]() {
        return (int)m_queue.size() < MAX_QUEUE_BATCHES || m_stopFlag.load();
    });
    if (m_stopFlag.load()) return;

    m_queue.push(data);
    lock.unlock();
    m_queueCv.notify_one();  // 通知写入线程
}

void DataFileWriter::stop()
{
    if (!m_writerThread.joinable()) {
        // 线程未运行，仅清理文件
        closeCurrentFile();
        return;
    }

    m_stopFlag.store(true);
    m_queueCv.notify_one();  // 唤醒写入线程

    m_writerThread.join();
    closeCurrentFile();

    // 清空残留队列
    {
        std::lock_guard<std::mutex> lock(m_queueMtx);
        while (!m_queue.empty()) m_queue.pop();
    }
    m_queueFullCv.notify_all();  // 唤醒可能在 wait 的 enqueue
}

int DataFileWriter::queueSize() const
{
    // 注意：非锁定快照，仅调试用
    return static_cast<int>(m_queue.size());
}

void DataFileWriter::setWriteSpeedCallback(WriteSpeedCallback cb)
{
    m_speedCallback = std::move(cb);
}

// ---- 私有方法 ----

void DataFileWriter::writerLoop()
{
    while (true) {
        QVector<QVector<float>> batch;

        {
            std::unique_lock<std::mutex> lock(m_queueMtx);
            m_queueCv.wait(lock, [this]() {
                return !m_queue.empty() || m_stopFlag.load();
            });

            if (!m_queue.empty()) {
                batch = m_queue.front();
                m_queue.pop();
            } else if (m_stopFlag.load()) {
                return;  // 队列已空且收到停止信号
            }
        }
        // 释放锁后通知 enqueue 有空位
        m_queueFullCv.notify_one();

        if (!batch.isEmpty()) {
            auto t0 = std::chrono::steady_clock::now();
            writeBatch(batch);
            auto t1 = std::chrono::steady_clock::now();
            double writeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

            // 性能监控：每 10 批检查一次队列深度和写入速度
            m_writeBatchCount++;
            m_totalWriteMs += writeMs;
            if (m_writeBatchCount % 10 == 0 && !m_warned.load()) {
                int qDepth;
                {
                    std::lock_guard<std::mutex> lock(m_queueMtx);
                    qDepth = static_cast<int>(m_queue.size());
                }
                // 队列深度超过 75% 或单次写入耗时超过 100ms，触发告警
                if (qDepth > MAX_QUEUE_BATCHES * 3 / 4 || writeMs > 100.0) {
                    m_warned.store(true);
                    if (m_speedCallback) {
                        // 通过 Qt 事件队列投递到主线程执行
                        auto cb = m_speedCallback;
                        QMetaObject::invokeMethod(QCoreApplication::instance(), cb, Qt::QueuedConnection);
                    }
                }
            }
        }
    }
}

bool DataFileWriter::openNextFile()
{
    closeCurrentFile();

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString fileName = QString("%1_%2_%3.csv")
                           .arg(m_filePrefix)
                           .arg(timestamp)
                           .arg(m_fileIndex, 3, 10, QChar('0'));
    QString filePath = m_savePath + "/" + fileName;

    m_fp = fopen(filePath.toLocal8Bit().constData(), "w");
    if (!m_fp) return false;

    // 元数据头
    double startTime = (m_sampleRate > 0)
                           ? static_cast<double>(m_globalSampleOffset) / m_sampleRate
                           : 0.0;
    QByteArray macBytes = m_mac.toLocal8Bit();
    QByteArray modeBytes = m_collectMode.toLocal8Bit();
    QByteArray dateBytes = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toLocal8Bit();

    fprintf(m_fp, "# MStar Oscilloscope Data\n");
    fprintf(m_fp, "# MAC = %s\n", macBytes.constData());
    fprintf(m_fp, "# SampleRate = %d\n", m_sampleRate);
    fprintf(m_fp, "# Channels = %d\n", m_channelCount);
    fprintf(m_fp, "# FileIndex = %d\n", m_fileIndex);
    fprintf(m_fp, "# FileStartTime = %.6f\n", startTime);
    fprintf(m_fp, "# CollectMode = %s\n", modeBytes.constData());
    fprintf(m_fp, "# Date = %s\n", dateBytes.constData());

    // 列头
    for (int i = 0; i < m_channelCount; ++i) {
        if (i > 0) fputc(',', m_fp);
        fprintf(m_fp, "CH%d(V)", m_enabledChannels[i] + 1);
    }
    fputc('\n', m_fp);

    m_samplesInFile = 0;
    m_fileIndex++;
    return true;
}

void DataFileWriter::closeCurrentFile()
{
    if (m_fp) {
        fflush(m_fp);
        fclose(m_fp);
        m_fp = nullptr;
    }
}

void DataFileWriter::writeBatch(const QVector<QVector<float>> &data)
{
    if (!m_fp || data.isEmpty()) return;

    int batchSamples = data[0].size();
    int chCount = m_channelCount;

    // 预分配行缓冲区（每通道约 16 字符 + 分隔符，加安全余量）
    int bufSize = chCount * 20 + 32;
    auto lineBuf = std::make_unique<char[]>(bufSize);

    for (int i = 0; i < batchSamples; ++i) {
        // 分片检查：当前文件已满，打开新文件
        if (m_samplesInFile >= m_maxSamplesPerFile) {
            if (!openNextFile()) return;
        }

        int pos = 0;

        // 各通道电压值
        for (int ch = 0; ch < chCount; ++ch) {
            if (ch > 0)
                lineBuf[pos++] = ',';
            if (ch < data.size() && i < data[ch].size())
                pos += snprintf(lineBuf.get() + pos, bufSize - pos, "%.6f", data[ch][i]);
        }
        lineBuf[pos++] = '\n';

        fwrite(lineBuf.get(), 1, pos, m_fp);

        m_globalSampleOffset++;
        m_samplesInFile++;
        m_totalWritten.fetch_add(1);
    }
}
