#include "ordermanager.h"
#include "ui_ordermanager.h"
#include "dbmanager.h"
#include <QMessageBox>
#include <QStackedWidget>
#include <QHeaderView>
#include <QDateTime>
#include <QUuid>
#include <QDebug>
#include <QTimer> // 新增：延迟恢复按钮

OrderManager::OrderManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderManager)
{
    ui->setupUi(this);

    // 1. 初始化表格
    setupTableView();
    // 2. 初始加载所有订单
    refreshOrderList();

    // 3. 设置日期默认值+格式
    ui->deOrderDate->setDate(QDate::currentDate());
    ui->deOrderDate->setDisplayFormat("yyyy-MM-dd");

    // 4. 绑定信号槽（修改取消订单按钮绑定，添加手动标记）
    connect(ui->btnQuery, &QPushButton::clicked, this, &OrderManager::on_btnQuery_clicked);
    connect(ui->btnReset, &QPushButton::clicked, this, &OrderManager::on_btnReset_clicked);
    // 修复：绑定取消订单按钮时标记为手动触发
    connect(ui->btnCancelOrder, &QPushButton::clicked, this, [this]() {
        m_isCancelManual = true;
        on_btnCancelOrder_clicked();
    });
    connect(ui->btnViewDetail, &QPushButton::clicked, this, &OrderManager::on_btnViewDetail_clicked);
    connect(ui->btnExit, &QPushButton::clicked, this, &OrderManager::on_btnExit_clicked);
    connect(ui->twOrderList, &QTableWidget::itemSelectionChanged,
            this, &OrderManager::on_twOrderList_itemSelectionChanged);

    // 初始同步按钮状态
    on_twOrderList_itemSelectionChanged();
}

OrderManager::~OrderManager()
{
    delete ui;
}

void OrderManager::refreshOrderList()
{
    QList<Order> allOrders = DBManager::instance().getAllOrders();
    qDebug() << "从数据库加载订单数量：" << allOrders.size();
    loadOrdersToTable(allOrders);
}

void OrderManager::setupTableView()
{
    ui->twOrderList->setColumnCount(9);
    ui->twOrderList->setHorizontalHeaderLabels({
        "订单ID", "订单号", "航班号", "出发城市", "到达城市",
        "出发时间", "座位号", "票价", "订单状态"
    });

    ui->twOrderList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->twOrderList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twOrderList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twOrderList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twOrderList->hideColumn(0);
    ui->twOrderList->setAlternatingRowColors(true);
}

void OrderManager::loadOrdersToTable(const QList<Order>& orders)
{
    // 清空选中项
    ui->twOrderList->clearSelection();
    ui->twOrderList->setCurrentItem(nullptr);

    // 清空原有数据
    ui->twOrderList->setRowCount(0);

    // 填充新数据
    for (const auto& order : orders) {
        int row = ui->twOrderList->rowCount();
        ui->twOrderList->insertRow(row);

        QString departTimeStr = order.departTime().toString("yyyy-MM-dd HH:mm");

        ui->twOrderList->setItem(row, 0, new QTableWidgetItem(QString::number(order.id())));
        ui->twOrderList->setItem(row, 1, new QTableWidgetItem(order.orderNumber().isEmpty() ? "未知" : order.orderNumber()));
        ui->twOrderList->setItem(row, 2, new QTableWidgetItem(order.flightNumber().isEmpty() ? "未知" : order.flightNumber()));
        ui->twOrderList->setItem(row, 3, new QTableWidgetItem(order.departureCity().isEmpty() ? "未知" : order.departureCity()));
        ui->twOrderList->setItem(row, 4, new QTableWidgetItem(order.arrivalCity().isEmpty() ? "未知" : order.arrivalCity()));
        ui->twOrderList->setItem(row, 5, new QTableWidgetItem(departTimeStr));
        ui->twOrderList->setItem(row, 6, new QTableWidgetItem(order.seatNumber().isEmpty() ? "未知" : order.seatNumber()));
        ui->twOrderList->setItem(row, 7, new QTableWidgetItem(QString::number(order.price(), 'f', 2)));
        ui->twOrderList->setItem(row, 8, new QTableWidgetItem(order.status().isEmpty() ? "未支付" : order.status()));
    }

    // 同步按钮状态
    on_twOrderList_itemSelectionChanged();
}

void OrderManager::on_btnQuery_clicked()
{
    QString flightNum = ui->leFlightNum->text().trimmed();
    QDate date = ui->deOrderDate->date();
    QString status = ui->cbxOrderStatus->currentText();

    QList<Order> results = DBManager::instance().findOrders(flightNum, date, status);
    qDebug() << "筛选后订单数量：" << results.size();
    loadOrdersToTable(results);
    on_twOrderList_itemSelectionChanged();
}

void OrderManager::on_btnReset_clicked()
{
    ui->leFlightNum->clear();
    ui->deOrderDate->setDate(QDate::currentDate());
    ui->cbxOrderStatus->setCurrentIndex(0);
    loadOrdersToTable(DBManager::instance().getAllOrders());
    on_twOrderList_itemSelectionChanged();
}

void OrderManager::on_btnCancelOrder_clicked()
{
    // 新增：非手动触发（自动/信号触发）直接返回，不执行任何逻辑
    if (!m_isCancelManual) {
        m_isCancelManual = true; // 重置标记，避免影响下次手动操作
        return;
    }

    Order selected = getSelectedOrder();
    qDebug() << "===== 进入取消订单流程 =====";
    qDebug() << "选中订单ID：" << selected.id() << " 订单号：" << selected.orderNumber() << " 当前状态：" << selected.status();

    // 1. 校验订单有效性（删除“请先选择要取消的订单”弹窗，仅保留日志输出）
    if (!selected.isValid()) {
        qDebug() << "订单无效，无法取消";
        return; // 直接返回，不弹出任何弹窗
    }
    if (selected.status() == "已取消") {
        qDebug() << "订单已处于取消状态";
        QMessageBox warningBox(this);
        warningBox.setWindowTitle("提示");
        warningBox.setText("该订单已取消，无需重复操作！");
        warningBox.setIcon(QMessageBox::Warning);
        warningBox.setStandardButtons(QMessageBox::Ok);
        warningBox.button(QMessageBox::Ok)->setText("确认");
        warningBox.setStyleSheet(R"(
            QMessageBox {
                background-color: white;
                color: #222222;
                font-family: "微软雅黑";
                font-size: 14px;
            }
            QPushButton {
                background-color: #666666;
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 20px;
                font-size: 14px;
            }
        )");
        warningBox.exec();
        return;
    }

    // 2. 确认取消弹窗
    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("确认取消订单");
    confirmBox.setText(QString("确定要取消订单 %1 吗？\n航班号：%2\n座位号：%3")
                           .arg(selected.orderNumber())
                           .arg(selected.flightNumber())
                           .arg(selected.seatNumber()));
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.button(QMessageBox::Yes)->setText("确认取消");
    confirmBox.button(QMessageBox::No)->setText("取消");
    confirmBox.setStyleSheet(R"(
        QMessageBox {
            background-color: white;
            color: #222222;
            font-family: "微软雅黑";
            font-size: 14px;
            padding: 20px;
        }
        QPushButton {
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
            font-size: 14px;
            min-width: 90px;
            margin-left: 10px;
        }
        QPushButton#qt_msgbox_yes {
            background-color: #E53935;
            color: white;
        }
        QPushButton#qt_msgbox_no {
            background-color: #666666;
            color: white;
        }
        QPushButton:hover {
            opacity: 0.9;
        }
    )");

    if (confirmBox.exec() != QMessageBox::Yes) {
        qDebug() << "用户取消操作";
        return;
    }

    // 3. 执行取消
    bool cancelSuccess = DBManager::instance().cancelOrder(selected.id());
    qDebug() << "取消订单SQL执行结果：" << cancelSuccess;
    if (!cancelSuccess) {
        qCritical() << "取消订单失败，错误信息：" << DBManager::instance().lastError();
        QMessageBox criticalBox(this);
        criticalBox.setWindowTitle("失败");
        criticalBox.setText("订单取消失败，请重试！\n错误原因：" + DBManager::instance().lastError());
        criticalBox.setIcon(QMessageBox::Critical);
        criticalBox.setStandardButtons(QMessageBox::Ok);
        criticalBox.button(QMessageBox::Ok)->setText("确认");
        criticalBox.setStyleSheet(R"(
            QMessageBox {
                background-color: white;
                color: #222222;
                font-family: "微软雅黑";
                font-size: 14px;
            }
            QPushButton {
                background-color: #E53935;
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 20px;
                font-size: 14px;
            }
        )");
        criticalBox.exec();
        return;
    }

    // 4. 恢复航班座位
    Flight flight = DBManager::instance().getFlightByFlightNum(selected.flightNumber());
    if (flight.isValid()) {
        qDebug() << "找到对应航班：" << flight.flightNumber() << " 原可用座位：" << flight.availableSeats();
        flight.setAvailableSeats(flight.availableSeats() + 1);
        if (DBManager::instance().updateFlight(flight)) {
            qDebug() << "航班座位恢复成功，新可用座位：" << flight.availableSeats();
        } else {
            qWarning() << "航班座位恢复失败：" << DBManager::instance().lastError();
            QMessageBox warningBox(this);
            warningBox.setWindowTitle("提示");
            warningBox.setText("订单取消成功，但航班座位恢复失败，请手动核对！");
            warningBox.setIcon(QMessageBox::Warning);
            warningBox.setStandardButtons(QMessageBox::Ok);
            warningBox.button(QMessageBox::Ok)->setText("确认");
            warningBox.setStyleSheet(R"(
                QMessageBox {
                    background-color: white;
                    color: #222222;
                    font-family: "微软雅黑";
                    font-size: 14px;
                }
                QPushButton {
                    background-color: #666666;
                    color: white;
                    border: none;
                    border-radius: 8px;
                    padding: 8px 20px;
                    font-size: 14px;
                }
            )");
            warningBox.exec();
        }
    } else {
        qWarning() << "未找到对应航班，无法恢复座位：" << selected.flightNumber();
    }

    // 5. 刷新+发送信号
    QString cancelSuccessText = QString("订单取消成功！\n\n订单号：%1\n航班号：%2\n出发城市：%3\n到达城市：%4\n出发时间：%5\n座位号：%6")
                                    .arg(selected.orderNumber())
                                    .arg(flight.flightNumber())
                                    .arg(flight.departureCity())
                                    .arg(flight.arrivalCity())
                                    .arg(flight.departureTime().toString("yyyy-MM-dd HH:mm"))
                                    .arg(selected.seatNumber());

    QMessageBox successBox(this);
    successBox.setWindowTitle("取消成功");
    successBox.setText(cancelSuccessText);
    successBox.setIcon(QMessageBox::Information);
    successBox.setStandardButtons(QMessageBox::Ok);
    successBox.button(QMessageBox::Ok)->setText("确认");
    successBox.setStyleSheet(R"(
        QMessageBox {
            background-color: white;
            color: #222222;
            font-family: "微软雅黑";
            font-size: 14px;
            padding: 20px;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
            font-size: 14px;
            min-width: 90px;
        }
        QPushButton:hover {
            opacity: 0.9;
        }
    )");
    successBox.exec();

    // 清空选中+刷新+发送信号
    ui->twOrderList->clearSelection();
    ui->twOrderList->setCurrentItem(nullptr);
    // 新增：禁用取消订单按钮，避免自动触发
    ui->btnCancelOrder->setEnabled(false);
    on_btnQuery_clicked();
    emit orderCanceled(selected.flightNumber());
    on_twOrderList_itemSelectionChanged();

    // 新增：延迟1秒恢复按钮状态（不影响用户后续手动操作）
    QTimer::singleShot(1000, this, [this]() {
        on_twOrderList_itemSelectionChanged(); // 重新同步按钮状态
    });
}

void OrderManager::on_btnViewDetail_clicked()
{
    Order selected = getSelectedOrder();
    if (!selected.isValid()) {
        QMessageBox warningBox(this);
        warningBox.setWindowTitle("提示");
        warningBox.setText("请先选择要查看的订单！");
        warningBox.setIcon(QMessageBox::Warning);
        warningBox.setStandardButtons(QMessageBox::Ok);
        warningBox.button(QMessageBox::Ok)->setText("确认");
        warningBox.setStyleSheet(R"(
            QMessageBox {
                background-color: white;
                color: #222222;
                font-family: "微软雅黑";
                font-size: 14px;
            }
            QPushButton {
                background-color: #666666;
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 20px;
                font-size: 14px;
            }
        )");
        warningBox.exec();
        return;
    }

    // 显示详情
    ui->gbOrderDetails->setVisible(true);
    ui->valOrderNo->setText(selected.orderNumber());
    ui->valFlight->setText(selected.flightNumber());
    ui->valDepart->setText(selected.departTime().toString("yyyy-MM-dd hh:mm"));
    ui->valSeat->setText(selected.seatNumber());
}

void OrderManager::on_btnExit_clicked()
{
    QWidget *mainWindow = this->topLevelWidget();
    QStackedWidget *stackedWidget = mainWindow->findChild<QStackedWidget*>("stackedWidget");
    QWidget *pageHome = stackedWidget->findChild<QWidget*>("pageHome");
    if (stackedWidget && pageHome) {
        stackedWidget->setCurrentWidget(pageHome);
    } else {
        QMessageBox errorBox(this);
        errorBox.setWindowTitle("错误");
        errorBox.setText("无法找到主页页面！");
        errorBox.setIcon(QMessageBox::Critical);
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.button(QMessageBox::Ok)->setText("确认");
        errorBox.setStyleSheet(R"(
            QMessageBox {
                background-color: white;
                color: #222222;
                font-family: "微软雅黑";
                font-size: 14px;
            }
            QPushButton {
                background-color: #E53935;
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 20px;
                font-size: 14px;
            }
        )");
        errorBox.exec();
    }
}

Order OrderManager::getSelectedOrder()
{
    QList<QTableWidgetItem*> selectedItems = ui->twOrderList->selectedItems();
    if (selectedItems.isEmpty()) {
        qDebug() << "未选中任何订单（selectedItems为空）";
        return Order();
    }

    int row = selectedItems.first()->row();
    if (row < 0 || row >= ui->twOrderList->rowCount()) {
        qDebug() << "选中行号无效：" << row;
        return Order();
    }

    Order order;
    order.setId(ui->twOrderList->item(row, 0)->text().toInt());
    order.setOrderNumber(ui->twOrderList->item(row, 1)->text());
    order.setFlightNumber(ui->twOrderList->item(row, 2)->text());
    order.setDepartureCity(ui->twOrderList->item(row, 3)->text());
    order.setArrivalCity(ui->twOrderList->item(row, 4)->text());
    order.setDepartTime(QDateTime::fromString(ui->twOrderList->item(row, 5)->text(), "yyyy-MM-dd HH:mm"));
    order.setSeatNumber(ui->twOrderList->item(row, 6)->text());
    order.setPrice(ui->twOrderList->item(row, 7)->text().toDouble());
    order.setStatus(ui->twOrderList->item(row, 8)->text());

    qDebug() << "成功获取选中订单：ID=" << order.id() << " 订单号=" << order.orderNumber();
    return order;
}

void OrderManager::on_twOrderList_itemSelectionChanged()
{
    bool hasSelection = !ui->twOrderList->selectedItems().isEmpty();
    qDebug() << "订单表格选择状态：" << (hasSelection ? "已选中" : "未选中");

    ui->btnCancelOrder->setEnabled(hasSelection);
    ui->btnViewDetail->setEnabled(hasSelection);
}
