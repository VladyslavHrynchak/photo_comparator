#include "photo_comparator.h"
#include "ui_photo_comparator.h"

#include <QDebug>
#include <QImage>

#include <QWindow>
#include <QScreen>
#include <QTimer>

#include <QtSql>
#include <QCryptographicHash>

#include <QLabel>

#ifdef __unix__

#define PATH_TO_DB ""../photo_comparator/db/database.db""

#elif defined(_WIN32) || defined(WIN32)

#define PATH_TO_DB "..\\photo_comparator\\db\\database.db"

#endif

Photo_Comparator::Photo_Comparator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Photo_Comparator)
{
    ui->setupUi(this);

    load_data_conection = QSqlDatabase::addDatabase("QSQLITE", "load_data_from_database");

    load_data_conection.setDatabaseName(PATH_TO_DB);
    load_data_conection.open();

    insert_data_connection = QSqlDatabase::addDatabase("QSQLITE");
    insert_data_connection.setDatabaseName(PATH_TO_DB);
    insert_data_connection.open();

    gridLayout = new QGridLayout;

    frame = new QFrame;
    frame->setLayout(gridLayout);

    ui->scrollArea->setWidget(frame);

    frame->setLayout(gridLayout);

    connect(this, &Photo_Comparator::start_add_items, this, &Photo_Comparator::addItems);
    connect(this, &Photo_Comparator::start_clear_Items, this, &Photo_Comparator::clear_Items);

    load_data_from_database();

    QThread* thread = new QThread(this);

    connect(thread, &QThread::started, [=]()
    {
        QTimer* timer1 = new QTimer(thread);
        timer1->setInterval(60000);
        connect(timer1, &QTimer::timeout, [=]()
        {
            start_compare();
        });
        timer1->start();
    });

    thread->start();
}

Photo_Comparator::~Photo_Comparator()
{
    delete ui;
}

void Photo_Comparator::addItems()
{
    for (int i = items.count() - 1; i >= 0; --i) {
        add_Item(items[i].first, items[i].second);
    }
}

void Photo_Comparator::add_Item(const QPixmap &pixmap_, const QString &text)
{
    QPixmap pixmap = pixmap_.scaled(750, 750, Qt::KeepAspectRatio);

    QLabel *imageLabel = new QLabel;
    imageLabel->setPixmap(pixmap);
    imageLabel->setAlignment(Qt::AlignCenter);

    QLabel *textLabel = new QLabel("Similarity: " +  text + " %");
    textLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *itemLayout = new QVBoxLayout;
    itemLayout->addWidget(imageLabel);

    itemLayout->addWidget(textLabel);
    itemLayout->addStretch();

    gridLayout->addLayout(itemLayout, x, y);

     if(y == 1){
         x++;
         y = 0;

     }else{
          y++;
     }
}

void Photo_Comparator::clear_Items()
{
    delete frame;

    frame = new QFrame;
    gridLayout = new QGridLayout;

    frame->setLayout(gridLayout);
    ui->scrollArea->setWidget(frame);
}

void Photo_Comparator::start_compare()
{
    if(can_compare){
        process_picture();
        load_picture();
    }
}

void Photo_Comparator::process_picture()
{
    compareScreen();
    insert_data_to_db();
}

void Photo_Comparator::load_picture()
{
    x = y = 0;
    load_data_from_database();
}

void Photo_Comparator::insert_data_to_db()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    currentPixmap.save(&buffer, "PNG");

    QByteArray hash_sum = QCryptographicHash::hash(byteArray, QCryptographicHash::Md5).toHex();

    QSqlQuery query(insert_data_connection);
    query.prepare("INSERT INTO images_similarity (image, hash_sum, similarity) VALUES (:image, :string, :integer)");
    query.bindValue(":image", byteArray);
    query.bindValue(":string", QString::fromLatin1(hash_sum));
    query.bindValue(":integer", current_similarity);

    if (query.exec()) {
        qDebug() << "Data inserted successfully.";

    } else {
        qDebug() << "Error: Unable to insert data." << query.lastError();
    }
}

void Photo_Comparator::load_data_from_database()
{
    QSqlQuery query("SELECT * FROM images_similarity", load_data_conection);

    items.clear();

    if (query.exec()) {
        while (query.next()) {
            QByteArray imageByteArray = query.value(1).toByteArray();
            int intData = query.value(3).toInt();

            QPixmap imagePixmap;
            imagePixmap.loadFromData(imageByteArray);

            prevPixmap = imagePixmap;

            items.append(qMakePair(imagePixmap, QString::number(intData)));
        }

        emit(start_clear_Items());
        emit(start_add_items());

    } else {
        qDebug() << "Error: Unable to execute query." << query.lastError();
    }
}

void Photo_Comparator::compareScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();

    if (const QWindow *window = windowHandle())
        screen = window->screen();

    if (!screen)
        return;

    if(prevPixmap.isNull())
    {
        prevPixmap = currentPixmap = screen->grabWindow(0);
        current_similarity = 100;
    }
    else
    {
        currentPixmap = screen->grabWindow(0);
        current_similarity = compare_img(prevPixmap.toImage(), currentPixmap.toImage());
        prevPixmap = currentPixmap;
    }
}

int Photo_Comparator::compare_img(const QImage &img1, const QImage &img2)
{
    double similar_px_count = 0;

    for (int y = 0; y < img1.height(); ++y) {
        for (int x = 0; x < img1.width(); ++x) {
            if(img1.pixel(x, y) == img2.pixel(x, y)){
                similar_px_count++;
            }
        }
    }

    double num_of_pixels = img1.height() * img1.width();

    int similar_percentage = ( similar_px_count / num_of_pixels ) * 100;

    return similar_percentage;
}

void Photo_Comparator::on_pushButton_On_Off_clicked()
{
    if(can_compare){
        ui->pushButton_On_Off->setText("Off");
        can_compare = false;
    }
    else{
        ui->pushButton_On_Off->setText("On");
        can_compare = true;
    }
}

