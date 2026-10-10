// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Mstar Semiconductor, Inc.

#include "CollectParamDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QFileDialog>
#include <QDir>
#include <QPushButton>
#include "MstarEthBridge.h"

CollectParamDialog::CollectParamDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("采集参数设置"));
    setModal(true);

    auto *mainLayout = new QVBoxLayout(this);

    // ---- 基本参数（两行布局） ----
    auto *basicGroup = new QGroupBox(tr("基本参数"));
    auto *basicVLayout = new QVBoxLayout(basicGroup);

    // 第一行：采样频率 + 过采样
    auto *row1 = new QHBoxLayout;
    m_spinSampleFreq = new QSpinBox;
    m_spinSampleFreq->setRange(1, 100000000);
    m_spinSampleFreq->setValue(1000000);
    m_spinSampleFreq->setSuffix(" Hz");
    row1->addWidget(new QLabel(tr("采样频率:")));
    row1->addWidget(m_spinSampleFreq);
    row1->addSpacing(20);
    row1->addWidget(new QLabel(tr("过采样:")));
    m_comboOversample = new QComboBox;
    m_comboOversample->addItem(tr("1倍（无过采样）"), COLLECT_OVERSAMPLE_1);
    m_comboOversample->addItem(tr("2倍"),           COLLECT_OVERSAMPLE_2);
    m_comboOversample->addItem(tr("4倍"),           COLLECT_OVERSAMPLE_4);
    m_comboOversample->addItem(tr("8倍"),           COLLECT_OVERSAMPLE_8);
    m_comboOversample->addItem(tr("16倍"),          COLLECT_OVERSAMPLE_16);
    m_comboOversample->addItem(tr("32倍"),          COLLECT_OVERSAMPLE_32);
    m_comboOversample->addItem(tr("64倍"),          COLLECT_OVERSAMPLE_64);
    m_comboOversample->addItem(tr("128倍"),         COLLECT_OVERSAMPLE_128);
    m_comboOversample->addItem(tr("256倍"),         COLLECT_OVERSAMPLE_256);
    m_comboOversample->setCurrentIndex(0);
    row1->addWidget(m_comboOversample);
    row1->addStretch();
    basicVLayout->addLayout(row1);

    // 第二行：设备缓存
    auto *row2 = new QHBoxLayout;
    m_labelBufLen = new QLabel(tr("设备缓存:"));
    row2->addWidget(m_labelBufLen);
    m_comboBufLen = new QComboBox;
    row2->addWidget(m_comboBufLen);
    row2->addStretch();
    basicVLayout->addLayout(row2);

    // 第三行：预触发 + 滑块
    auto *row3 = new QHBoxLayout;
    row3->addWidget(new QLabel(tr("预触发:")));
    m_spinPreTrig = new QSpinBox;
    m_spinPreTrig->setRange(0, 0);
    m_spinPreTrig->setValue(0);
    m_spinPreTrig->setSuffix(" pts");
    row3->addWidget(m_spinPreTrig);
    m_sliderPreTrig = new QSlider(Qt::Horizontal);
    m_sliderPreTrig->setRange(0, 0);
    m_sliderPreTrig->setValue(0);
    m_sliderPreTrig->setFixedWidth(120);
    row3->addWidget(m_sliderPreTrig);
    row3->addStretch();
    basicVLayout->addLayout(row3);

    // 滑块与数值框双向联动
    connect(m_sliderPreTrig, &QSlider::valueChanged, m_spinPreTrig, &QSpinBox::setValue);
    connect(m_spinPreTrig, QOverload<int>::of(&QSpinBox::valueChanged), m_sliderPreTrig, &QSlider::setValue);

    // 过采样切换时，动态调整采样频率上限：maxFreq = baseMaxFreq / 过采样倍数
    connect(m_comboOversample, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        uint8_t osVal = m_comboOversample->itemData(index).toUInt();
        int factor = 1 << osVal;  // OVERSAMPLE_1=0 -> 1, OVERSAMPLE_2=1 -> 2, ...
        int maxFreq = m_baseMaxSampleFreq / factor;
        m_spinSampleFreq->setMaximum(maxFreq);
        // 若当前值超出新上限，自动降低
        if (m_spinSampleFreq->value() > maxFreq)
            m_spinSampleFreq->setValue(maxFreq);
    });

    mainLayout->addWidget(basicGroup);

    // ---- 板间同步 ----
    auto *syncGroup = new QGroupBox(tr("板间同步"));
    auto *syncVLayout = new QVBoxLayout(syncGroup);

    // 同步模式 + 触发信号 IO
    auto *row4 = new QHBoxLayout;
    row4->addWidget(new QLabel(tr("同步模式:")));
    m_comboSyncMode = new QComboBox;
    m_comboSyncMode->addItem(tr("无级联"),                     0);  // 输入输出都禁止
    m_comboSyncMode->addItem(tr("主设备（CLK输出）"), 1);  // 输出使能，输入禁止
    m_comboSyncMode->addItem(tr("从设备（CLK输入）"), 2);  // 输出禁止，输入使能
    m_comboSyncMode->setCurrentIndex(0);
    row4->addWidget(m_comboSyncMode);
    row4->addSpacing(20);
    row4->addWidget(new QLabel(tr("触发信号:")));
    m_chkTrigOut = new QCheckBox(tr("输出"));
    m_chkTrigIn  = new QCheckBox(tr("输入"));
    row4->addWidget(m_chkTrigOut);
    row4->addWidget(m_chkTrigIn);
    row4->addStretch();
    syncVLayout->addLayout(row4);

    // 主设备 IP 地址（从设备时显示）
    auto *row5 = new QHBoxLayout;
    m_labelMasterIp = new QLabel(tr("主设备IP:"));
    row5->addWidget(m_labelMasterIp);
    m_editMasterIp = new QLineEdit;
    m_editMasterIp->setPlaceholderText("192.168.0.100");
    m_editMasterIp->setMaximumWidth(150);
    row5->addWidget(m_editMasterIp);
    row5->addStretch();
    syncVLayout->addLayout(row5);

    // 主设备IP输入框：仅在从设备模式时显示
    auto updateMasterIpVisible = [this]() {
        bool isSlave = (m_comboSyncMode->currentIndex() == 2);
        m_labelMasterIp->setVisible(isSlave);
        m_editMasterIp->setVisible(isSlave);
    };
    updateMasterIpVisible();
    connect(m_comboSyncMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [updateMasterIpVisible](int) { updateMasterIpVisible(); });

    mainLayout->addWidget(syncGroup);

    // ---- 设备发送速度 ----
    auto *speedGroup = new QGroupBox(tr("设备发送速度"));
    auto *speedLayout = new QHBoxLayout(speedGroup);
    speedLayout->addWidget(new QLabel(tr("包间隔:")));
    m_spinFrameDelay = new QSpinBox;
    m_spinFrameDelay->setRange(0, 65535);
    m_spinFrameDelay->setValue(0);
    m_spinFrameDelay->setSuffix(" us");
    m_spinFrameDelay->setToolTip(tr("数据包发送间隔时间（微秒），主动降低网口传输负载，可同一网口同时采集多台设备"));
    speedLayout->addWidget(m_spinFrameDelay);
    speedLayout->addSpacing(10);
    speedLayout->addWidget(new QLabel(tr("注: 1000M网速时，20us大约降一半速度")));
    speedLayout->addStretch();
    mainLayout->addWidget(speedGroup);

    // ---- 实时采集 ----
    auto *saveGroup = new QGroupBox(tr("实时采集"));
    auto *saveVLayout = new QVBoxLayout(saveGroup);

    // 第一行：主机缓存
    auto *hostBufRow = new QHBoxLayout;
    m_labelHostBuf = new QLabel(tr("主机缓存:"));
    hostBufRow->addWidget(m_labelHostBuf);
    m_comboHostBuf = new QComboBox;
    m_comboHostBuf->addItem("1s",   1);
    m_comboHostBuf->addItem("2s",   2);
    m_comboHostBuf->addItem("5s",   5);
    m_comboHostBuf->addItem("10s",  10);
    m_comboHostBuf->addItem("30s",  30);
    m_comboHostBuf->addItem("1min", 60);
    m_comboHostBuf->addItem("2min", 120);
    m_comboHostBuf->addItem("5min", 300);
    m_comboHostBuf->setCurrentIndex(3);  // 默认 10s
    hostBufRow->addWidget(m_comboHostBuf);
    hostBufRow->addStretch();
    saveVLayout->addLayout(hostBufRow);

    // 第二行：实时保存
    auto *saveLayout = new QHBoxLayout;
    m_chkSaveEnable = new QCheckBox(tr("保存使能"));
    saveLayout->addWidget(m_chkSaveEnable);
    saveLayout->addSpacing(10);
    saveLayout->addWidget(new QLabel(tr("保存地址:")));
    m_editSavePath = new QLineEdit;
    m_editSavePath->setPlaceholderText(tr("选择保存目录"));
    m_editSavePath->setText(QDir::homePath());
    saveLayout->addWidget(m_editSavePath, 1);
    auto *btnBrowse = new QPushButton(tr("浏览..."));
    connect(btnBrowse, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("选择保存目录"), m_editSavePath->text());
        if (!dir.isEmpty()) m_editSavePath->setText(dir);
    });
    saveLayout->addWidget(btnBrowse);
    saveLayout->addSpacing(10);
    saveLayout->addWidget(new QLabel(tr("文件前缀:")));
    m_editFilePrefix = new QLineEdit;
    m_editFilePrefix->setText("MCollectDat");
    m_editFilePrefix->setFixedWidth(100);
    saveLayout->addWidget(m_editFilePrefix);
    saveVLayout->addLayout(saveLayout);
    mainLayout->addWidget(saveGroup);

    // ---- 触发设置（每通道独立，在通道列表中） ----

    // ---- 通道设置（使能 + 触发 + 偏移） ----
    auto *chGroup = new QGroupBox(tr("通道设置"));
    // 用 QScrollArea 包裹，通道多时可滚动
    auto *chScroll = new QScrollArea;
    chScroll->setWidgetResizable(true);
    auto *chContainer = new QWidget;
    auto *chGrid = new QGridLayout(chContainer);
    chGrid->setColumnStretch(7, 1);

    // 表头（居中 + 加粗）
    const char *headers[] = {"通道", "采集使能", "触发使能", "触发模式", "触发值(V)", "量程", "带宽", "偏移"};
    for (int col = 0; col < 8; ++col) {
        auto *hdr = new QLabel(tr(headers[col]));
        hdr->setAlignment(Qt::AlignCenter);
        QFont font = hdr->font();
        font.setBold(true);
        hdr->setFont(font);
        chGrid->addWidget(hdr, 0, col);
    }

    // 通道行在 buildChannelTable 中动态创建
    // 先保存布局指针，后续 initFromDevice 使用
    chGroup->setLayout(new QVBoxLayout);
    chGroup->layout()->addWidget(chScroll);
    chScroll->setWidget(chContainer);

    mainLayout->addWidget(chGroup, 1);

    // 保存 grid 布局指针到成员（通过 chContainer 的 property 传递）
    chContainer->setProperty("grid", QVariant::fromValue((void*)chGrid));

    // ---- 确定/取消按钮 ----
    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    resize(400, 500);
}

void CollectParamDialog::buildChannelTable(int nchannel, int nbits)
{
    m_nchannel = nchannel;
    m_nbits = nbits;
    m_chEnableChecks.clear();
    m_chTrigEnableChecks.clear();
    m_chTrigModes.clear();
    m_chTrigValues.clear();
    m_chOffsetSpins.clear();
    m_chRangeCombos.clear();
    m_chBandwidthCombos.clear();

    // 找到 chContainer 的 grid 布局
    QGroupBox *chGroup = findChild<QGroupBox*>(QString(), Qt::FindDirectChildrenOnly);
    // 通过遍历找到 QScrollArea 内的 container
    QWidget *chContainer = nullptr;
    for (auto *group : findChildren<QGroupBox*>()) {
        if (group->title() == tr("通道设置")) {
            auto *scroll = group->findChild<QScrollArea*>();
            if (scroll) chContainer = scroll->widget();
            break;
        }
    }
    if (!chContainer) return;
    auto *chGrid = (QGridLayout*)chContainer->layout();

    for (int ch = 0; ch < nchannel; ++ch) {
        int row = ch + 1;

        // 通道标签（居中显示）
        auto *label = new QLabel(QString("CH%1").arg(ch));
        label->setAlignment(Qt::AlignCenter);
        chGrid->addWidget(label, row, 0);

        // 采集使能复选框（默认使能）
        auto *cb = new QCheckBox;
        cb->setChecked(true);
        chGrid->addWidget(cb, row, 1);
        m_chEnableChecks.append(cb);

        // 触发使能复选框（默认禁止）
        auto *trigEnCb = new QCheckBox;
        trigEnCb->setChecked(false);
        chGrid->addWidget(trigEnCb, row, 2);
        m_chTrigEnableChecks.append(trigEnCb);

        // 触发模式（触发使能时可选：电平/上升沿/下降沿；禁止时默认连续采集）
        auto *comboTrig = new QComboBox;
        comboTrig->addItem(tr("电平触发"),     TRIG_TYPE_LEVEL);
        comboTrig->addItem(tr("上升沿触发"),   TRIG_TYPE_UP);
        comboTrig->addItem(tr("下降沿触发"),   TRIG_TYPE_DOWN);
        comboTrig->setEnabled(false);  // 默认触发禁止，模式不可选
        chGrid->addWidget(comboTrig, row, 3);
        m_chTrigModes.append(comboTrig);

        // 触发值 (V)
        auto *spinTrig = new QDoubleSpinBox;
        spinTrig->setRange(-100.0, 100.0);
        spinTrig->setDecimals(3);
        spinTrig->setSingleStep(0.01);
        spinTrig->setValue(0.0);
        spinTrig->setSuffix(" V");
        spinTrig->setFixedWidth(100);
        spinTrig->setEnabled(false);  // 默认触发禁止，不可设
        chGrid->addWidget(spinTrig, row, 4);
        m_chTrigValues.append(spinTrig);

        // 触发使能联动：使能时可选模式+设值，禁止时默认连续采集
        connect(trigEnCb, &QCheckBox::toggled, [comboTrig, spinTrig](bool checked) {
            comboTrig->setEnabled(checked);
            spinTrig->setEnabled(checked);
        });

        // 量程选择（选项在 initFromDevice 中根据设备信息填充）
        auto *comboRange = new QComboBox;
        chGrid->addWidget(comboRange, row, 5);
        m_chRangeCombos.append(comboRange);

        // 输入带宽（高/低）
        auto *comboBw = new QComboBox;
        comboBw->addItem(tr("高"), BANDWIDTH_HIGH);
        comboBw->addItem(tr("低"), BANDWIDTH_LOW);
        chGrid->addWidget(comboBw, row, 6);
        m_chBandwidthCombos.append(comboBw);

        // 偏移校准 (int16_t AD值)，可编辑，通过 set_collect_offset 写入设备
        auto *spinOffset = new QSpinBox;
        spinOffset->setRange(-32768, 32767);
        spinOffset->setSingleStep(1);
        spinOffset->setValue(0);
        spinOffset->setFixedWidth(80);
        chGrid->addWidget(spinOffset, row, 7);
        m_chOffsetSpins.append(spinOffset);
    }

    // 隔行变色 + 表格线 + 固定行高
    QColor altColor("#f0f4f8");
    for (int i = 0; i < chGrid->count(); ++i) {
        auto *item = chGrid->itemAt(i);
        if (!item) continue;
        int r, c, rs, cs;
        chGrid->getItemPosition(i, &r, &c, &rs, &cs);
        QWidget *w = item->widget();
        if (!w) continue;
        if (r % 2 == 1) {
            w->setStyleSheet(QString("background-color: %1; border: 1px solid #c0c0c0; margin: 0px; padding: 0px;")
                .arg(altColor.name()));
        } else {
            w->setStyleSheet("border: 1px solid #c0c0c0; margin: 0px; padding: 0px;");
        }
        w->setFixedHeight(26);
    }
}

void CollectParamDialog::initFromDevice(MHANDLE hdev, const collectinforst &info)
{
    m_hdev = hdev;
    m_nchannel = info.nchannel;
    m_nbits = info.nbits;
    m_bytesPerSample = (info.nbits + 7) / 8;

    // 基础最高采样频率（无过采样时），过采样时按倍数降低
    m_baseMaxSampleFreq = info.samplefreq_max;
    m_spinSampleFreq->setRange(1, m_baseMaxSampleFreq);

    // 缓存选项：显示单通道采样点数（总字节 / 通道数 / 每采样字节）
    m_comboBufLen->clear();
    uint32_t totalBuf = info.databuflen;
    int perChSamples = totalBuf / info.nchannel / m_bytesPerSample;
    m_devMaxPerChSamples = perChSamples;  // 保存设备最大缓存（单通道采样点数）
    // 从最小到最大，按 2 的幂次生成选项（存储值为单通道采样点数）
    int minSamples = 4096 / info.nchannel / m_bytesPerSample;
    if (minSamples < 1) minSamples = 1;
    for (int sz = minSamples; sz <= perChSamples; sz *= 2) {
        QString label;
        if (sz >= 1024 * 1024)
            label = QString("%1 M").arg(sz / (1024 * 1024));
        else if (sz >= 1024)
            label = QString("%1 K").arg(sz / 1024);
        else
            label = QString::number(sz);
        m_comboBufLen->addItem(label, sz);
    }
    if (m_comboBufLen->count() > 0)
        m_comboBufLen->setCurrentIndex(m_comboBufLen->count() - 1);

    // 预触发位置范围：0 ~ 单通道缓存大小 - 1（从 0 开始计数）
    m_spinPreTrig->setRange(0, perChSamples - 1);
    m_spinPreTrig->setValue(0);
    m_sliderPreTrig->setRange(0, perChSamples - 1);
    m_sliderPreTrig->setValue(0);

    // 预触发范围联动设备缓存：切换缓存大小时，预触发上限不超过当前缓存的每通道采样点数 - 1，默认置为中间值
    connect(m_comboBufLen, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        int perCh = m_comboBufLen->itemData(index).toInt();
        m_spinPreTrig->setRange(0, perCh - 1);
        m_sliderPreTrig->setRange(0, perCh - 1);
        m_spinPreTrig->setValue((perCh - 1) / 2);
    });

    // 构建通道表格
    // 先清除旧的通道控件
    QWidget *chContainer = nullptr;
    for (auto *group : findChildren<QGroupBox*>()) {
        if (group->title() == tr("通道设置")) {
            auto *scroll = group->findChild<QScrollArea*>();
            if (scroll) chContainer = scroll->widget();
            break;
        }
    }
    if (chContainer) {
        // 清除旧控件（保留表头行）
        auto *layout = chContainer->layout();
        while (layout->count() > 8) {  // 保留前 8 个（表头）
            auto *item = layout->takeAt(3);
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
    }
    buildChannelTable(info.nchannel, info.nbits);

    // 偏移校准默认归零
    for (int ch = 0; ch < m_chOffsetSpins.size(); ++ch) {
        m_chOffsetSpins[ch]->setValue(0);
    }
    // 填充量程选项并读取当前量程
    if (hdev && info.nchannel > 0) {
        // 量程码 → 显示名称
        auto rangeName = [this](uint8_t code) -> QString {
            switch (code) {
                case AD_AIN_RANGE_SPN2V5:  return tr("\u00b12.5V(\u5355\u7aef)");
                case AD_AIN_RANGE_SPN5V:   return tr("\u00b15V(\u5355\u7aef)");
                case AD_AIN_RANGE_SPN6V25: return tr("\u00b16.25V(\u5355\u7aef)");
                case AD_AIN_RANGE_SPN10V:  return tr("\u00b110V(\u5355\u7aef)");
                case AD_AIN_RANGE_SPN12V5: return tr("\u00b112.5V(\u5355\u7aef)");
                case AD_AIN_RANGE_SP5V:    return tr("0~5V(\u5355\u7aef)");
                case AD_AIN_RANGE_SP10V:   return tr("0~10V(\u5355\u7aef)");
                case AD_AIN_RANGE_SP12V5:  return tr("0~12.5V(\u5355\u7aef)");
                case AD_AIN_RANGE_D5V:     return tr("\u00b15V(\u5dee\u5206)");
                case AD_AIN_RANGE_D10V:    return tr("\u00b110V(\u5dee\u5206)");
                case AD_AIN_RANGE_D12V5:   return tr("\u00b112.5V(\u5dee\u5206)");
                case AD_AIN_RANGE_D20V:    return tr("\u00b120V(\u5dee\u5206)");
                case AD_AIN_RANGE_D25V:    return tr("\u00b125V(\u5dee\u5206)");
                default: return QString("0x%1").arg(code, 2, 16, QChar('0'));
            }
        };

        // 为每个通道的量程下拉框添加选项
        for (int i = 0; i < info.rangearry_num; ++i) {
            uint8_t code = info.singer_rangarry[i];
            if (code == AD_AIN_RANGE_NO) break;
            for (auto *combo : m_chRangeCombos)
                combo->addItem(rangeName(code), code);
        }

        // 读取当前量程并设置选中项
        QVector<uint8_t> ranges(info.nchannel, AD_AIN_RANGE_SPN5V);
        if (get_collect_range(hdev, ranges.data(), info.nchannel) > 0) {
            for (int ch = 0; ch < info.nchannel; ++ch) {
                if (ch < m_chRangeCombos.size()) {
                    int idx = m_chRangeCombos[ch]->findData(ranges[ch]);
                    if (idx >= 0) m_chRangeCombos[ch]->setCurrentIndex(idx);
                }
            }
        }

        // 读取当前带宽并设置选中项
        QVector<uint8_t> bw(info.nchannel, BANDWIDTH_HIGH);
        if (get_bandwidth(hdev, bw.data(), info.nchannel) > 0) {
            for (int ch = 0; ch < info.nchannel; ++ch) {
                if (ch < m_chBandwidthCombos.size()) {
                    int idx = m_chBandwidthCombos[ch]->findData(bw[ch]);
                    if (idx >= 0) m_chBandwidthCombos[ch]->setCurrentIndex(idx);
                }
            }
        }
    }

    // 读取数据包发送间隔
    if (hdev) {
        uint32_t delay = get_framdelay(hdev);
        m_spinFrameDelay->setValue(delay);
    }
}

int CollectParamDialog::sampleFreq() const
{
    return m_spinSampleFreq->value();
}

void CollectParamDialog::setSampleFreq(int freq)
{
    m_spinSampleFreq->setValue(freq);
}

int CollectParamDialog::bufLen() const
{
    // combo 存储的是单通道采样点数，返回总字节数
    int perChSamples = m_comboBufLen->currentData().toInt();
    return perChSamples * m_nchannel * m_bytesPerSample;
}

void CollectParamDialog::setBufLen(int buflen)
{
    // buflen 是总字节数，转换为单通道采样点数查找
    int perChSamples = buflen / m_nchannel / m_bytesPerSample;
    int idx = m_comboBufLen->findData(perChSamples);
    if (idx >= 0)
        m_comboBufLen->setCurrentIndex(idx);
}

int CollectParamDialog::hostBufSamples() const
{
    int seconds = m_comboHostBuf->currentData().toInt();
    return seconds * m_spinSampleFreq->value();
}

int CollectParamDialog::hostBufSeconds() const
{
    return m_comboHostBuf->currentData().toInt();
}

void CollectParamDialog::setHostBufSeconds(int seconds)
{
    int idx = m_comboHostBuf->findData(seconds);
    if (idx >= 0)
        m_comboHostBuf->setCurrentIndex(idx);
}


int CollectParamDialog::preTrigger() const
{
    return m_spinPreTrig->value();
}

void CollectParamDialog::setPreTrigger(int samples)
{
    m_spinPreTrig->setValue(samples);
}

uint8_t CollectParamDialog::oversample() const
{
    return m_comboOversample->currentData().toUInt();
}

void CollectParamDialog::setOversample(uint8_t val)
{
    int idx = m_comboOversample->findData(val);
    if (idx >= 0)
        m_comboOversample->setCurrentIndex(idx);
}

QVector<bool> CollectParamDialog::channelEnable() const
{
    QVector<bool> enables;
    for (auto *cb : m_chEnableChecks)
        enables.append(cb->isChecked());
    return enables;
}

void CollectParamDialog::setChannelEnable(const QVector<bool> &enables)
{
    for (int i = 0; i < qMin(enables.size(), m_chEnableChecks.size()); ++i)
        m_chEnableChecks[i]->setChecked(enables[i]);
}

QVector<bool> CollectParamDialog::trigEnable() const
{
    QVector<bool> enables;
    for (auto *cb : m_chTrigEnableChecks)
        enables.append(cb->isChecked());
    return enables;
}

void CollectParamDialog::setTrigEnable(const QVector<bool> &enables)
{
    for (int i = 0; i < qMin(enables.size(), m_chTrigEnableChecks.size()); ++i)
        m_chTrigEnableChecks[i]->setChecked(enables[i]);
}

QVector<uint32_t> CollectParamDialog::trigType() const
{
    QVector<uint32_t> types;
    for (auto *cb : m_chTrigModes)
        types.append(cb->currentData().toUInt());
    return types;
}

void CollectParamDialog::setTrigType(const QVector<uint32_t> &types)
{
    for (int i = 0; i < qMin(types.size(), m_chTrigModes.size()); ++i) {
        int idx = m_chTrigModes[i]->findData(types[i]);
        if (idx >= 0)
            m_chTrigModes[i]->setCurrentIndex(idx);
    }
}

QVector<double> CollectParamDialog::trigValue() const
{
    QVector<double> values;
    for (auto *sp : m_chTrigValues)
        values.append(sp->value());
    return values;
}

void CollectParamDialog::setTrigValue(const QVector<double> &values)
{
    for (int i = 0; i < qMin(values.size(), m_chTrigValues.size()); ++i)
        m_chTrigValues[i]->setValue(values[i]);
}

QVector<int16_t> CollectParamDialog::channelOffset() const
{
    QVector<int16_t> offsets;
    for (auto *sp : m_chOffsetSpins)
        offsets.append((int16_t)sp->value());
    return offsets;
}

QVector<int16_t> CollectParamDialog::channelOffsetAD() const
{
    QVector<int16_t> offsets;
    for (auto *sp : m_chOffsetSpins)
        offsets.append((int16_t)sp->value());
    return offsets;
}

void CollectParamDialog::setChannelOffsetAD(const QVector<int16_t> &adValues)
{
    for (int ch = 0; ch < qMin(adValues.size(), m_chOffsetSpins.size()); ++ch)
        m_chOffsetSpins[ch]->setValue(adValues[ch]);
}

QVector<uint8_t> CollectParamDialog::channelRange() const
{
    QVector<uint8_t> ranges;
    for (auto *cb : m_chRangeCombos)
        ranges.append(cb->currentData().toUInt());
    return ranges;
}

void CollectParamDialog::setChannelRange(const QVector<uint8_t> &ranges)
{
    for (int ch = 0; ch < qMin(ranges.size(), m_chRangeCombos.size()); ++ch) {
        int idx = m_chRangeCombos[ch]->findData(ranges[ch]);
        if (idx >= 0)
            m_chRangeCombos[ch]->setCurrentIndex(idx);
    }
}

QVector<uint8_t> CollectParamDialog::channelBandwidth() const
{
    QVector<uint8_t> bw;
    for (auto *cb : m_chBandwidthCombos)
        bw.append(cb->currentData().toUInt());
    return bw;
}

void CollectParamDialog::setChannelBandwidth(const QVector<uint8_t> &bw)
{
    for (int ch = 0; ch < qMin(bw.size(), m_chBandwidthCombos.size()); ++ch) {
        int idx = m_chBandwidthCombos[ch]->findData(bw[ch]);
        if (idx >= 0)
            m_chBandwidthCombos[ch]->setCurrentIndex(idx);
    }
}

// --- 同步CLK / 触发信号 IO 使能 ---

// 同步模式：0=无级联, 1=主设备(CLK输出), 2=从设备(CLK输入)
bool CollectParamDialog::convClkOut() const { return m_comboSyncMode->currentData().toInt() == 1; }
bool CollectParamDialog::convClkIn() const  { return m_comboSyncMode->currentData().toInt() == 2; }
bool CollectParamDialog::trigOut() const    { return m_chkTrigOut->isChecked(); }
bool CollectParamDialog::trigIn() const     { return m_chkTrigIn->isChecked(); }

void CollectParamDialog::setConvClkOut(bool en) { m_comboSyncMode->setCurrentIndex(en ? 1 : 0); }
void CollectParamDialog::setConvClkIn(bool en)  { m_comboSyncMode->setCurrentIndex(en ? 2 : 0); }
void CollectParamDialog::setTrigOut(bool en)    { m_chkTrigOut->setChecked(en); }
void CollectParamDialog::setTrigIn(bool en)     { m_chkTrigIn->setChecked(en); }

// --- 主设备 IP 地址 ---

QString CollectParamDialog::masterIp() const
{
    return m_editMasterIp->text().trimmed();
}

void CollectParamDialog::setMasterIp(const QString &ip)
{
    m_editMasterIp->setText(ip);
}

// --- 实时保存 ---

bool CollectParamDialog::saveEnable() const { return m_chkSaveEnable->isChecked(); }
void CollectParamDialog::setSaveEnable(bool en) { m_chkSaveEnable->setChecked(en); }

QString CollectParamDialog::savePath() const { return m_editSavePath->text().trimmed(); }
void CollectParamDialog::setSavePath(const QString &path) { m_editSavePath->setText(path); }

QString CollectParamDialog::filePrefix() const { return m_editFilePrefix->text().trimmed(); }
void CollectParamDialog::setFilePrefix(const QString &prefix) { m_editFilePrefix->setText(prefix); }

// --- 数据包发送间隔 ---

uint32_t CollectParamDialog::frameDelay() const { return m_spinFrameDelay->value(); }
void CollectParamDialog::setFrameDelay(uint32_t delay) { m_spinFrameDelay->setValue(delay); }
