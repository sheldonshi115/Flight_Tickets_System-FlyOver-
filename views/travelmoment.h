#ifndef TRAVELMOMENT_H
#define TRAVELMOMENT_H

#include <QWidget>
#include <QDateTime>
#include <QVector>
#include <QStringList>
#include "dbmanager.h" // 引入公共结构体
#include<QVBoxLayout>

// 前置声明 ClickableLabel（避免循环包含）
class ClickableLabel;

namespace Ui {
class TravelMoment;
}

class TravelMoment : public QWidget
{
    Q_OBJECT

public:
    explicit TravelMoment(QWidget *parent = nullptr);
    ~TravelMoment();

    // 新增：发布动态
    void addMoment(const MomentItem &item);

private slots:
    // 按钮点击槽函数
    void on_publishBtn_clicked();
    void on_selectImageBtn_clicked();
    void onLikeClicked(int id);
    void onCommentClicked(int id);
    void onImageClicked(const QString &path);

private:
    // 核心函数（补充缺失声明）
    void refreshList(); // 刷新动态列表
    void loadMomentsFromDB(); // 从数据库加载动态
    void showImagePreview(const QString &imagePath); // 图片预览
    void refreshComments(int momentId, QVBoxLayout* cardLayout); // 刷新评论列表

private:
    Ui::TravelMoment *ui;
    QVector<MomentItem> moments; // 动态列表
    int nextId = 1; // 下一个动态ID
    QStringList selectedImages; // 选中的图片路径
};

#endif // TRAVELMOMENT_H
