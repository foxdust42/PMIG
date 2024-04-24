#ifndef VECTORCOMPONENT_H
#define VECTORCOMPONENT_H

#include <QObject>
#include <QImage>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QtGlobal>
#include <QRectF>
#include <QTextStream>

class VectorComponent;
class MovedObject;

class CustomGraphicsScene : public QGraphicsScene {
    Q_OBJECT
private:
    static double constexpr hit_margin = 7.0;

    QImage *vec_raster = nullptr;
    QImage *base_image = nullptr;

    QImage target;

    //QPixmap *result_img = nullptr;

    QGraphicsPixmapItem *bg_image = nullptr;
    QGraphicsPixmapItem *vec_over = nullptr;

    VectorComponent *generated = nullptr;
    VectorComponent *active = nullptr;

    std::vector<QPoint> target_points;

    std::vector<VectorComponent*> vec_items;

    MovedObject * moved = nullptr;

    QRgb base_color = qRgb(0, 0, 0);

    int base_thickness = 5;

    bool antialias = false;

    void init();

    void renderTargetPoints(QImage *image, std::vector<QPoint> pts, const QImage *background);

    static int clamp(int val, int min, int max);

signals:
    void mousePressed(QGraphicsSceneMouseEvent *event);

public:

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    //

    void ClearVectorComponents();

    CustomGraphicsScene(QObject *parent = nullptr) : QGraphicsScene(parent) { this->init(); }
    CustomGraphicsScene(const QRectF &sceneRect, QObject *parent = nullptr) : QGraphicsScene(sceneRect, parent) { this->init(); }
    CustomGraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr) : QGraphicsScene(x, y, width, height, parent) { this->init(); }
    ~CustomGraphicsScene();
    void renderVectorComponents(/*std::vector<VectorComponent> vobj*/);

    void setBackgroundImage(QImage *image);

    void render();

    void addComponent(VectorComponent *c){
        this->vec_items.push_back(c);
    }

    void setThickness(int thickness) {
        this->base_thickness = thickness;
    }

    void setColor(QRgb col){
        this->base_color = col;
    }

    void setColor(int r, int g, int b){
        this->base_color = qRgb(clamp(r, 0, 255), clamp(g, 0, 255), clamp(b, 0 ,255));
    }

};

class VectorComponentHandler {
private:
    std::vector<VectorComponent> components;
    static constexpr double detection_distance = 1.5;

public:
    //static double PointDistance(QPoint p1, QPoint p2);
    void addComponent(VectorComponent c);
};

class VectorComponent
{
protected:
    QRgb color = qRgb(0, 0, 0);
    int thickness = 1;
    std::vector<std::vector<int> > brush_mask;

    static std::vector<std::vector<int> > GenerateBrushMask(int n);

    void apply_brush(QImage* image, int x, int y, QRgb color);

    static int clamp(int val, int min, int max);
public:
    std::vector<QPoint> points;

    VectorComponent(std::vector<QPoint> points, int thickness = 1, QRgb color = qRgb(0,0,0)){
        this->points = points;
        this->thickness = thickness;
        this->color = color;
        this->brush_mask = GenerateBrushMask(this->thickness);
    }

    VectorComponent(int thickness = 1, QRgb color = qRgb(0,0,0)){
        this->thickness = thickness;
        this->color = color;
        this->brush_mask = GenerateBrushMask(this->thickness);
    }

    virtual ~VectorComponent() {}

    static double PointDistance(QPoint p1, QPoint p2);

    static double LineDistance(QPoint l1, QPoint l2, QPoint p);

    static QRgb invert(QRgb p){
        return qRgb(255-qRed(p), 255-qGreen(p), 255 - qBlue(p));
    }

    virtual void addPoint(QPoint p){
        this->points.push_back(p);
    }

    //~VectorComponent(){}

    void setColor(unsigned char r, unsigned char g, unsigned char b);
    QRgb getColor();

    void SetThickness(int t) {
        this->thickness = t;
        this->brush_mask = GenerateBrushMask(this->thickness);
    }
    int getThickness() {return this->thickness;}

    virtual MovedObject* CheckClickMove(QPoint clickPos, double tolreance) = 0;

    virtual MovedObject* MoveAll(QPoint clickPos, double tolerance) = 0;

    virtual const QString TypeSelf() = 0;

    virtual void RenderSelf(QImage* Image, bool anitalias = false) = 0;

    virtual bool IsReady() = 0;

    virtual bool IsRenderable() = 0;

    virtual bool ForceReady() = 0;

    //virtual void mouseMoveEvent(QMouseEvent *event) = 0;
};

class MidpointLine : public VectorComponent {
    //Midpoint Line Algorithm
protected:
    void renderLine_M(QImage* image, QPoint p1, QPoint p2);
    void renderLineLo(QImage* image, QPoint p1, QPoint p2);
    void renderLineHi(QImage* image, QPoint p1, QPoint p2);
public:
    MidpointLine(std::vector<QPoint> points, int thickness = 1, QRgb color = qRgb(0,0,0)) : VectorComponent(points, thickness, color) {}
    MidpointLine(int thickness = 1, QRgb color = qRgb(0,0,0)) : VectorComponent(thickness, color) {}

    const QString TypeSelf() override {
        return QString("Line");
    }

    void RenderSelf(QImage* Image, bool anitalias = false) override;

    MovedObject* CheckClickMove(QPoint clickPos, double tolerance) override;
    MovedObject* MoveAll(QPoint clickPos, double tolerance) override;

    bool IsReady() override {
        if(this->points.size() >= 2){
            return true;
        }
        return false;
    }

    bool IsRenderable() override {
        return this->IsReady();
    }

    bool ForceReady() override {
        if (!IsReady()){
            return false;
        }
        return true;
    }

};

class LinePolygon : public MidpointLine {
private:
    bool is_closed = false;
    static double constexpr meld_distance = 20.0f;
public:
    LinePolygon(std::vector<QPoint> points, int thickness = 1, QRgb color = qRgb(0,0,0)) : MidpointLine(points, thickness, color) {}
    LinePolygon(int thickness = 1, QRgb color = qRgb(0,0,0)) : MidpointLine(thickness, color) {}

    void RenderSelf(QImage* Image, bool anitalias = false) override;

    bool IsRenderable() override {
        if(this->points.size() >= 2){
            return true;
        }
        return false;
    }

    bool IsReady() override{
        return is_closed;
    }

    MovedObject* CheckClickMove(QPoint clickPos, double tolerance) override;
    MovedObject* MoveAll(QPoint clickPos, double tolerance) override;

    void addPoint(QPoint p) override{
        if (this->points.size() <= 2){
            VectorComponent::addPoint(p);
            return;
        }
        if (VectorComponent::PointDistance(this->points[0], p) <= meld_distance){
            VectorComponent::addPoint(this->points[0]);
            this->is_closed = true;
            QTextStream(stdout) << "Closed polygon\n";
        }
        else {
            VectorComponent::addPoint(p);
        }
    }

    bool ForceReady() override {
        if(!IsRenderable()){
            return false;
        }
        VectorComponent::addPoint(this->points[0]);
        this->is_closed = true;
        return IsReady();
    }

};

class MovedObject {
private:
    QPoint anchor;
    std::vector<QPoint*> pts;
public:
    MovedObject(std::vector<QPoint*> moved, QPoint axis) {pts = moved; anchor = axis;}

    std::vector<QPoint> PointVals (){
        std::vector<QPoint> res;
        for (int i=0; i < pts.size(); i++){
            res.push_back(QPoint(pts[i]->x(), pts[i]->y()));
        }
        return res;
    }

    void MoveTo(QPoint newPos);

};

#endif // VECTORCOMPONENT_H
