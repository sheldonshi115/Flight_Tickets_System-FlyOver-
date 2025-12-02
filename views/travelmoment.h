#ifndef TRAVELMOMENT_H
#define TRAVELMOMENT_H

#include <QWidget>
#include <QDateTime>
#include <QVector>
#include <QStringList>

namespace Ui {
class TravelMoment;
}

struct MomentItem {
    int id = 0;
    QString userName = "匿名";
    QString content;
    QStringList images;
    QDateTime publishTime;
    int likeCount = 0;
    bool liked = false;
    int commentCount = 0;
};

class TravelMoment : public QWidget
{
    Q_OBJECT

public:
    explicit TravelMoment(QWidget *parent = nullptr);
    ~TravelMoment();

    void addMoment(const MomentItem &item);

private slots:
    void on_publishBtn_clicked();
    void on_selectImageBtn_clicked();
    void onLikeClicked(int id);
    void onCommentClicked(int id);
    void onImageClicked(const QString &path);

private:
    void refreshList();
    void showImagePreview(const QString &imagePath);

private:
    Ui::TravelMoment *ui;
    QVector<MomentItem> moments;
    int nextId = 1;
    QStringList selectedImages;
};

#endif // TRAVELMOMENT_H
