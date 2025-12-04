CONFIG += rtti  # Qt 内置的 RTTI 启用开关
QMAKE_CXXFLAGS += -frtti  # MinGW 编译器强制启用 RTTI（核心）
QMAKE_CXXFLAGS_DEBUG += -frtti
QMAKE_CXXFLAGS_RELEASE += -frtti
QMAKE_CXXFLAGS -= -fno-rtti
QMAKE_CXXFLAGS_DEBUG -= -fno-rtti
QMAKE_CXXFLAGS_RELEASE -= -fno-rtti
QT       += core gui sql network# 核心库、GUI库、数据库模块（MySQL支持）
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets  # 兼容Qt5及以上版本的Widgets

TARGET = Flight_Tickets_System-FlyOver-  # 可执行程序名称（与项目名一致）
TEMPLATE = app

# 源文件列表（按实际目录结构填写）
SOURCES += \
    flight.cpp \
    flightdialog.cpp \
    main.cpp \
    dbmanager.cpp \
    register.cpp \
    login.cpp \
    mainwindow.cpp \
    flightmanager.cpp \
    views/travelmoment.cpp \
    ai.cpp\

    # models/flight.cpp \
    # models/order.cpp \
    models/user.cpp

# 头文件列表
HEADERS += \
    clickablelabel.h \
    dbmanager.h \
    flight.h \
    flightdialog.h \
    register.h \
    login.h \
    mainwindow.h \
    flightmanager.h \
    clickablelabel.h \
    views/travelmoment.h \
    ai.h \
    # models/flight.h \
    # models/order.h \
    models/user.h

# UI界面文件
FORMS += \
    flightdialog.ui \
    register.ui \
    login.ui \
    mainwindow.ui \
    flightmanager.ui \
    ai.ui \
    views/travelmoment.ui \
    profileDisplaydialog.ui \
    profileRefreshdialog.ui \

# 资源文件（样式表、图标等）
RESOURCES += resources/resources.qrc  # 若需添加图标，可在resources下创建icons目录并更新此处
RESOURCES += resources/images
# 数据库驱动配置（确保MySQL连接）
# QT += sql
# unix: LIBS += -lmysqlclient  # Linux/Mac系统链接MySQL客户端库
# win32: {
#     LIBS += -llibmysql  # Windows系统链接MySQL库
#     # 若Windows下提示库找不到，可手动指定库路径（示例）：
#     # INCLUDEPATH += "C:/Program Files/MySQL/MySQL Server 8.0/include"
#     # LIBS += -L"C:/Program Files/MySQL/MySQL Server 8.0/lib" -llibmysql
# }

# 确保样式表文件被正确识别（可选）
RESOURCES += resources/style.qss
