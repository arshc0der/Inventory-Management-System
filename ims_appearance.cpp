#include "ims_appearance.h"
#include "ims_theme.h"

#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QColor>
#include <QList>

namespace {
struct Swatch { const char *name; const char *hex; };
const Swatch kSwatches[] = {
    {"IBM Blue", "#0F62FE"}, {"Purple", "#8A3FFC"}, {"Teal", "#007D79"},
    {"Magenta", "#D02670"},  {"Orange", "#C45500"}, {"Green", "#198038"},
    {"Slate", "#4D5358"},
};

QLabel *caption(const QString &text, QWidget *parent)
{
    auto *l = new QLabel(text, parent);
    l->setProperty("role", "muted");
    return l;
}
} // namespace

AppearanceDialog::AppearanceDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Appearance & preferences"));
    setProperty("ims_managed", true);
    setModal(true);
    setFixedWidth(460);

    ThemeManager &tm = ThemeManager::instance();

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 22, 24, 20);
    root->setSpacing(10);

    auto *title = new QLabel(tr("Appearance"), this);
    title->setProperty("role", "title");
    root->addWidget(title);
    root->addWidget(caption(tr("Changes apply instantly and are remembered."), this));
    root->addSpacing(8);

    // ---- theme -----------------------------------------------------------
    root->addWidget(caption(tr("THEME"), this));
    auto *themeRow = new QHBoxLayout;
    m_light = new QPushButton(tr("Light"), this);
    m_dark = new QPushButton(tr("Dark"), this);
    auto *themeGroup = new QButtonGroup(this);
    for (QPushButton *b : {m_light, m_dark}) {
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        themeGroup->addButton(b);
        themeRow->addWidget(b, 1);
    }
    root->addLayout(themeRow);
    connect(m_light, &QPushButton::clicked, this, []() { ThemeManager::instance().setMode(ThemeManager::Light); });
    connect(m_dark, &QPushButton::clicked, this, []() { ThemeManager::instance().setMode(ThemeManager::Dark); });

    // ---- accent ----------------------------------------------------------
    root->addSpacing(6);
    root->addWidget(caption(tr("ACCENT COLOUR"), this));
    auto *swatchRow = new QHBoxLayout;
    swatchRow->setSpacing(10);
    auto *swatchGroup = new QButtonGroup(this);
    for (const Swatch &sw : kSwatches) {
        auto *b = new QPushButton(this);
        b->setCheckable(true);
        b->setFixedSize(32, 32);
        b->setCursor(Qt::PointingHandCursor);
        b->setToolTip(tr(sw.name));
        b->setProperty("hex", QString(sw.hex));
        b->setStyleSheet(QString("QPushButton { background: %1; border: 3px solid transparent; border-radius: 16px; padding: 0; }"
                                 "QPushButton:hover { border: 3px solid rgba(128,128,128,120); }"
                                 "QPushButton:checked { border: 3px solid rgba(128,128,128,230); }")
                             .arg(sw.hex));
        swatchGroup->addButton(b);
        swatchRow->addWidget(b);
        m_swatches << b;
        connect(b, &QPushButton::clicked, this, [b]() {
            ThemeManager::instance().setAccentColor(QColor(b->property("hex").toString()));
        });
    }
    swatchRow->addStretch(1);
    root->addLayout(swatchRow);

    // ---- density ---------------------------------------------------------
    root->addSpacing(6);
    root->addWidget(caption(tr("DENSITY"), this));
    auto *denRow = new QHBoxLayout;
    m_comfortable = new QPushButton(tr("Comfortable"), this);
    m_compact = new QPushButton(tr("Compact"), this);
    auto *denGroup = new QButtonGroup(this);
    for (QPushButton *b : {m_comfortable, m_compact}) {
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        denGroup->addButton(b);
        denRow->addWidget(b, 1);
    }
    root->addLayout(denRow);
    connect(m_comfortable, &QPushButton::clicked, this,
            []() { ThemeManager::instance().setDensity(ThemeManager::Comfortable); });
    connect(m_compact, &QPushButton::clicked, this,
            []() { ThemeManager::instance().setDensity(ThemeManager::Compact); });

    // ---- text size -------------------------------------------------------
    root->addSpacing(6);
    root->addWidget(caption(tr("TEXT SIZE"), this));
    auto *fontRow = new QHBoxLayout;
    m_font = new QSlider(Qt::Horizontal, this);
    m_font->setRange(9, 15);
    m_font->setPageStep(1);
    m_fontValue = new QLabel(this);
    m_fontValue->setMinimumWidth(44);
    m_fontValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    fontRow->addWidget(m_font, 1);
    fontRow->addWidget(m_fontValue);
    root->addLayout(fontRow);
    connect(m_font, &QSlider::valueChanged, this, [this](int v) {
        m_fontValue->setText(QString("%1 pt").arg(v));
        ThemeManager::instance().setFontSize(v);
    });

    // ---- motion ----------------------------------------------------------
    root->addSpacing(6);
    root->addWidget(caption(tr("MOTION"), this));
    m_motion = new QCheckBox(tr("Animations and page transitions"), this);
    root->addWidget(m_motion);
    connect(m_motion, &QCheckBox::toggled, this,
            [](bool on) { ThemeManager::instance().setAnimationsEnabled(on); });

    // ---- footer ----------------------------------------------------------
    root->addSpacing(12);
    auto *foot = new QHBoxLayout;
    auto *reset = new QPushButton(tr("Reset to defaults"), this);
    auto *done = new QPushButton(tr("Done"), this);
    done->setProperty("role", "primary");
    reset->setCursor(Qt::PointingHandCursor);
    done->setCursor(Qt::PointingHandCursor);
    foot->addWidget(reset);
    foot->addStretch(1);
    foot->addWidget(done);
    root->addLayout(foot);
    connect(reset, &QPushButton::clicked, this, []() { ThemeManager::instance().resetAppearance(); });
    connect(done, &QPushButton::clicked, this, &QDialog::accept);

    connect(&tm, &ThemeManager::appearanceChanged, this, [this]() { sync(); });
    sync();
}

void AppearanceDialog::sync()
{
    const ThemeManager &tm = ThemeManager::instance();

    m_light->setChecked(!tm.isDark());
    m_dark->setChecked(tm.isDark());
    m_comfortable->setChecked(tm.density() == ThemeManager::Comfortable);
    m_compact->setChecked(tm.density() == ThemeManager::Compact);

    for (QPushButton *b : m_swatches)
        b->setChecked(QColor(b->property("hex").toString()) == tm.accentColor());

    {
        const QSignalBlocker block(m_font);
        m_font->setValue(tm.fontSize());
    }
    m_fontValue->setText(QString("%1 pt").arg(tm.fontSize()));

    {
        const QSignalBlocker block(m_motion);
        m_motion->setChecked(tm.animationsEnabled());
    }
}
