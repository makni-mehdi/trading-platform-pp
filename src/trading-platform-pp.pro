QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

linux-g++* {
  LIBS += -lcurl
}

win32 {
  # remember to put libcurl-x64 to mingw folder
  LIBS += C:/curl-7.74.0-win64-mingw/lib/libcurl.a \
          C:/curl-7.74.0-win64-mingw/lib/libcurl.dll.a
  INCLUDEPATH += C:/curl-7.74.0-win64-mingw/include
}

SOURCES += \
    api/api.cpp \
    components/news.cpp \
    components/stock.cpp \
    components/portfolio.cpp \
    components/trading_order.cpp \
    gui/chooseportfolio.cpp \
    gui/companynewscard.cpp \
    gui/homepage.cpp \
    gui/newscard.cpp \
    gui/newspage.cpp \
    gui/signup.cpp \
    gui/stockdashboard.cpp \
    gui/stockgraph.cpp \
    gui/stockgraphoneday.cpp \
    gui/stockgraphonemonth.cpp \
    gui/stockgraphsixmonths.cpp \
    gui/stockgraphthreedays.cpp \
    main.cpp \
    gui/mainwindow.cpp \
    library/qcustomplot.cpp \
    helper/helper.cpp \
    gui/widgets/selectwatchlistwidget.cpp \
    gui/neworder.cpp

HEADERS += \
    api/api.h \
    components/news.h \
    components/stock.h \
    components/portfolio.h \
    components/trading_order.h \
    gui/chooseportfolio.h \
    gui/companynewscard.h \
    gui/homepage.h \
    gui/newscard.h \
    gui/newspage.h \
    gui/signup.h \
    gui/stockdashboard.h \
    gui/stockgraph.h \
    gui/stockgraphoneday.h \
    gui/stockgraphonemonth.h \
    gui/stockgraphsixmonths.h \
    gui/stockgraphthreedays.h \
    library/qcustomplot.h \
    gui/neworder.h \
    gui/mainwindow.h \
    gui/widgets/selectwatchlistwidget.h \
    helper/helper.h
    
FORMS += \
    gui/chooseportfolio.ui \
    gui/companynewscard.ui \
    gui/homepage.ui \
    gui/mainwindow.ui \
    gui/signup.ui \
    gui/stockdashboard.ui \
    gui/stockgraph.ui \
    gui/newscard.ui \
    gui/newspage.ui \
    gui/neworder.ui \
    gui/widgets/selectwatchlistwidget.ui

DISTFILES += \
    style.astylerc

RESOURCES += \
    resources/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
