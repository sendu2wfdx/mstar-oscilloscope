// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "DeviceProfileManager.h"
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonParseError>

DeviceProfileManager::DeviceProfileManager()
{
}

QString DeviceProfileManager::configFilePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/device_profiles.json";
}

QJsonObject DeviceProfileManager::loadJson() const
{
    QString path = configFilePath();
    QFile file(path);
    if (!file.exists()) return QJsonObject();

    if (!file.open(QIODevice::ReadOnly)) return QJsonObject();

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseErr);
    file.close();

    if (parseErr.error != QJsonParseError::NoError) return QJsonObject();
    return doc.object();
}

void DeviceProfileManager::saveJson(const QJsonObject &root) const
{
    QString path = configFilePath();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
}

bool DeviceProfileManager::loadProfile(const QString &mac, QVariantMap &profile) const
{
    if (mac.isEmpty()) return false;

    QJsonObject root = loadJson();
    QJsonObject devices = root.value("devices").toObject();
    QString key = mac.toLower().trimmed();

    if (!devices.contains(key)) return false;

    profile = profileFromJson(devices.value(key).toObject());
    return true;
}

void DeviceProfileManager::saveProfile(const QString &mac, const QVariantMap &profile)
{
    if (mac.isEmpty()) return;

    QJsonObject root = loadJson();
    QJsonObject devices = root.value("devices").toObject();
    QString key = mac.toLower().trimmed();

    devices[key] = profileToJson(profile);
    root["version"] = 1;
    root["devices"] = devices;

    saveJson(root);
}

// ---- JSON <-> QVariantMap 转换 ----

QJsonObject DeviceProfileManager::profileToJson(const QVariantMap &profile)
{
    QJsonObject obj;

    // 基本参数
    obj["sampleFreq"]     = profile.value("sampleFreq").toInt();
    obj["bufLen"]         = profile.value("bufLen").toInt();
    obj["hostBufSeconds"] = profile.value("hostBufSeconds").toInt();
    obj["preTrigger"]     = profile.value("preTrigger").toInt();
    obj["oversample"]     = profile.value("oversample").toInt();
    obj["frameDelay"]     = profile.value("frameDelay").toInt();

    // 通道数组 → QJsonArray
    auto toBoolArray = [](const QVariant &v) -> QJsonArray {
        QJsonArray arr;
        for (const auto &item : v.toList()) arr.append(item.toBool());
        return arr;
    };
    auto toIntArray = [](const QVariant &v) -> QJsonArray {
        QJsonArray arr;
        for (const auto &item : v.toList()) arr.append(item.toInt());
        return arr;
    };
    auto toDoubleArray = [](const QVariant &v) -> QJsonArray {
        QJsonArray arr;
        for (const auto &item : v.toList()) arr.append(item.toDouble());
        return arr;
    };

    obj["channelEnable"] = toBoolArray(profile.value("channelEnable"));
    obj["trigEnable"]    = toBoolArray(profile.value("trigEnable"));
    obj["trigType"]      = toIntArray(profile.value("trigType"));
    obj["trigValue"]     = toDoubleArray(profile.value("trigValue"));
    obj["channelOffset"] = toDoubleArray(profile.value("channelOffset"));
    obj["calibOffset"]   = toDoubleArray(profile.value("calibOffset"));
    obj["channelRange"]  = toIntArray(profile.value("channelRange"));
    obj["channelBandwidth"] = toIntArray(profile.value("channelBandwidth"));

    return obj;
}

QVariantMap DeviceProfileManager::profileFromJson(const QJsonObject &obj)
{
    QVariantMap map;

    // 基本参数
    map["sampleFreq"]     = obj.value("sampleFreq").toInt();
    map["bufLen"]         = obj.value("bufLen").toInt();
    map["hostBufSeconds"] = obj.value("hostBufSeconds").toInt();
    map["preTrigger"]     = obj.value("preTrigger").toInt();
    map["oversample"]     = obj.value("oversample").toInt();
    map["frameDelay"]     = obj.value("frameDelay").toInt();

    // QJsonArray → QVariantList
    auto toVariantList = [](const QJsonArray &arr) -> QVariantList {
        QVariantList list;
        for (const auto &v : arr) list.append(v.toVariant());
        return list;
    };

    map["channelEnable"] = toVariantList(obj.value("channelEnable").toArray());
    map["trigEnable"]    = toVariantList(obj.value("trigEnable").toArray());
    map["trigType"]      = toVariantList(obj.value("trigType").toArray());
    map["trigValue"]     = toVariantList(obj.value("trigValue").toArray());
    map["channelOffset"] = toVariantList(obj.value("channelOffset").toArray());
    map["calibOffset"]   = toVariantList(obj.value("calibOffset").toArray());
    map["channelRange"]  = toVariantList(obj.value("channelRange").toArray());
    map["channelBandwidth"] = toVariantList(obj.value("channelBandwidth").toArray());

    return map;
}
