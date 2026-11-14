#ifndef IMS_THEME_H
#define IMS_THEME_H

#include <QObject>
#include <QColor>
#include <QString>
#include <QMap>

class QWidget;
class QChart;
class QEvent;

/*
 * ThemeManager
 * ------------
 * Enterprise design system for IMS, modelled on IBM's Carbon language:
 *   - IBM Blue (#0F62FE) as the single interactive colour
 *   - flat surfaces, 1px hairline borders, bottom-border form fields
 *   - a "White" (light) theme and a "Gray 100" (dark) theme, persisted in QSettings
 *
 * It does three jobs:
 *   1. Builds one application-wide stylesheet from design tokens.
 *   2. Installs an application event filter. The first time any top-level
 *      window is shown it removes the legacy per-widget inline stylesheets that
 *      were baked into the .ui files, tags widgets with a semantic "role"
 *      property (card, nav, primary, danger, ...) that the stylesheet targets,
 *      and fades the window in.
 *   3. Styles QtCharts so charts follow the active theme and animate.
 */
class ThemeManager : public QObject
{
    Q_OBJECT
public:
    enum Mode { Light, Dark };
    enum Density { Comfortable, Compact };

    static ThemeManager &instance();

    // Call once, right after QApplication is created.
    void install();

    Mode mode() const { return m_mode; }
    bool isDark() const { return m_mode == Dark; }
    void setMode(Mode m);
    void toggle();

    // ---- user-adjustable appearance (all persisted, all applied live) ----
    QColor accentColor() const { return m_accent; }
    void setAccentColor(const QColor &c);
    Density density() const { return m_density; }
    void setDensity(Density d);
    int fontSize() const { return m_fontSize; }
    void setFontSize(int points);
    bool animationsEnabled() const { return m_animations; }
    void setAnimationsEnabled(bool on);
    int rowHeight() const { return m_density == Compact ? 30 : 40; }
    void resetAppearance();

    // Colour token lookup, e.g. color("primary").
    QColor color(const QString &token) const;

    // Remove legacy inline styles + tag semantic roles. Safe to call twice.
    void prepare(QWidget *root);

    // Apply theme colours, fonts and animation to a chart.
    static void styleChart(QChart *chart);

    // Re-evaluate the stylesheet on one widget after changing a dynamic property.
    static void repolish(QWidget *w);

signals:
    void themeChanged(bool dark);
    void appearanceChanged();      // any appearance setting changed (theme, accent, density, font, motion)

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    ThemeManager();
    void rebuild();
    QString buildStyleSheet() const;
    QMap<QString, QString> tokens() const;

    void classifyButtons(QWidget *root);
    void classifyFrames(QWidget *root);
    void classifyLabels(QWidget *root);

    void saveSettings() const;
    void applyFont();

    Mode m_mode = Light;
    QColor m_accent = QColor("#0F62FE");
    Density m_density = Comfortable;
    int m_fontSize = 10;
    bool m_animations = true;
    bool m_installed = false;
};

#endif // IMS_THEME_H
