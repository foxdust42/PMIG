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
#include <QBuffer>

class VectorComponent;
class MovedObject;

enum VECTOR_COMPONENT_TYPE {
    VECTOR_COMPONENT_LINE,
    VECTOR_COMPONENT_POLYGON,
    VECTOR_COMPONENT_CIRCLE,
    VECTOR_COMPONENT_HALF_CIRCLES,
    VECTOR_COMPONENT_RECTANGLE,
    INVALID
};

typedef struct AETEntry {
    int y_max;
    double x;
    double inv_m;
} AETE;

typedef std::pair<QPoint, QPoint> edge;

class CustomGraphicsScene : public QGraphicsScene {
    Q_OBJECT
private:
    static double constexpr hit_margin = 3.0;

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

    VECTOR_COMPONENT_TYPE selectedType = VECTOR_COMPONENT_TYPE::VECTOR_COMPONENT_LINE;

    void init();

    void renderTargetPoints(QImage *image, std::vector<QPoint> pts, const QImage *background);

    static int clamp(int val, int min, int max);

    VectorComponent* (*generate)(int thickness, QRgb color);

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

    void setVectorComponentType(VECTOR_COMPONENT_TYPE vct);

    QString serializeComponents();

    int componentsCount() {
        return vec_items.size();
    }

    void deserializeComponents(QFile *in);

    static VECTOR_COMPONENT_TYPE QStringToVCT(QString s);

    // returns function pointer taking an int and QRgb and returning a pointer to a VectorComponent
    static VectorComponent* (*VCTToGenerator(VECTOR_COMPONENT_TYPE vct)) (std::vector<QPoint>, int, QRgb);

public slots:
    void deleteSelected();
    void setColorSelected(bool);
    void setThicknessSelected();
    void setAntialiasing(bool);
    void FillPolygon();
    void SetFillImage(QImage img);
    void ClearFillImage();
    bool CanSetImage();
};

class VectorComponent
{
protected:
    QRgb color = qRgb(0, 0, 0);
    QRgb clipColor = qRgb(255, 0, 0);
    QRgb fillColor = qRgb(0, 0, 0);
    int thickness = 1;
    std::vector<std::vector<int> > brush_mask;

    bool fill = false;

    static std::vector<std::vector<int> > GenerateBrushMask(int n);

    static void apply_brush(QImage* image, int x, int y, QRgb color, int thickness, std::vector<std::vector<int> > brush_mask);

    static int clamp(int val, int min, int max);

    static QColor ColorMeld(QColor c1, QColor c2, float pos);

    VectorComponent* clipTo = nullptr;

    std::vector<std::pair<QPoint, QPoint> > clipped_lines;
    std::vector<std::pair<QPoint, QPoint> > nonclipped_lines;

    QImage FillImage;

    bool imgFill = false;

public:
    std::vector<QPoint> points;

    QString SerializeSelf(QString prepend = "");

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

    static int InnerProduct(QPoint v1, QPoint v2){
        return v1.x()*v2.x() + v1.y()*v2.y();
    }

    static inline QRgb invert(QRgb p){
        return qRgb(255-qRed(p), 255-qGreen(p), 255 - qBlue(p));
    }

    virtual void addPoint(QPoint p){
        this->points.push_back(p);
    }

    void setColor(unsigned char r, unsigned char g, unsigned char b);
    void setColor(QRgb col);
    QRgb getColor();

    void setClipColor(unsigned char r, unsigned char g, unsigned char b);
    void setClipColor(QRgb col);
    QRgb getClipColor();

    void setFillColor(unsigned char r, unsigned char g, unsigned char b);
    void setFillColor(QRgb col);
    QRgb getFillColor();

    void setFillImage(QImage image) {this->FillImage = image; this->imgFill = true;}
    void clearFillImage() {this->imgFill = false;}
    QImage getFillImage() {return this->FillImage;}
    bool isFillImage() {return this->imgFill;}

    inline void SetThickness(int t) {
        this->thickness = t;
        this->brush_mask = GenerateBrushMask(this->thickness);
    }
    inline int getThickness() {return this->thickness;}

    inline void ClearClip() {
        this->clipTo = nullptr;
    }

    inline void SetClip(VectorComponent* ptr) {
        this->clipTo = ptr;
    }

    inline VectorComponent* GetClip() {
        return this->clipTo;
    }

    virtual MovedObject* CheckClickMove(QPoint clickPos, double tolreance) = 0;

    virtual MovedObject* MoveAll(QPoint clickPos, double tolerance) = 0;

    virtual const QString TypeSelf() = 0;

    virtual void RenderSelf(QImage* Image, bool anitalias = false) = 0;

    virtual bool IsReady() = 0;

    virtual bool IsRenderable() = 0;

    virtual bool ForceReady() = 0;

    virtual bool CanClipTo() = 0;

    virtual bool IsConvex() = 0;

    virtual std::vector<std::pair<QPoint, QPoint> > GetEdges() const = 0;

    virtual bool CanFill() = 0;

    bool GetFill() {return this->fill;}
    void SetFill() { this->fill = true; }
    void ClearFill() {this->fill = false; }

    QPoint VertexCenterOfMass() const {
        int x = 0, y = 0;
        for (QPoint p : this->points){
            x += p.x();
            y += p.y();
        }
        x /= this->points.size();
        y /= this->points.size();
        return QPoint(x, y);
    }

    virtual std::vector<QPoint> GetPoints(){return this->points;}

    static QPoint GetOutwardNormal(QPoint p1, QPoint p2, QPoint center, QPoint *lineAnchor = nullptr);
    static std::vector<QPoint> GetOutwardNormals(std::vector<edge> edges, QPoint center, std::vector<QPoint> *lineAnchors);

    static void FillVertexSorting(QImage *Image, VectorComponent *ToFill, QRgb fillColor, QImage *fillIMage = nullptr);
    static void RenderScanLine(QImage *Image, std::vector<AETE> AET, int y, QRgb col, QImage* image = nullptr);
};

class MidpointLine : public VectorComponent {
    //Midpoint Line Algorithm
protected:
    static void renderLine_M(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask);
    static void renderLineLo(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask);
    static void renderLineHi(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask);

    static void renderLineXiaolinWu(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask);
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

    bool CanClipTo() override {
        return false;
    }

    bool IsConvex() override {
        return false;
    }

    bool CanFill() override {
        return false;
    }

    virtual std::vector<std::pair<QPoint, QPoint> > GetEdges() const override {
        std::vector<std::pair<QPoint, QPoint> > edges;
        edges.push_back(std::pair<QPoint, QPoint> (points[0], points[1]));
        return edges;
    }

    static std::vector<std::pair<QPoint, QPoint> > ClipLineCyrusBeck(const std::vector<std::pair<QPoint, QPoint> > lines, const VectorComponent* const against, std::vector<std::pair<QPoint, QPoint> > *unclipped = nullptr);

};

class LinePolygon : public MidpointLine {
private:
    bool is_closed = false;
    static double constexpr meld_distance = 20.0f;
public:
    LinePolygon(std::vector<QPoint> points, int thickness = 1, QRgb color = qRgb(0,0,0)) : MidpointLine(points, thickness, color) {}
    LinePolygon(int thickness = 1, QRgb color = qRgb(0,0,0)) : MidpointLine(thickness, color) {}

    virtual void RenderSelf(QImage* Image, bool anitalias = false) override;

    const QString TypeSelf() override {
        return QString("LinePolygon");
    }

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

    virtual std::vector<std::pair<QPoint, QPoint> > GetEdges() const override {
        std::vector<std::pair<QPoint, QPoint> > edges;
        for(int i=0; i < points.size()-1; i++) {
            edges.push_back(std::pair<QPoint, QPoint>(points[i], points[i+1]));
        }
        return edges;
    }

    bool CanFill() override {
        if(this->points[0] == this->points[this->points.size()-1]){
            return true;
        }
        return false;
    }

    bool CanClipTo() override {return true;}

    bool IsConvex() override;

    void Fill();

};

class VectorCircle : public VectorComponent {
private:
    void RenderAltMidpointCircle(QImage *image);

    void RenderXiaolinWuCircle(QImage *image);

    void ApplyReflectedBrush(QImage *image, QPoint c, int x, int y, QRgb color);

    QColor XiaolinWuColorTransform(QColor Line, QColor Background, double T);

public:
    VectorCircle(std::vector<QPoint> points, int thickness = 1, QRgb color = qRgb(0,0,0)) : VectorComponent(points, thickness, color) {}
    VectorCircle(int thickness = 1, QRgb color = qRgb(0,0,0)) : VectorComponent(thickness, color) {}

    const QString TypeSelf() override {
        return QString("VectorCircle");
    }

    bool IsRenderable() override {
        if (points.size() < 2) {
            return false;
        }
        return true;
    }

    bool IsReady() override {
        return this->IsRenderable();
    }

    bool ForceReady() override {
        return this->IsReady();
    }

    void RenderSelf(QImage* Image, bool anitalias = false) override;

    MovedObject* CheckClickMove(QPoint clickPos, double tolerance) override;
    MovedObject* MoveAll(QPoint clickPos, double tolerance) override;

    virtual std::vector<std::pair<QPoint, QPoint> > GetEdges() const override {
        throw std::logic_error("No edges on a circle");
    }

    bool CanClipTo() override {
        return false;
    }

    bool IsConvex() override {
        return true;
    }

    bool CanFill() override {
        return false;
    }

};

class MovedObject {
protected:
    QPoint anchor;
    std::vector<QPoint*> pts;
public:
    MovedObject(std::vector<QPoint*> moved, QPoint axis) {pts = moved; anchor = axis;}

    virtual std::vector<QPoint> PointVals (){
        std::vector<QPoint> res;
        for (int i=0; i < pts.size(); i++){
            res.push_back(QPoint(pts[i]->x(), pts[i]->y()));
        }
        return res;
    }

    virtual void MoveTo(QPoint newPos);

};

class HalfCircles : public MidpointLine {
private:
    static int constexpr no_halfcircles = 5;

    void RenderAltMidpointCircle(QImage *image, QPoint center, int R, QPoint l1, QPoint l2, bool side);

    void ApplyReflectedBrush(QImage *image, QPoint c, int x, int y, QRgb color, QPoint l1, QPoint l2, bool side);

public:
    HalfCircles(std::vector<QPoint> points, int thickness = 1, QRgb color = qRgb(0,0,0)) : MidpointLine(points, thickness, color) {}
    HalfCircles(int thickness = 1, QRgb color = qRgb(0,0,0)) : MidpointLine(thickness, color) {}
    const QString TypeSelf() override { return QString("HalfCircles");}

    void RenderSelf(QImage* Image, bool anitalias = false) override;

    bool CanClipTo() override {return false;}
    bool IsConvex() override {return false;}

    virtual std::vector<std::pair<QPoint, QPoint> > GetEdges() const override {
        throw std::logic_error("No edges on whatever this is");
    }

};

class VectorRectangle : public MidpointLine {
protected:
    std::vector<QPoint> phantom_points;

    bool genPhantom();
public:
    VectorRectangle(std::vector<QPoint> points, int thickness = 1, QRgb color = qRgb(0,0,0)) : MidpointLine(points, thickness, color) {}
    VectorRectangle(int thickness = 1, QRgb color = qRgb(0,0,0)) : MidpointLine(thickness, color) {}

    const QString TypeSelf() override {
        return QString("Rectangle");
    }

    void RenderSelf(QImage* Image, bool anitalias = false) override;

    void addPoint(QPoint p) override;

    bool IsReady() override;
    bool ForceReady() override;

    MovedObject* CheckClickMove(QPoint clickPos, double tolerance) override;
    MovedObject* MoveAll(QPoint clickPos, double tolerance) override;

    bool CanClipTo() override {return true;}
    bool IsConvex() override {return true;}

    bool CanFill() override {
        return true;
    }

    std::vector<QPoint> GetPoints() override {
        std::vector<QPoint> ans;
        ans.push_back(points[0]);
        ans.push_back(phantom_points[0]);
        ans.push_back(points[1]);
        ans.push_back(phantom_points[1]);
        ans.push_back(points[0]);
        return ans;
    }

    virtual std::vector<std::pair<QPoint, QPoint> > GetEdges() const override {
        std::vector<edge> edges;
        edges.push_back(edge(points[0], phantom_points[0]));
        edges.push_back(edge(points[0], phantom_points[1]));
        edges.push_back(edge(points[1], phantom_points[0]));
        edges.push_back(edge(points[1], phantom_points[1]));
        return edges;
    }
};

class RectangleMoveObject : public MovedObject {
protected:
    std::vector<QPoint*> movedOX;
    std::vector<QPoint*> movedOY;
public:
    RectangleMoveObject(std::vector<QPoint*> moved, std::vector<QPoint*> movedOX, std::vector<QPoint*> movedOY, QPoint axis) : MovedObject(moved, axis) {
        this->movedOX = movedOX;
        this->movedOY = movedOY;
    }

    std::vector<QPoint> PointVals() override;

    virtual void MoveTo(QPoint newPos) override;
};

#endif // VECTORCOMPONENT_H
