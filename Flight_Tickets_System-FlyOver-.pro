QT       += core gui sql  # 核心库、GUI库、数据库模块（MySQL支持）

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets  # 兼容Qt5及以上版本的Widgets

TARGET = Flight_Tickets_System-FlyOver-  # 可执行程序名称（与项目名一致）
TEMPLATE = app

# 源文件列表（按实际目录结构填写）
SOURCES += \
    main.cpp \
    mainwindow.cpp
    # login.cpp \
    # register.cpp \
    # flightmanager.cpp \
    # dbmanager.cpp \
    # models/flight.cpp \
    # models/order.cpp \
    # models/user.cpp

# 头文件列表
HEADERS += \
    mainwindow.h
    # login.h \
    # register.h \
    # flightmanager.h \
    # dbmanager.h \
    # models/flight.h \
    # models/order.h \
    # models/user.h

# UI界面文件（Qt Designer生成）
FORMS += \
    mainwindow.ui
    # login.ui \
    # register.ui \
    # flightmanager.ui

# 资源文件（样式表、图标等）
# RESOURCES += resources/resources.qrc  # 若需添加图标，可在resources下创建icons目录并更新此处

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
# RESOURCES += resources/resources.qrc
# RESOURCES += resources/style.qss
