// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <QString>
#include <QMap>
#include <QObject>

/**
 * @brief 轻量级翻译单例
 *
 * 不依赖 LinguistTools / .ts / .qm，翻译表直接编译进二进制。
 * 通过 T("中文") 获取当前语言的字符串。
 * 工具栏切换语言后 emit languageChanged()，各窗口 retranslateUi() 刷新。
 */
class Translation : public QObject
{
    Q_OBJECT
public:
    static Translation &instance();

    /// 获取翻译后的字符串。中文模式下直接返回 source。
    QString t(const QString &source) const;

    /// 当前是否为英文模式
    bool isEnglish() const { return m_lang == "en"; }

    /// 切换语言 ("zh" / "en")
    void setLanguage(const QString &lang);
    QString language() const { return m_lang; }

signals:
    void languageChanged();

private:
    Translation();
    void buildMap();

    QString m_lang;  // "zh" or "en"
    QMap<QString, QString> m_map;
};

/// 全局快捷宏
inline QString T(const QString &source) { return Translation::instance().t(source); }

#endif // TRANSLATION_H
