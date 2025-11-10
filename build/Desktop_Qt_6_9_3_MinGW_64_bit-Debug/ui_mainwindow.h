/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionAddFlight;
    QAction *actionManageFlights;
    QAction *actionViewOrders;
    QAction *actionProfile;
    QAction *actionLogout;
    QAction *actionAbout;
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QFrame *sideBar;
    QVBoxLayout *sideLayout;
    QLabel *logoLabel;
    QPushButton *btnHome;
    QPushButton *btnFlights;
    QPushButton *btnOrders;
    QPushButton *btnProfile;
    QStackedWidget *stackedWidget;
    QWidget *pageHome;
    QVBoxLayout *homeLayout;
    QLabel *labelWelcome;
    QWidget *pageFlights;
    QWidget *pageOrders;
    QWidget *pageProfile;
    QMenuBar *menuBar;
    QMenu *menuFlights;
    QMenu *menuOrders;
    QMenu *menuUser;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1000, 700);
        actionAddFlight = new QAction(MainWindow);
        actionAddFlight->setObjectName("actionAddFlight");
        actionManageFlights = new QAction(MainWindow);
        actionManageFlights->setObjectName("actionManageFlights");
        actionViewOrders = new QAction(MainWindow);
        actionViewOrders->setObjectName("actionViewOrders");
        actionProfile = new QAction(MainWindow);
        actionProfile->setObjectName("actionProfile");
        actionLogout = new QAction(MainWindow);
        actionLogout->setObjectName("actionLogout");
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName("actionAbout");
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        mainLayout = new QHBoxLayout(centralWidget);
        mainLayout->setSpacing(6);
        mainLayout->setContentsMargins(11, 11, 11, 11);
        mainLayout->setObjectName("mainLayout");
        sideBar = new QFrame(centralWidget);
        sideBar->setObjectName("sideBar");
        sideBar->setFrameShape(QFrame::Shape::StyledPanel);
        sideLayout = new QVBoxLayout(sideBar);
        sideLayout->setSpacing(6);
        sideLayout->setContentsMargins(11, 11, 11, 11);
        sideLayout->setObjectName("sideLayout");
        logoLabel = new QLabel(sideBar);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; color: #0078d7;"));
        logoLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        sideLayout->addWidget(logoLabel);

        btnHome = new QPushButton(sideBar);
        btnHome->setObjectName("btnHome");

        sideLayout->addWidget(btnHome);

        btnFlights = new QPushButton(sideBar);
        btnFlights->setObjectName("btnFlights");

        sideLayout->addWidget(btnFlights);

        btnOrders = new QPushButton(sideBar);
        btnOrders->setObjectName("btnOrders");

        sideLayout->addWidget(btnOrders);

        btnProfile = new QPushButton(sideBar);
        btnProfile->setObjectName("btnProfile");

        sideLayout->addWidget(btnProfile);


        mainLayout->addWidget(sideBar);

        stackedWidget = new QStackedWidget(centralWidget);
        stackedWidget->setObjectName("stackedWidget");
        pageHome = new QWidget();
        pageHome->setObjectName("pageHome");
        homeLayout = new QVBoxLayout(pageHome);
        homeLayout->setSpacing(6);
        homeLayout->setContentsMargins(11, 11, 11, 11);
        homeLayout->setObjectName("homeLayout");
        labelWelcome = new QLabel(pageHome);
        labelWelcome->setObjectName("labelWelcome");
        labelWelcome->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; margin-top: 80px;"));
        labelWelcome->setAlignment(Qt::AlignmentFlag::AlignCenter);

        homeLayout->addWidget(labelWelcome);

        stackedWidget->addWidget(pageHome);
        pageFlights = new QWidget();
        pageFlights->setObjectName("pageFlights");
        stackedWidget->addWidget(pageFlights);
        pageOrders = new QWidget();
        pageOrders->setObjectName("pageOrders");
        stackedWidget->addWidget(pageOrders);
        pageProfile = new QWidget();
        pageProfile->setObjectName("pageProfile");
        stackedWidget->addWidget(pageProfile);

        mainLayout->addWidget(stackedWidget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 1000, 33));
        menuFlights = new QMenu(menuBar);
        menuFlights->setObjectName("menuFlights");
        menuOrders = new QMenu(menuBar);
        menuOrders->setObjectName("menuOrders");
        menuUser = new QMenu(menuBar);
        menuUser->setObjectName("menuUser");
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName("menuHelp");
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName("mainToolBar");
        MainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFlights->menuAction());
        menuBar->addAction(menuOrders->menuAction());
        menuBar->addAction(menuUser->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFlights->addAction(actionAddFlight);
        menuFlights->addAction(actionManageFlights);
        menuOrders->addAction(actionViewOrders);
        menuUser->addAction(actionProfile);
        menuUser->addAction(actionLogout);
        menuHelp->addAction(actionAbout);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\350\210\252\347\217\255\347\245\250\345\212\241\347\256\241\347\220\206\347\263\273\347\273\237 - FlyOver", nullptr));
        actionAddFlight->setText(QCoreApplication::translate("MainWindow", "\346\267\273\345\212\240\350\210\252\347\217\255", nullptr));
        actionManageFlights->setText(QCoreApplication::translate("MainWindow", "\350\210\252\347\217\255\345\210\227\350\241\250", nullptr));
        actionViewOrders->setText(QCoreApplication::translate("MainWindow", "\346\237\245\347\234\213\350\256\242\345\215\225", nullptr));
        actionProfile->setText(QCoreApplication::translate("MainWindow", "\344\270\252\344\272\272\344\277\241\346\201\257", nullptr));
        actionLogout->setText(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272\347\231\273\345\275\225", nullptr));
        actionAbout->setText(QCoreApplication::translate("MainWindow", "\345\205\263\344\272\216\347\263\273\347\273\237", nullptr));
        logoLabel->setText(QCoreApplication::translate("MainWindow", "\342\234\210\357\270\217 FlyOver", nullptr));
        btnHome->setText(QCoreApplication::translate("MainWindow", "\360\237\217\240 \344\270\273\351\241\265", nullptr));
        btnFlights->setText(QCoreApplication::translate("MainWindow", "\342\234\210\357\270\217 \350\210\252\347\217\255\346\237\245\350\257\242", nullptr));
        btnOrders->setText(QCoreApplication::translate("MainWindow", "\360\237\247\276 \350\256\242\345\215\225\347\256\241\347\220\206", nullptr));
        btnProfile->setText(QCoreApplication::translate("MainWindow", "\360\237\221\244 \346\210\221\347\232\204\344\277\241\346\201\257", nullptr));
        labelWelcome->setText(QCoreApplication::translate("MainWindow", "\346\254\242\350\277\216\344\275\277\347\224\250\350\210\252\347\217\255\347\245\250\345\212\241\347\256\241\347\220\206\347\263\273\347\273\237 - FlyOver", nullptr));
        menuFlights->setTitle(QCoreApplication::translate("MainWindow", "\350\210\252\347\217\255\347\256\241\347\220\206(&F)", nullptr));
        menuOrders->setTitle(QCoreApplication::translate("MainWindow", "\350\256\242\345\215\225\347\256\241\347\220\206(&O)", nullptr));
        menuUser->setTitle(QCoreApplication::translate("MainWindow", "\347\224\250\346\210\267\344\270\255\345\277\203(&U)", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "\345\270\256\345\212\251(&H)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
