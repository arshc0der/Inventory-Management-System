#include "ims_theme.h"
#include "ims_fx.h"

#include <QApplication>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTableView>
#include <QHeaderView>
#include <QStyle>
#include <QFont>
#include <QSettings>
#include <QEvent>
#include <QRegularExpression>
#include <QStringList>
#include <QPen>
#include <QBrush>
#include <QIcon>
#include <QVariant>
#include <QEasingCurve>

#include <QtCharts/QChart>
#include <QtCharts/QLegend>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QAbstractSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAbstractBarSeries>
#include <QtCharts/QBarSet>

namespace {
// Widgets inside a subtree flagged with ims_managed=true (e.g. InventoryExplorer)
// set their own roles and must not be re-classified by the legacy heuristics.
bool isManaged(const QWidget *w)
{
    for (const QWidget *p = w; p; p = p->parentWidget()) {
        if (p->property("ims_managed").toBool())
            return true;
    }
    return false;
}
} // namespace

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------
ThemeManager &ThemeManager::instance()
{
    static ThemeManager inst;
    return inst;
}

ThemeManager::ThemeManager() : QObject(nullptr)
{
    // Explicit org/app so we never touch QCoreApplication's names
    // (DatabaseManager derives the database path from them).
    QSettings s("IMS", "InventoryManagementSystem");
    m_mode = (s.value("ui/theme", "light").toString() == "dark") ? Dark : Light;
    const QColor acc(s.value("ui/accent", "#0F62FE").toString());
    if (acc.isValid())
        m_accent = acc;
    m_density = (s.value("ui/density", "comfortable").toString() == "compact") ? Compact : Comfortable;
    m_fontSize = qBound(9, s.value("ui/fontSize", 10).toInt(), 15);
    m_animations = s.value("ui/animations", true).toBool();
}

void ThemeManager::install()
{
    if (m_installed)
        return;
    m_installed = true;

    qApp->setStyle("Fusion");   // predictable base for QSS on every platform

    applyFont();
    qApp->installEventFilter(this);
    rebuild();
}

void ThemeManager::setMode(Mode m)
{
    if (m == m_mode)
        return;
    m_mode = m;
    saveSettings();
    rebuild();
    emit themeChanged(m == Dark);
    emit appearanceChanged();
}

void ThemeManager::saveSettings() const
{
    QSettings s("IMS", "InventoryManagementSystem");
    s.setValue("ui/theme", m_mode == Dark ? "dark" : "light");
    s.setValue("ui/accent", m_accent.name());
    s.setValue("ui/density", m_density == Compact ? "compact" : "comfortable");
    s.setValue("ui/fontSize", m_fontSize);
    s.setValue("ui/animations", m_animations);
}

void ThemeManager::applyFont()
{
    QFont f = qApp->font();
    f.setFamilies({"IBM Plex Sans", "Segoe UI", "Inter", "Helvetica Neue", "Arial"});
    f.setPointSize(m_fontSize);
    qApp->setFont(f);
}

void ThemeManager::setAccentColor(const QColor &c)
{
    if (!c.isValid() || c == m_accent)
        return;
    m_accent = c;
    saveSettings();
    rebuild();
    emit appearanceChanged();
}

void ThemeManager::setDensity(Density d)
{
    if (d == m_density)
        return;
    m_density = d;
    saveSettings();
    rebuild();
    emit appearanceChanged();
}

void ThemeManager::setFontSize(int points)
{
    points = qBound(9, points, 15);
    if (points == m_fontSize)
        return;
    m_fontSize = points;
    saveSettings();
    applyFont();
    rebuild();
    emit appearanceChanged();
}

void ThemeManager::setAnimationsEnabled(bool on)
{
    if (on == m_animations)
        return;
    m_animations = on;
    saveSettings();
    emit appearanceChanged();
}

void ThemeManager::resetAppearance()
{
    m_accent = QColor("#0F62FE");
    m_density = Comfortable;
    m_fontSize = 10;
    m_animations = true;
    m_mode = Light;
    saveSettings();
    applyFont();
    rebuild();
    emit themeChanged(false);
    emit appearanceChanged();
}

void ThemeManager::toggle()
{
    setMode(m_mode == Dark ? Light : Dark);
}

void ThemeManager::repolish(QWidget *w)
{
    if (!w)
        return;
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

// ---------------------------------------------------------------------------
// Design tokens
// ---------------------------------------------------------------------------
QMap<QString, QString> ThemeManager::tokens() const
{
    QMap<QString, QString> t;
    if (m_mode == Light) {
        t["bg"] = "#F4F4F4";           t["surface"] = "#FFFFFF";
        t["field"] = "#F4F4F4";        t["border"] = "#E0E0E0";
        t["borderStrong"] = "#8D8D8D"; t["text"] = "#161616";
        t["text2"] = "#525252";        t["text3"] = "#8D8D8D";
        t["accent"] = "#0F62FE";       t["btnPrimary"] = "#0F62FE";
        t["btnPrimaryHover"] = "#0353E9"; t["btnPrimaryActive"] = "#002D9C";
        t["btnSecondary"] = "#393939"; t["btnSecondaryHover"] = "#4C4C4C";
        t["btnSecondaryActive"] = "#6F6F6F";
        t["danger"] = "#DA1E28";       t["dangerHover"] = "#BA1B23";
        t["dangerActive"] = "#750E13";
        t["navHover"] = "#E8E8E8";     t["navActive"] = "#EDF5FF";
        t["navText"] = "#525252";      t["headerBg"] = "#E0E0E0";
        t["rowHover"] = "#E8E8E8";     t["rowAlt"] = "#F9F9F9";
        t["selection"] = "#D0E2FF";    t["disabledBg"] = "#C6C6C6";
        t["disabledText"] = "#8D8D8D"; t["shell"] = "#161616";
        t["shellText"] = "#C6C6C6";    t["tipBg"] = "#393939";
        t["tipText"] = "#F4F4F4";      t["thumb"] = "#A8A8A8";
        t["ok"] = "#198038";           t["okBg"] = "#DEFBE6";
        t["warn"] = "#8E6A00";         t["warnBg"] = "#FCF4D6";
        t["bad"] = "#A2191F";          t["badBg"] = "#FFD7D9";
    } else {
        t["bg"] = "#161616";           t["surface"] = "#262626";
        t["field"] = "#393939";        t["border"] = "#393939";
        t["borderStrong"] = "#6F6F6F"; t["text"] = "#F4F4F4";
        t["text2"] = "#C6C6C6";        t["text3"] = "#8D8D8D";
        t["accent"] = "#78A9FF";       t["btnPrimary"] = "#0F62FE";
        t["btnPrimaryHover"] = "#0353E9"; t["btnPrimaryActive"] = "#002D9C";
        t["btnSecondary"] = "#6F6F6F"; t["btnSecondaryHover"] = "#606060";
        t["btnSecondaryActive"] = "#393939";
        t["danger"] = "#DA1E28";       t["dangerHover"] = "#BA1B23";
        t["dangerActive"] = "#750E13";
        t["navHover"] = "#353535";     t["navActive"] = "#1C2B45";
        t["navText"] = "#C6C6C6";      t["headerBg"] = "#393939";
        t["rowHover"] = "#353535";     t["rowAlt"] = "#2B2B2B";
        t["selection"] = "#1F3A66";    t["disabledBg"] = "#393939";
        t["disabledText"] = "#6F6F6F"; t["shell"] = "#0D0D0D";
        t["shellText"] = "#C6C6C6";    t["tipBg"] = "#F4F4F4";
        t["tipText"] = "#161616";      t["thumb"] = "#6F6F6F";
        t["ok"] = "#42BE65";           t["okBg"] = "#0E3B1E";
        t["warn"] = "#F1C21B";         t["warnBg"] = "#3D3200";
        t["bad"] = "#FF8389";          t["badBg"] = "#4A1519";
    }

    // ---- accent-derived tokens -------------------------------------------
    auto blend = [](const QColor &a, const QColor &b, double k) {
        return QColor(int(a.red() * (1 - k) + b.red() * k),
                      int(a.green() * (1 - k) + b.green() * k),
                      int(a.blue() * (1 - k) + b.blue() * k));
    };
    const QColor base = (m_mode == Dark) ? QColor("#262626") : QColor("#FFFFFF");
    t["btnPrimary"]       = m_accent.name();
    t["btnPrimaryHover"]  = m_accent.darker(115).name();
    t["btnPrimaryActive"] = m_accent.darker(165).name();
    t["accent"]           = (m_mode == Dark ? m_accent.lighter(145) : m_accent).name();
    t["navActive"]        = blend(base, m_accent, m_mode == Dark ? 0.24 : 0.10).name();
    t["selection"]        = blend(base, m_accent, m_mode == Dark ? 0.38 : 0.20).name();
    t["heroStart"]        = m_accent.name();
    t["heroEnd"]          = m_accent.darker(320).name();

    // ---- density-derived paddings ----------------------------------------
    const bool c = (m_density == Compact);
    t["padBtn"]        = c ? "5px 14px"  : "9px 18px";
    t["padBtnFocus"]   = c ? "3px 12px"  : "7px 16px";
    t["padField"]      = c ? "4px 8px"   : "8px 10px";
    t["padFieldFocus"] = c ? "3px 7px"   : "7px 9px";
    t["padNav"]        = c ? "8px 14px"  : "11px 14px";
    t["padTab"]        = c ? "6px 14px"  : "10px 18px";
    return t;
}

QColor ThemeManager::color(const QString &token) const
{
    return QColor(tokens().value(token, "#FF00FF"));
}

// ---------------------------------------------------------------------------
// Stylesheet
// ---------------------------------------------------------------------------
QString ThemeManager::buildStyleSheet() const
{
    QString q = QStringLiteral(R"QSS(
/* ---------- base ---------- */
QMainWindow, QDialog { background: @bg@; }
QStackedWidget > QWidget { background: @bg@; }
QLabel { background: transparent; color: @text@; }
QCheckBox, QRadioButton { background: transparent; color: @text@; spacing: 8px; }
QToolTip { background: @tipBg@; color: @tipText@; border: none; padding: 6px 10px; }
QScrollArea { background: transparent; border: none; }
QScrollArea > QWidget > QWidget { background: transparent; }
QMessageBox { background: @surface@; }
QMessageBox QLabel { color: @text@; }

/* ---------- containers (roles are assigned by ThemeManager::prepare) ---------- */
QFrame[role="card"]    { background: @surface@; border: 1px solid @border@; border-radius: 4px; }
QFrame[role="panel"]   { background: transparent; border: none; }
QFrame[role="sidebar"] { background: @surface@; border: none; border-right: 1px solid @border@; border-radius: 0; }
QFrame[role="field"]   { background: @field@; border: none; border-bottom: 1px solid @borderStrong@; border-radius: 0; }
QFrame[role="pill"]    { background: @field@; border: 1px solid @border@; border-radius: 20px; }
QFrame[role="hero"]    { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 @heroStart@, stop:1 @heroEnd@); border: none; border-radius: 0; }
QFrame[role="hero"] QLabel { color: #FFFFFF; background: transparent; }
QFrame[role="field"] QLineEdit, QFrame[role="field"] QComboBox { background: transparent; border: none; }
QFrame[role="field"] QLineEdit:focus { border: none; }

QFrame[role="card"][accent="0"] { border-left: 4px solid @btnPrimary@; }
QFrame[role="card"][accent="1"] { border-left: 4px solid #8A3FFC; }
QFrame[role="card"][accent="2"] { border-left: 4px solid #009D9A; }
QFrame[role="card"][accent="3"] { border-left: 4px solid #FF832B; }

QLabel[role="kpi"]   { font-size: 15px; font-weight: 600; color: @text@; }
QLabel[role="muted"] { color: @text2@; }
QLabel[role="title"] { font-size: 22px; font-weight: 600; color: @text@; }
QLabel[role="stat"]  { font-size: 26px; font-weight: 300; color: @text@; }

/* ---------- buttons ---------- */
QPushButton { background: @btnSecondary@; color: #FFFFFF; border: none; border-radius: 4px; padding: @padBtn@; font-weight: 500; }
QPushButton:hover   { background: @btnSecondaryHover@; }
QPushButton:pressed { background: @btnSecondaryActive@; }
QPushButton:focus   { outline: none; border: 2px solid @accent@; padding: @padBtnFocus@; }
QPushButton:checked  { background: @btnPrimary@; }
QPushButton:disabled { background: @disabledBg@; color: @disabledText@; }

QPushButton[role="primary"]         { background: @btnPrimary@; }
QPushButton[role="primary"]:hover   { background: @btnPrimaryHover@; }
QPushButton[role="primary"]:pressed { background: @btnPrimaryActive@; }
QPushButton[role="danger"]          { background: @danger@; }
QPushButton[role="danger"]:hover    { background: @dangerHover@; }
QPushButton[role="danger"]:pressed  { background: @dangerActive@; }
QPushButton[role="ghost"]           { background: transparent; color: @accent@; padding: 6px 10px; }
QPushButton[role="ghost"]:hover     { background: @navHover@; }
QPushButton[role="icon"]            { background: transparent; border: none; padding: 0; }
QPushButton[role="icon"]:hover      { background: transparent; }
QPushButton[role="icon"]:focus, QPushButton[role="statusbtn"]:focus { border: none; padding: 0; }
QPushButton[role="ghost"]:focus     { border: 2px solid @accent@; padding: 4px 8px; }
QPushButton[role="statusbtn"]       { background: transparent; color: @shellText@; padding: 2px 10px; border-radius: 3px; }
QPushButton[role="statusbtn"]:hover { background: #393939; color: #FFFFFF; }

QPushButton[role="nav"] { background: transparent; color: @navText@; text-align: left; padding: @padNav@; border: none; border-left: 3px solid transparent; border-radius: 0; font-weight: 400; }
QPushButton[role="nav"]:hover  { background: @navHover@; color: @text@; }
QPushButton[role="nav"]:focus  { border: none; border-left: 3px solid transparent; padding: @padNav@; }
QPushButton[role="nav"][active="true"] { background: @navActive@; color: @accent@; border-left: 3px solid @accent@; font-weight: 600; }

QPushButton#toggle_button_main_circle, QPushButton#toggle_button_admin_circle { background: @btnPrimary@; border-radius: 15px; padding: 0; }

/* ---------- inputs ---------- */
QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QDateTimeEdit, QTimeEdit, QTextEdit, QPlainTextEdit {
    background: @field@; color: @text@; border: 1px solid transparent; border-bottom: 1px solid @borderStrong@;
    border-radius: 0; padding: @padField@; min-height: 20px;
    selection-background-color: @btnPrimary@; selection-color: #FFFFFF; }
QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus,
QDateTimeEdit:focus, QTimeEdit:focus, QTextEdit:focus, QPlainTextEdit:focus { border: 2px solid @accent@; padding: @padFieldFocus@; }
QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled { color: @disabledText@; background: @bg@; }
QComboBox::drop-down { border: none; width: 28px; }
QComboBox QAbstractItemView { background: @surface@; color: @text@; border: 1px solid @border@; outline: 0;
    selection-background-color: @btnPrimary@; selection-color: #FFFFFF; }

QCheckBox::indicator, QRadioButton::indicator { width: 16px; height: 16px; border: 1px solid @borderStrong@; background: @field@; border-radius: 2px; }
QRadioButton::indicator { border-radius: 9px; }
QCheckBox::indicator:hover, QRadioButton::indicator:hover { border-color: @accent@; }
QCheckBox::indicator:checked, QRadioButton::indicator:checked { background: @btnPrimary@; border-color: @btnPrimary@; }

/* ---------- tables ---------- */
QTableView, QTableWidget, QListView, QListWidget, QTreeView {
    background: @surface@; alternate-background-color: @rowAlt@; color: @text@;
    border: 1px solid @border@; gridline-color: @border@; outline: 0;
    selection-background-color: @selection@; selection-color: @text@; }
QTableView::item, QListView::item { padding: 6px 10px; border: none; }
QTableView::item:hover, QListView::item:hover { background: @rowHover@; }
QTableView::item:selected, QListView::item:selected { background: @selection@; color: @text@; }
QHeaderView::section { background: @headerBg@; color: @text@; padding: 10px 12px; border: none; border-right: 1px solid @surface@; font-weight: 600; }
QTableCornerButton::section { background: @headerBg@; border: none; }

/* ---------- slider ---------- */
QSlider::groove:horizontal { height: 4px; background: @border@; border-radius: 2px; }
QSlider::sub-page:horizontal { background: @btnPrimary@; border-radius: 2px; }
QSlider::handle:horizontal { background: @btnPrimary@; width: 16px; height: 16px; margin: -6px 0; border-radius: 8px; }

/* ---------- tabs / group / progress ---------- */
QTabWidget::pane { border: none; border-top: 1px solid @border@; background: transparent; }
QTabBar::tab { background: transparent; color: @text2@; padding: @padTab@; border: none; border-bottom: 2px solid transparent; }
QTabBar::tab:hover { color: @text@; background: @navHover@; }
QTabBar::tab:selected { color: @text@; border-bottom: 2px solid @accent@; font-weight: 600; }
QGroupBox { border: 1px solid @border@; border-radius: 4px; margin-top: 14px; padding-top: 10px; color: @text@; }
QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }
QProgressBar { background: @border@; border: none; border-radius: 2px; height: 4px; text-align: center; color: transparent; }
QProgressBar::chunk { background: @btnPrimary@; border-radius: 2px; }

/* ---------- status / menus ---------- */
QStatusBar { background: @shell@; color: @shellText@; }
QStatusBar QLabel { color: @shellText@; }
QStatusBar::item { border: none; }
QMenu { background: @surface@; color: @text@; border: 1px solid @border@; padding: 4px; }
QMenu::item { padding: 8px 24px; }
QMenu::item:selected { background: @btnPrimary@; color: #FFFFFF; }

/* ---------- scrollbars ---------- */
QScrollBar:vertical   { background: transparent; width: 10px; margin: 0; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 0; }
QScrollBar::handle:vertical   { background: @thumb@; min-height: 32px; border-radius: 4px; margin: 2px; }
QScrollBar::handle:horizontal { background: @thumb@; min-width: 32px; border-radius: 4px; margin: 2px; }
QScrollBar::handle:hover { background: @borderStrong@; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

/* ---------- command palette + toast (built in code) ---------- */
QDialog[role="palette"] { background: @surface@; border: 1px solid @borderStrong@; }
QDialog[role="palette"] QLineEdit { font-size: 15px; padding: 12px 14px; }
QDialog[role="palette"] QListWidget { border: none; border-top: 1px solid @border@; }
QDialog[role="palette"] QListWidget::item { padding: 10px 14px; }
QDialog[role="palette"] QListWidget::item:selected { background: @btnPrimary@; color: #FFFFFF; }
QLabel[role="chip-ok"]   { background: @okBg@;   color: @ok@;   border-radius: 10px; padding: 2px 10px; font-weight: 600; }
QLabel[role="chip-warn"] { background: @warnBg@; color: @warn@; border-radius: 10px; padding: 2px 10px; font-weight: 600; }
QLabel[role="chip-bad"]  { background: @badBg@;  color: @bad@;  border-radius: 10px; padding: 2px 10px; font-weight: 600; }
)QSS");

    const QMap<QString, QString> t = tokens();
    for (auto it = t.constBegin(); it != t.constEnd(); ++it)
        q.replace("@" + it.key() + "@", it.value());
    return q;
}

void ThemeManager::rebuild()
{
    qApp->setStyleSheet(buildStyleSheet());
}

// ---------------------------------------------------------------------------
// Application-wide hook: first show of a top-level window
// ---------------------------------------------------------------------------
bool ThemeManager::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Show && obj->isWidgetType()) {
        QWidget *w = static_cast<QWidget *>(obj);
        const Qt::WindowType type = static_cast<Qt::WindowType>(int(w->windowFlags() & Qt::WindowType_Mask));
        const bool normalWindow = (type == Qt::Window || type == Qt::Dialog);

        if (w->isWindow() && normalWindow
            && !w->property("ims_skip").toBool()
            && !w->property("ims_prepared").toBool()) {
            prepare(w);
            Fx::fadeInWindow(w);
        }
    }
    return QObject::eventFilter(obj, event);
}

// ---------------------------------------------------------------------------
// Legacy UI migration
// ---------------------------------------------------------------------------
void ThemeManager::prepare(QWidget *root)
{
    if (!root || root->property("ims_prepared").toBool())
        return;
    root->setProperty("ims_prepared", true);

    // 1. Drop the inline stylesheets baked into the .ui files.
    root->setStyleSheet(QString());
    const QList<QWidget *> all = root->findChildren<QWidget *>();
    for (QWidget *w : all) {
        if (!isManaged(w) && !w->styleSheet().isEmpty())
            w->setStyleSheet(QString());
    }

    // 2. Semantic roles that the stylesheet understands.
    classifyButtons(root);
    classifyLabels(root);
    classifyFrames(root);

    // 3. Tables get a calmer default look.
    const QList<QTableView *> tables = root->findChildren<QTableView *>();
    for (QTableView *tv : tables) {
        if (isManaged(tv))
            continue;
        tv->setAlternatingRowColors(true);
        tv->setShowGrid(false);
        tv->horizontalHeader()->setHighlightSections(false);
    }

    repolish(root);
    for (QWidget *w : all)
        repolish(w);
}

void ThemeManager::classifyButtons(QWidget *root)
{
    // object name -> blue (active/hover) icon for the sidebar
    static const QMap<QString, QString> navIcons = {
        {"dashboard_btn",    ":/resources/dashboard-icons/dashboard-blue.svg"},
        {"product_btn",      ":/resources/dashboard-icons/product-blue.svg"},
        {"supplier_btn",     ":/resources/dashboard-icons/supplier-blue.svg"},
        {"orders_btn",       ":/resources/dashboard-icons/orders-blue.svg"},
        {"adjust_stock_btn", ":/resources/dashboard-icons/adjust_stock-blue.svg"},
        {"notification_btn", ":/resources/dashboard-icons/notification-blue.svg"},
        {"activity_btn",     ":/resources/dashboard-icons/activity-blue.svg"},
        {"promotion_btn",    ":/resources/dashboard-icons/promotion-blue.svg"},
        {"users_btn",        ":/resources/dashboard-icons/user-blue.svg"},
        {"settings_btn",     ":/resources/dashboard-icons/settings-blue.svg"},
        {"explorer_btn",     ":/resources/product-icons/advance_view.svg"},
        {"overview_btn",     ":/resources/dashboard-icons/team-blue.svg"},
        {"datacenter_btn",   ":/resources/product-icons/inventry-value.svg"},
    };

    const QRegularExpression dangerRx("delete|remove|terminate|cancel order",
                                      QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression primaryRx("^(add|create|save|submit|login|log in|sign|register|apply|confirm|update|reset|send|verify|continue|next|place|load|generate|export)",
                                       QRegularExpression::CaseInsensitiveOption);

    const bool isLogin = (root->objectName() == "MainWindow");

    const QList<QPushButton *> buttons = root->findChildren<QPushButton *>();
    for (QPushButton *b : buttons) {
        if (isManaged(b))
            continue;
        b->setCursor(Qt::PointingHandCursor);
        const QString name = b->objectName();
        const QString text = b->text().trimmed();

        if (navIcons.contains(name)) {
            b->setProperty("role", "nav");
            b->setProperty("active", false);
            b->setProperty("ims_icon_base", QVariant::fromValue(b->icon()));
            b->setProperty("ims_icon_active", QVariant::fromValue(QIcon(navIcons.value(name))));
            b->setIconSize(QSize(22, 22));
            Fx::installNavHover(b);
            if (b->property("active").toBool())
                b->setIcon(QIcon(navIcons.value(name)));
        } else if (text.isEmpty() && !b->icon().isNull()) {
            b->setProperty("role", "icon");
        } else if (isLogin && (name == "pushButton" || name == "forgot_password_btn")) {
            b->setProperty("role", "ghost");
        } else if (name == "toggle_button_main_circle" || name == "toggle_button_admin_circle") {
            // styled by id in the stylesheet
        } else if (dangerRx.match(text).hasMatch() || dangerRx.match(name).hasMatch()) {
            b->setProperty("role", "danger");
        } else if (primaryRx.match(text).hasMatch()) {
            b->setProperty("role", "primary");
        }
        // everything else keeps the secondary (dark grey) default
    }
}

void ThemeManager::classifyLabels(QWidget *root)
{
    static const QRegularExpression genericName("^label(_\\d+)?$");
    static const QRegularExpression hasDigit("\\d");

    const QList<QLabel *> labels = root->findChildren<QLabel *>();
    for (QLabel *l : labels) {
        if (isManaged(l))
            continue;
        const QString text = l->text();
        if (!text.isEmpty() && !genericName.match(l->objectName()).hasMatch()
            && hasDigit.match(text).hasMatch() && l->pixmap(Qt::ReturnByValue).isNull()) {
            l->setProperty("role", "kpi");
        }
    }
}

void ThemeManager::classifyFrames(QWidget *root)
{
    auto isPlainFrame = [](QObject *o) {
        return QString::fromLatin1(o->metaObject()->className()) == QLatin1String("QFrame");
    };

    QFrame *hero = nullptr;
    if (root->objectName() == "MainWindow")
        hero = root->findChild<QFrame *>("horizontalFrame");

    int kpiCards = 0;   // cycles the coloured accent stripe on KPI cards
    const QList<QFrame *> frames = root->findChildren<QFrame *>();
    for (QFrame *f : frames) {
        if (!isPlainFrame(f) || isManaged(f))
            continue;
        if (f->frameShape() == QFrame::HLine || f->frameShape() == QFrame::VLine)
            continue;   // keep separators as thin lines

        if (f == hero) {
            f->setProperty("role", "hero");
            continue;
        }
        if (f->objectName() == "toggle_frame") {
            f->setProperty("role", "pill");
            continue;
        }

        // sidebar = frame that holds nav buttons
        bool hasNav = false;
        const QList<QPushButton *> btns = f->findChildren<QPushButton *>();
        for (QPushButton *b : btns) {
            if (b->property("role").toString() == "nav") { hasNav = true; break; }
        }
        if (hasNav) {
            f->setProperty("role", "sidebar");
            continue;
        }

        // nested plain frames -> transparent layout panel
        bool nested = false;
        const QList<QFrame *> kids = f->findChildren<QFrame *>();
        for (QFrame *k : kids) {
            if (isPlainFrame(k)) { nested = true; break; }
        }
        if (nested) {
            f->setProperty("role", "panel");
            continue;
        }

        // single input row (icon + line edit) -> Carbon "field"
        const int inputs = f->findChildren<QLineEdit *>().size() + f->findChildren<QComboBox *>().size();
        const int direct = f->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly).size();
        if (inputs == 1 && direct <= 3) {
            f->setProperty("role", "field");
            continue;
        }
        if (inputs > 1) {
            f->setProperty("role", "panel");
            continue;
        }

        // leaf container -> card
        f->setProperty("role", "card");

        // KPI cards get a hover lift
        const QList<QLabel *> labs = f->findChildren<QLabel *>();
        for (QLabel *l : labs) {
            if (l->property("role").toString() == "kpi") {
                f->setProperty("accent", QString::number(kpiCards++ % 4));
                Fx::installHoverLift(f);
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Charts
// ---------------------------------------------------------------------------
void ThemeManager::styleChart(QChart *chart)
{
    if (!chart)
        return;
    const ThemeManager &tm = ThemeManager::instance();
    const QColor text = tm.color("text");
    const QColor text2 = tm.color("text2");
    const QColor grid = tm.color("border");
    const QColor axisLine = tm.color("borderStrong");
    const QColor primary = tm.color("btnPrimary");

    chart->setBackgroundBrush(QBrush(Qt::transparent));
    chart->setBackgroundPen(Qt::NoPen);
    chart->setBackgroundRoundness(0);
    chart->setTitleBrush(QBrush(text));
    QFont tf = chart->titleFont();
    tf.setPointSize(11);
    tf.setBold(true);
    chart->setTitleFont(tf);
    chart->legend()->setLabelColor(text2);
    chart->legend()->setAlignment(Qt::AlignBottom);

    chart->setAnimationOptions(tm.animationsEnabled() ? QChart::SeriesAnimations : QChart::NoAnimation);
    chart->setAnimationDuration(900);
    chart->setAnimationEasingCurve(QEasingCurve(QEasingCurve::OutCubic));

    const QList<QAbstractAxis *> axes = chart->axes();
    for (QAbstractAxis *a : axes) {
        a->setLabelsBrush(QBrush(text2));
        a->setTitleBrush(QBrush(text));
        a->setGridLinePen(QPen(grid, 1));
        a->setLinePen(QPen(axisLine, 1));
    }

    const QList<QAbstractSeries *> series = chart->series();
    for (QAbstractSeries *s : series) {
        if (auto *line = qobject_cast<QLineSeries *>(s)) {
            line->setPen(QPen(primary, 2.5));
            line->setPointsVisible(true);
        } else if (auto *bars = qobject_cast<QAbstractBarSeries *>(s)) {
            const QList<QBarSet *> sets = bars->barSets();
            for (QBarSet *set : sets) {
                set->setColor(primary);
                set->setBorderColor(Qt::transparent);
            }
            bars->setBarWidth(0.55);
        }
    }
}
