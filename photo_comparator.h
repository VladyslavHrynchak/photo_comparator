#ifndef PHOTO_COMPARATOR_H
#define PHOTO_COMPARATOR_H

#include <QMainWindow>
#include <QGridLayout>
#include <QFrame>
#include <QThread>

#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class Photo_Comparator; }
QT_END_NAMESPACE

class Photo_Comparator : public QMainWindow
{
    Q_OBJECT

public:
    Photo_Comparator(QWidget *parent = nullptr);
    ~Photo_Comparator();

private slots:
    void process_picture();
    void load_picture();

    void start_compare();

    void compareScreen();

    void on_pushButton_On_Off_clicked();

    void addItems();

signals:
    void process_picture_finished();

    void start_add_items();
    void start_clear_Items();

private:
    int compare_img(const QImage &img1, const QImage &img2);

    void insert_data_to_db();
    void load_data_from_database();

    void add_Item(const QPixmap &pixmap, const QString &text);

    void clear_Items();

private:
    QGridLayout *gridLayout = nullptr;
    QFrame *frame = nullptr;

    int x{0};
    int y{0};

    bool can_compare = true;

    QPixmap prevPixmap;
    QPixmap currentPixmap;

    int current_similarity{0};

    QList<QPair<QPixmap, QString>> items;

    QSqlDatabase load_data_conection;
    QSqlDatabase insert_data_connection;

    Ui::Photo_Comparator *ui;
};
#endif // PHOTO_COMPARATOR_H
