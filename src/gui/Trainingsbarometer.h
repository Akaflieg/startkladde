#ifndef TRAININGSBAROMETER_H
#define TRAININGSBAROMETER_H

#define XMAX 60
#define YMAX 158
#define YIMAGEMAX 3*YMAX
#define GREENAREA YMAX
#define YELLOWAREA 2*YMAX
#define STUNDENMAX 30
#define STARTSMAX 42
#define MARKERSIZE XMAX/10
#define LABELDIFF 30
#define XOFFSETIMAGE XMAX + XMAX/2
#define YOFFSETIMAGE YMAX - 20

#include <QImage>
#include <QString>

class Trainingsbarometer
{

public:
    enum Status {RED, GREEN, YELLOW};
    Trainingsbarometer(int hours, int starts, QString pilot);
    QImage generate_barometer();
    QSize preferredSize();
    Trainingsbarometer::Status get_training_status();
    ~Trainingsbarometer();

private:
   int hours;
   int starts;
   QString pilot_name;
   int starts2pixel(int starts);
   int stunden2pixel(int stunden);
   QImage plot_axes(QImage image);
   QImage drawline(QImage image);

};

#endif // TRAININGSBAROMETER_H
