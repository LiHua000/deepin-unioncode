// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "plugindetailsview.h"
#include "framework/lifecycle/lifecycle.h"
#include "services/option/optionservice.h"
#include "common/util/eventdefinitions.h"

#include <DWidget>
#include <DLabel>
#include <DTextEdit>
#include <DListWidget>
#include <DPushButton>
#include <DSuggestButton>
#include <DFrame>
#include <DPalette>

#include <QGridLayout>
#include <QDesktopServices>
#ifndef NOT_USE_WEBENGINE
#include <QWebEngineView>
#include <QWebEngineSettings>
#endif
#include <QDir>
#include <QScreen>
#include <QGuiApplication>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

#ifndef NOT_USE_WEBENGINE
class AutoZoomWebEngineView : public QWebEngineView {
public:
    explicit AutoZoomWebEngineView(QWidget *parent = nullptr)
        : QWebEngineView(parent)
    {
        page()->settings()->setAttribute(QWebEngineSettings::ShowScrollBars, false);
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QWebEngineView::resizeEvent(event);

        int pageWidth = static_cast<int>(QGuiApplication::primaryScreen()->size().width() * webPageWidthScale);

        qreal zoomFactor = calculateZoomFactor(event->size(), pageWidth);
        setZoomFactor(zoomFactor);
    }

    qreal calculateZoomFactor(const QSize &size, int pageWidth)
    {
        if (pageWidth == 0)
            return 1;

        qreal zoomFactor = static_cast<qreal>(size.width()) / pageWidth;
        if (zoomFactor > 1) {
            zoomFactor = 1;
        }
        return zoomFactor;
    }

private:
    qreal webPageWidthScale = 0.8;
};
#endif

DetailsView::DetailsView(QWidget *parent)
    : DWidget(parent)
{
    setupUi();
}

DetailsView::~DetailsView()
{
#ifndef NOT_USE_WEBENGINE
    if (webView) {
        webView->stop();
        // delete webView will cause app cannot exit.
        webView->setParent(nullptr);
    }
#endif
}

void DetailsView::update(const dpf::PluginMetaObjectPointer &metaInfo)
{
    pluginMetaInfo = metaInfo;

    if (metaInfo.isNull())
        return;

    QString dependencies;
    for (auto depend : metaInfo->depends()) {
        dependencies += depend.toString();
        if (depend.name() != metaInfo->depends().back().name()) {
            dependencies += "、";
        }
    }
    QString licenses;
    for (QString &text : metaInfo->license()) {
        licenses += text.simplified();
        if (text != metaInfo->license().back()) {
            licenses += "、";
        }
    }

    name->setText(metaInfo->name());
    version->setText(metaInfo->version());
    category->setText(metaInfo->category());
    vendor->setText(metaInfo->vendor());
    description->setText(tr("Description") + ": " + metaInfo->description());
    dependency->setText(tr("Dependency") + ": " + dependencies);

    bool pluginIsEnabled = pluginMetaInfo->isEnabledBySettings();
    updateLoadBtnDisplay(pluginIsEnabled);

    QString pluginPath = CustomPaths::global(CustomPaths::Plugins);
    QString pluginLogoPath = pluginPath + QDir::separator() + pluginMetaInfo->name() + ".svg";
    QIcon pluginLogo;
    if (QFile::exists(pluginLogoPath)) {
        pluginLogo = QIcon::fromTheme(pluginLogoPath);
    } else {
        pluginLogo = QIcon::fromTheme("default_plugin");
    }

    logoLabel->setPixmap(pluginLogo.pixmap(QSize(96, 96)));

#ifndef NOT_USE_WEBENGINE
    webView->load(QUrl(metaInfo->urlLink()));
#endif
}

void DetailsView::changeLoadBtnState()
{
    if (pluginMetaInfo.isNull())
        return;

    bool isEnabled = !pluginMetaInfo->isEnabledBySettings();
    updateLoadBtnDisplay(isEnabled);

    pluginMetaInfo->setEnabledBySettings(isEnabled);
    dpf::LifeCycle::getPluginManagerInstance()->writeSettings();
}

void DetailsView::showCfgWidget()
{
    dpfGetService(OptionService)->showOptionDialog(pluginMetaInfo->name());
    options.showCfgDialg(pluginMetaInfo->name());
}

void DetailsView::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(2);
    mainLayout->setMargin(0);
    DFrame *detailFrame = new DFrame(this);
    auto detailLayout = new QHBoxLayout(detailFrame);
    DStyle::setFrameRadius(detailFrame, 0);
    detailFrame->setLineWidth(0);

    auto midLayout = new QVBoxLayout();
    midLayout->setSpacing(0);
    midLayout->setMargin(0);
    initMetaInfoLayout();

    auto operationLayout = new QHBoxLayout();
    operationLayout->setSpacing(0);
    operationLayout->setMargin(0);
    loadBtn = new DSuggestButton(this);
    loadBtn->setFixedSize(86, 36);
    loadBtn->setToolTip(tr("reLaunch when changed!"));
    loadBtn->setChecked(true);
    operationLayout->addWidget(loadBtn);

    auto *cfgBtn = new DPushButton(this);
    cfgBtn->setIcon(QIcon::fromTheme("options_setting_black"));
    cfgBtn->setFixedSize(36, 36);
    cfgBtn->setToolTip(tr("Settings"));
    connect(cfgBtn, &DPushButton::clicked, this, &DetailsView::showCfgWidget);
    operationLayout->addWidget(cfgBtn);
    operationLayout->setSpacing(10);

    DLabel *tipLabel = new DLabel(this);
    tipLabel->setText(tr("Relaunch required!"));
    tipLabel->setForegroundRole(DPalette::TextWarning);
    operationLayout->addWidget(tipLabel);
    operationLayout->setAlignment(Qt::AlignLeft);
    tipLabel->hide();

    connect(loadBtn, &DSuggestButton::clicked, this, [=](){
        changeLoadBtnState();
        tipLabel->show();
    });
    
    logoLabel = new QLabel(this);
    auto logo = QIcon::fromTheme("default_plugin");
    logoLabel->setPixmap(logo.pixmap(QSize(96, 96)));

    auto webViewLayout = new QHBoxLayout();
#ifndef NOT_USE_WEBENGINE
    webView = new AutoZoomWebEngineView(this);
    webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    webViewLayout->addWidget(webView);
#else
    webViewLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    QLabel *label = new QLabel(tr("This platform not support doc display!"));
    webViewLayout->addWidget(label);

    QFont font = label->font();
    font.setPointSize(20);
    font.setItalic(true);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);

    QPalette palette = label->palette();
    palette.setColor(QPalette::WindowText, Qt::darkGray);
    label->setPalette(palette);

    label->setAutoFillBackground(true);
#endif

    midLayout->addLayout(metaInfoLayout);
    midLayout->addSpacing(10);
    midLayout->addLayout(operationLayout);
    detailLayout->addWidget(logoLabel);
    detailLayout->addSpacing(30);
    detailLayout->addLayout(midLayout, 1);
    detailLayout->setContentsMargins(80, 40, 80, 40);
    mainLayout->addWidget(detailFrame);
    mainLayout->addLayout(webViewLayout);
}

void DetailsView::initMetaInfoLayout()
{
    metaInfoLayout = new QVBoxLayout();
    name = new DLabel(this);
    QFont font = name->font();
    font.setBold(true);
    font.setPointSize(20);
    name->setFont(font);

    versionFrame = new DFrame(this);
    versionFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    versionFrame->setBackgroundRole(DPalette::FrameBorder);
    versionFrame->setLineWidth(0);
    DStyle::setFrameRadius(versionFrame, 4);
    QVBoxLayout *versionLayout = new QVBoxLayout(versionFrame);
    version = new DLabel(versionFrame);
    version->setContentsMargins(7, 0, 7, 0);
    versionLayout->addWidget(version);
    versionLayout->setAlignment(Qt::AlignCenter);
    versionLayout->setSpacing(0);
    versionLayout->setContentsMargins(0, 0, 0, 0);

    categoryFrame = new DFrame(this);
    categoryFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    DPalette palette = categoryFrame->palette();
    QColor color = palette.color(QPalette::Highlight);
    color.setAlpha(20);
    palette.setColor(QPalette::Base, color);
    categoryFrame->setPalette(palette);
    categoryFrame->setLineWidth(0);
    DStyle::setFrameRadius(categoryFrame, 4);
    QVBoxLayout *categoryLayout = new QVBoxLayout(categoryFrame);
    category = new DLabel(categoryFrame);
    category->setForegroundRole(DPalette::LightLively);
    category->setContentsMargins(7, 0, 7, 0);
    categoryLayout->addWidget(category);
    categoryLayout->setAlignment(Qt::AlignCenter);
    categoryLayout->setSpacing(0);
    categoryLayout->setContentsMargins(0, 0, 0, 0);

    description = new DLabel(this);
    vendor = new DLabel(this);
    dependency = new DLabel(this);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setAlignment(Qt::AlignLeft);
    hbox->setSpacing(10);
    hbox->addWidget(name);
    hbox->addWidget(versionFrame);
    hbox->addWidget(categoryFrame);

    metaInfoLayout->addLayout(hbox);
    metaInfoLayout->addWidget(vendor);
    metaInfoLayout->addSpacing(10);
    metaInfoLayout->addWidget(description);
    metaInfoLayout->addWidget(dependency);
}

void DetailsView::updateLoadBtnDisplay(bool isEnabled)
{
    QString loadOperation = isEnabled ? tr("unload") : tr("load");
    loadBtn->setText(loadOperation);
}
