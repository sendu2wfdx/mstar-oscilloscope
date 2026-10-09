// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef DEVICEPROFILEMANAGER_H
#define DEVICEPROFILEMANAGER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariantMap>
#include <QString>
#include <QVector>

/**
 * @brief 设备参数配置管理器
 *
 * 以设备 MAC 地址为键，将采集参数保存为 JSON 文件。
 * 下次连接同一设备时自动加载已保存的参数。
 *
 * JSON 文件结构：
 * {
 *   "version": 1,
 *   "devices": {
 *     "aa:bb:cc:dd:ee:ff": {
 *       "sampleFreq": 1000000,
 *       "bufLen": 16384,
 *       "hostBufSeconds": 10,
 *       "preTrigger": 0,
 *       "oversample": 0,
 *       "frameDelay": 0,
 *       "channelEnable": [true, true, ...],
 *       "trigEnable":    [false, false, ...],
 *       "trigType":      [0, 0, ...],
 *       "trigValue":     [0.0, 0.0, ...],
 *       "channelOffset": [0.0, 0.0, ...],
 *       "calibOffset":   [0.0, 0.0, ...],
 *       "channelRange":  [5, 5, ...]
 *     }
 *   }
 * }
 */
class DeviceProfileManager
{
public:
    DeviceProfileManager();

    /// 加载指定 MAC 地址的设备参数，成功返回 true
    bool loadProfile(const QString &mac, QVariantMap &profile) const;

    /// 保存指定 MAC 地址的设备参数
    void saveProfile(const QString &mac, const QVariantMap &profile);

    /// 获取配置文件完整路径
    QString configFilePath() const;

private:
    QJsonObject loadJson() const;
    void saveJson(const QJsonObject &root) const;

    static QJsonObject profileToJson(const QVariantMap &profile);
    static QVariantMap profileFromJson(const QJsonObject &obj);
};

#endif // DEVICEPROFILEMANAGER_H
