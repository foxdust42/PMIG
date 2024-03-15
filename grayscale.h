#ifndef TASK2_GRAYSCALE_H
#define TASK2_GRAYSCALE_H

#include <QImage>
#include <QAction>
namespace Task2 {

class Grayscale
{
private:
    QString _name;

public:

    Grayscale(QString name) { _name = name;}

    QString getName() { return _name; }

    static void toGray(QImage * image){}
};

class LightnessGray : public Grayscale {
public:
    LightnessGray() : Grayscale("Lightness") {}

    static void toGray(QImage* image);
};

class AverageGray : public Grayscale {
public:
    AverageGray() : Grayscale("Average") {}

    static void toGray(QImage* image);
};

class LuminosityGray : public Grayscale {
public:
    LuminosityGray() : Grayscale("Lightness") {}

    static void toGray(QImage* image);
};

} // namespace Task2

#endif // TASK2_GRAYSCALE_H
