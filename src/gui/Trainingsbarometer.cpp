#include "Trainingsbarometer.h"
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QLabel>

Trainingsbarometer::Trainingsbarometer(int hours, int starts, QString pilot_name){
    this->hours = hours > 30 ? 30 : hours;
    this->starts = starts > 42 ? 42 : starts;
    this->pilot_name = pilot_name;
}

Trainingsbarometer::~Trainingsbarometer(){

}

QSize Trainingsbarometer::preferredSize()
{
    return QSize(4*XMAX, 4*YMAX);

}

QImage Trainingsbarometer::generate_barometer(){
    QRgb white, red, green, yellow;
    QImage image(4*XMAX, 4*YMAX, QImage::Format_RGB32);

    red = qRgb(238, 28, 35);
    green = qRgb(0, 165, 79);
    yellow = qRgb(255, 186, 0);
    white = qRgb(255, 255, 255);

    int x;
    int y;

    for(x = 0; x<4*XMAX; x++){
        for(y = 0; y< 4*YMAX; y++){
            image.setPixel(x,y, white);
        }
    }

    for(x = XOFFSETIMAGE; x< XMAX + XOFFSETIMAGE; x++){
        for(y = YOFFSETIMAGE; y < YMAX + YOFFSETIMAGE; y++){
            image.setPixel(x, y, green);
        }
    }

    for(x = XOFFSETIMAGE; x< XMAX + XOFFSETIMAGE; x++){
        for(y = YMAX + YOFFSETIMAGE; y < 2*YMAX + YOFFSETIMAGE; y++){
            image.setPixel(x, y, yellow);
        }
    }

    for(x = XOFFSETIMAGE; x< XMAX + XOFFSETIMAGE; x++){
        for(y = 2*YMAX + YOFFSETIMAGE; y < 3*YMAX + YOFFSETIMAGE; y++){
            image.setPixel(x, y, red);
        }
    }
    image = plot_axes(image);
    image = drawline(image);

    return image;
}

int Trainingsbarometer::stunden2pixel(int stunden){
    return (STUNDENMAX-stunden)*YIMAGEMAX/STUNDENMAX;
}

int Trainingsbarometer::starts2pixel(int starts){
    return (STARTSMAX-starts)*YIMAGEMAX/STARTSMAX;
}

QImage Trainingsbarometer::drawline(QImage image){
    int x_start = 0;
    int y_start = stunden2pixel(this->hours);
    int x_end = XMAX;
    int y_end = starts2pixel(this->starts);

    QRgb black = qRgb(0, 0, 0);
    QRgb white = qRgb(255, 255, 255);
    QPainter p(&image);
    QPoint p1(x_start + XOFFSETIMAGE, y_start + YOFFSETIMAGE);
    QPoint p2(x_end + XOFFSETIMAGE, y_end + YOFFSETIMAGE);
    p.drawLine(p1, p2);

    int y_offset, x, y;
    y_offset = 0;
    for(int y = YOFFSETIMAGE;y <= YIMAGEMAX + YOFFSETIMAGE; y++){
        if(image.pixel(XMAX/2 + XOFFSETIMAGE, y) == black){
            y_offset = y;
            break;
        }
    }

    for(x = XMAX/2-MARKERSIZE/2 + XOFFSETIMAGE; x<= XMAX/2+MARKERSIZE/2 + XOFFSETIMAGE; x++){
        for(y = y_offset - MARKERSIZE/2; y <= y_offset + MARKERSIZE/2; y++){
            if((x == XMAX/2-MARKERSIZE/2 + XOFFSETIMAGE) | (x == XMAX/2+MARKERSIZE/2 + XOFFSETIMAGE) | (y == y_offset - MARKERSIZE/2) | (y == y_offset + MARKERSIZE/2)){
                image.setPixel(x, y, black);
            }
            else{
                image.setPixel(x, y, white);
            }
            }
    }
    return image;
}

QImage Trainingsbarometer::plot_axes(QImage image){
    QPainter p(&image);
    QString title = "Trainingsbarometer";
    QString xlabel_stunden[] = {"30 h - ", "25 h - ", "20 h - ", "15 h - ", "10 h - ", "5 h   - "};
    QString xlabel_starts[] = {"- 40", "- 35", "- 30", "- 25", "- 20", "- 15", "- 10", "- 5"};

    QPoint point(XMAX+5 + XOFFSETIMAGE ,0);

    int i;
    for(i = 0; i < 8; i++){
        point.setY(3 + starts2pixel(42 - 2) + i*(YIMAGEMAX-starts2pixel(42 - 2))/8 + YOFFSETIMAGE);
        p.drawText(point, xlabel_starts[i]);
    }
    point.setX(XOFFSETIMAGE - 40);
    for(i = 0; i < 6; i++){
        point.setY(3 + i*YIMAGEMAX/6 + YOFFSETIMAGE);
        p.drawText(point, xlabel_stunden[i]);
    }

    QFont font("Arial", 15);
    p.setFont(font);
    QRect rect_title(QPoint(0, 20), QPoint(4*XMAX, 60));
    p.drawText(rect_title, Qt::AlignCenter, title);


    QRect rect_pilot(QPoint(0,60), QPoint(XMAX*4,80));
    font.setPixelSize(14);
    p.setFont(font);
    p.drawText(rect_pilot, Qt::AlignCenter, this->pilot_name);
    return image;
}

Trainingsbarometer::Status Trainingsbarometer::get_training_status(){
    int start_pixel = starts2pixel(this->starts);
    int hours_pixel = stunden2pixel(this->hours);
    int status_pixel = (start_pixel+hours_pixel)/2;

    if(status_pixel > YELLOWAREA){
        return RED;
    }
    else if(status_pixel > GREENAREA){
        return YELLOW;
    }
    else{
        return GREEN;
    }
}
