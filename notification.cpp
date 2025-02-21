#include "notification.h"
#include "ui_notification.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

Notification::Notification(const QString &title, const QString &message, const QString &noti_type, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Notification)
{
    ui->setupUi(this);
    ui->title->setText(title);
    ui->message->setText(message);

    if (noti_type == "info") {
        setStyleSheet("QFrame {"
                      "background-color: rgb(80, 70, 229);"
                      "color: white;"
                      "border-radius: 10px;"
                      "}"
                      "QPushButton {"
                      "background-color: rgb(237, 240, 247);"
                      "color: rgb(80, 70, 229);"
                      "border: none;"
                      "border-radius: 10px;"
                      "padding: 10px;"
                      "}"
                      "QPushButton:hover {"
                      "background-color: #919ae5;"
                      "}");
    } else if (noti_type == "risk") {
        setStyleSheet("QFrame {"
                      "background-color: #F43F5E;"
                      "color: white;"
                      "border-radius: 10px;"
                      "}"
                      "QPushButton {"
                      "background-color: rgb(237, 240, 247);"
                      "color: #F43F5E;"
                      "border: none;"
                      "border-radius: 10px;"
                      "padding: 10px;"
                      "}"
                      "QPushButton:hover {"
                      "background-color: #f4b1b4;"
                      "}");
    }

    // Set the initial position of the widget (off-screen to the right)
    this->move(this->x() + 500, this->y());
}

Notification::~Notification()
{
    delete ui;
}

void Notification::on_dismiss_btn_clicked()
{
    // Create an animation for the sliding effect
    QPropertyAnimation *slideOutAnimation = new QPropertyAnimation(this, "pos");
    slideOutAnimation->setDuration(500);  // Duration in milliseconds
    slideOutAnimation->setStartValue(this->pos()); // Current position
    slideOutAnimation->setEndValue(QPoint(this->x() + 500, this->y()));  // Slide to the right off-screen

    // Optionally, you can also add a fade-out effect using QGraphicsOpacityEffect
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(opacityEffect);
    QPropertyAnimation *fadeOutAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeOutAnimation->setDuration(500);
    fadeOutAnimation->setStartValue(1.0);  // Fully visible
    fadeOutAnimation->setEndValue(0.0);    // Fully transparent

    // Start the animations
    slideOutAnimation->start();
    fadeOutAnimation->start();

    // After the animation finishes, close the widget
    connect(slideOutAnimation, &QPropertyAnimation::finished, this, &QWidget::close);
}

