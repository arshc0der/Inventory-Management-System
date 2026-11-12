#ifndef IMS_PALETTE_H
#define IMS_PALETTE_H

#include <QDialog>
#include <QList>
#include <QString>
#include <functional>

class QLineEdit;
class QListWidget;

/*
 * CommandPalette - Ctrl+K quick launcher (think VS Code / Slack).
 * Type a few letters, arrow keys to pick, Enter to run.
 */
class CommandPalette : public QDialog
{
    Q_OBJECT
public:
    struct Command {
        QString title;                  // "Go to Products"
        QString hint;                   // "Navigate"
        std::function<void()> action;
    };

    explicit CommandPalette(QWidget *parent, const QList<Command> &commands);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void applyFilter(const QString &text);
    void runCurrent();

    QLineEdit *m_edit = nullptr;
    QListWidget *m_list = nullptr;
    QList<Command> m_commands;
};

#endif // IMS_PALETTE_H
