#ifndef IMS_APPEARANCE_H
#define IMS_APPEARANCE_H

#include <QDialog>

class QPushButton;
class QSlider;
class QLabel;
class QCheckBox;
class QButtonGroup;

/*
 * AppearanceDialog - live controls for the look of the whole application:
 * theme, accent colour, density, text size and motion. Every change is applied
 * immediately and remembered between launches. Open with Ctrl+, .
 */
class AppearanceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AppearanceDialog(QWidget *parent = nullptr);

private:
    void sync();

    QPushButton *m_light = nullptr;
    QPushButton *m_dark = nullptr;
    QPushButton *m_comfortable = nullptr;
    QPushButton *m_compact = nullptr;
    QList<QPushButton *> m_swatches;
    QSlider *m_font = nullptr;
    QLabel *m_fontValue = nullptr;
    QCheckBox *m_motion = nullptr;
};

#endif // IMS_APPEARANCE_H
