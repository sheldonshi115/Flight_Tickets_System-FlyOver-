/********************************************************************************
** Form generated from reading UI file 'flightmanager.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FLIGHTMANAGER_H
#define UI_FLIGHTMANAGER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FlightManager
{
public:

    void setupUi(QWidget *FlightManager)
    {
        if (FlightManager->objectName().isEmpty())
            FlightManager->setObjectName("FlightManager");
        FlightManager->resize(800, 600);

        retranslateUi(FlightManager);

        QMetaObject::connectSlotsByName(FlightManager);
    } // setupUi

    void retranslateUi(QWidget *FlightManager)
    {
        FlightManager->setWindowTitle(QCoreApplication::translate("FlightManager", "Flight Manager", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FlightManager: public Ui_FlightManager {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FLIGHTMANAGER_H
