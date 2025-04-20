QT       += core gui sql charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    add_supplier.cpp \
    addadjustment.cpp \
    addemployee.cpp \
    addorders.cpp \
    addproduct.cpp \
    addpromotion.cpp \
    addsales.cpp \
    checkdiscount.cpp \
    dashboard.cpp \
    deleteemployee.cpp \
    deleteorders.cpp \
    deleteproduct.cpp \
    deletepromotion.cpp \
    deletesales.cpp \
    forgot_password_dialog.cpp \
    globalfunctions.cpp \
    main.cpp \
    mainwindow.cpp \
    manageactivity.cpp \
    notification.cpp \
    orders_trends.cpp \
    products_hightlights_min_level_chart.cpp \
    products_loader.cpp \
    register.cpp \
    registercustomer.cpp \
    salestrends_chart.cpp \
    updateemployee.cpp \
    updateorders.cpp \
    updatepermissions.cpp \
    updateproduct.cpp \
    updatepromotion.cpp \
    updatesales.cpp

HEADERS += \
    DatabaseManager.h \
    DatabaseSchema.h \
    add_supplier.h \
    addadjustment.h \
    addemployee.h \
    addorders.h \
    addproduct.h \
    addpromotion.h \
    addsales.h \
    checkdiscount.h \
    dashboard.h \
    deleteemployee.h \
    deleteorders.h \
    deleteproduct.h \
    deletepromotion.h \
    deletesales.h \
    forgot_password_dialog.h \
    globalfunctions.h \
    mainwindow.h \
    manageactivity.h \
    notification.h \
    orders_trends.h \
    products_hightlights_min_level_chart.h \
    products_loader.h \
    register.h \
    registercustomer.h \
    salestrends_chart.h \
    updateemployee.h \
    updateorders.h \
    updatepermissions.h \
    updateproduct.h \
    updatepromotion.h \
    updatesales.h

FORMS += \
    add_supplier.ui \
    addadjustment.ui \
    addemployee.ui \
    addorders.ui \
    addproduct.ui \
    addpromotion.ui \
    addsales.ui \
    checkdiscount.ui \
    dashboard.ui \
    deleteemployee.ui \
    deleteorders.ui \
    deleteproduct.ui \
    deletepromotion.ui \
    deletesales.ui \
    forgot_password_dialog.ui \
    mainwindow.ui \
    manageactivity.ui \
    notification.ui \
    orders_trends.ui \
    products_hightlights_min_level_chart.ui \
    products_loader.ui \
    register.ui \
    registercustomer.ui \
    salestrends_chart.ui \
    updateemployee.ui \
    updateorders.ui \
    updatepermissions.ui \
    updateproduct.ui \
    updatepromotion.ui \
    updatesales.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
    RC_ICONS = logo.ico
