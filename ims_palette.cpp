#include "ims_palette.h"
#include "ims_fx.h"

#include <QApplication>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QShowEvent>
#include <QTimer>
#include <QStringList>

CommandPalette::CommandPalette(QWidget *parent, const QList<Command> &commands)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint)
    , m_commands(commands)
{
    setProperty("role", "palette");
    setProperty("ims_skip", true);      // no legacy-style stripping, we fade ourselves
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setFixedSize(580, 400);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_edit = new QLineEdit(this);
    m_edit->setPlaceholderText(tr("Type a command or page name..."));
    m_edit->installEventFilter(this);
    lay->addWidget(m_edit);

    m_list = new QListWidget(this);
    m_list->setFocusPolicy(Qt::NoFocus);
    lay->addWidget(m_list, 1);

    connect(m_edit, &QLineEdit::textChanged, this, [this](const QString &t) { applyFilter(t); });
    connect(m_list, &QListWidget::itemActivated, this, [this]() { runCurrent(); });
    connect(m_list, &QListWidget::itemClicked, this, [this]() { runCurrent(); });

    applyFilter(QString());
}

void CommandPalette::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (QWidget *p = parentWidget()) {
        const QPoint topLeft = p->mapToGlobal(QPoint((p->width() - width()) / 2, 90));
        move(topLeft);
    }
    m_edit->setFocus();
    Fx::fadeInWindow(this, 160);
}

void CommandPalette::applyFilter(const QString &text)
{
    m_list->clear();
    const QStringList tokens = text.toLower().split(' ', Qt::SkipEmptyParts);

    for (int i = 0; i < m_commands.size(); ++i) {
        const Command &c = m_commands.at(i);
        const QString hay = (c.title + " " + c.hint).toLower();
        bool all = true;
        for (const QString &tok : tokens) {
            if (!hay.contains(tok)) { all = false; break; }
        }
        if (!all)
            continue;

        auto *item = new QListWidgetItem(c.hint.isEmpty() ? c.title
                                                          : c.title + "      \u00B7  " + c.hint,
                                         m_list);
        item->setData(Qt::UserRole, i);
    }
    if (m_list->count() > 0)
        m_list->setCurrentRow(0);
}

void CommandPalette::runCurrent()
{
    QListWidgetItem *item = m_list->currentItem();
    if (!item)
        return;
    const int idx = item->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= m_commands.size())
        return;

    const std::function<void()> fn = m_commands.at(idx).action;
    close();
    if (fn)
        QTimer::singleShot(0, qApp, [fn]() { fn(); });
}

bool CommandPalette::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_edit && event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(event);
        const int count = m_list->count();
        switch (ke->key()) {
        case Qt::Key_Down:
            if (count > 0)
                m_list->setCurrentRow((m_list->currentRow() + 1) % count);
            return true;
        case Qt::Key_Up:
            if (count > 0)
                m_list->setCurrentRow((m_list->currentRow() - 1 + count) % count);
            return true;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            runCurrent();
            return true;
        default:
            break;
        }
    }
    return QDialog::eventFilter(obj, event);
}
