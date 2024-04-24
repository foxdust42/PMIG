#include "vectorcomponent.h"

#include <QTextStream>
#include <QWidget>
#include <QPainter>
#include <cmath>

/*
VectorComponent::VectorComponent(std::vector<QPoint> points) {
    this->points = points;
}
*/
void CustomGraphicsScene::init(){
    this->vec_over = new QGraphicsPixmapItem();
    this->bg_image = new QGraphicsPixmapItem();
    //this->vec_raster = new QImage(0, 0, QImage::Format_RGBA64);

    this->target = QImage(":/vec/target.png");
    if (target.isNull()){
        throw std::runtime_error("Failed to load resource");
    }

    this->addItem(bg_image);
    this->addItem(vec_over);
}

void CustomGraphicsScene::renderTargetPoints(QImage *image, std::vector<QPoint> pts, const QImage *background){
    int wp = target.width()/2;
    int hp = target.height()/2;

    bool first = true;

    std::sort(pts.begin(), pts.end(), [](QPoint p1, QPoint p2){
        if(p1.x() >  p2.x()){ return true ;}
        if(p1.x() <  p2.x()){ return false;}
        if(p1.y() >= p2.y()){ return true; }
        return false;
    });

    QPoint prev;

    for (QPoint &p : pts){
        if (!first && p == prev){
            continue;
        }

        prev = p;

        QTextStream(stdout) << "P: " << p.x() << " " << p.y() << "\n";

        if (p == pts[0] ){
            if (first){
                first = false;
            }else {
                continue;
            }
        }

        for (int i = 0; i < target.width(); i++){
            for (int j = 0; j < target.height(); j++){

                QPoint curr_point(
                    this->clamp(p.x() + i - wp, 0, image->width()),
                    this->clamp(p.y() + j - hp, 0, image->height())
                    );

                QColor c = image->pixelColor(curr_point);

                //QTextStream(stdout) << " " << (target.pixelColor(i, j) == Qt::transparent ? 0 : 1);

                if (c == Qt::transparent) {
                    c = background->pixelColor(curr_point);
                }

                //c = c == Qt::transparent ? background->pixelColor(curr_point) : c;

                if (target.pixelColor(QPoint(i,j)) != Qt::transparent){
                    image->setPixel(curr_point, VectorComponent::invert( c.rgb()));
                }
            }
            //QTextStream(stdout) << "\n";
        }
    }
}

CustomGraphicsScene::~CustomGraphicsScene() {

    this->clear(); //QGraphicsScene takes ownership of its items
    /*
    if (this->vec_over != nullptr){
        delete this->vec_over;
    }

    if (this->bg_image != nullptr){
        delete this->bg_image;
    }
    */
    if (this->vec_raster != nullptr) {
        delete this->vec_raster;
    }

    for (VectorComponent* vc : this->vec_items){
        delete vc;
    }

}

void CustomGraphicsScene::ClearVectorComponents(){
    this->moved = nullptr;
    if(this->generated != nullptr){
        delete generated;
    }
    generated = nullptr;
    this->active = nullptr;

    for (int i=0; i < vec_items.size(); i++){
        delete vec_items[i];
    }
    vec_items.clear();
}


void CustomGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mousePressEvent(event);

    QTextStream(stdout) << "QGSME: " << event->pos().x() << " " << event->pos().y() << " " << (event->modifiers() & Qt::KeyboardModifier::ControlModifier) << "\n";

    if (vec_raster != nullptr){
        delete vec_raster;
    }
    vec_raster = new QImage(bg_image->pixmap().width(), bg_image->pixmap().height(), QImage::Format_RGBA64);
    vec_raster->fill(Qt::transparent);

    /*
    if (this->generated == nullptr){
        this->generated = new LinePolygon(5, qRgb(100,0,255));
        this->active = this->generated;
    }
    */

    if (event->button() == Qt::MouseButton::RightButton){
        if(generated != nullptr){
            if(generated->ForceReady()){
                this->addComponent(generated);
            }
            else {
                delete generated;
            }
        }
        generated = nullptr;
        moved = nullptr;
        active = nullptr;
        goto render;
    }

    //event->type() == QEvent::Type::MouseButtonRelease
    //event->buttons() == Qt::MouseButton::LeftButton;

    if (this->moved != nullptr){
        //this->moved->setX(event->pos().x());
        //this->moved->setY(event->pos().y());

        if ( (event->modifiers() & Qt::KeyboardModifier::ControlModifier) != 0){
            QTextStream(stdout) << "CTRL\n";
            MovedObject* tmp = active->MoveAll(event->pos().toPoint(), hit_margin);
            if (tmp != nullptr){
                moved = tmp;
            }
            goto render;
        }
        moved->MoveTo(event->pos().toPoint());

        if (event->type() == QEvent::Type::MouseButtonRelease){
            delete moved;
            moved = nullptr;
        }
        goto render;
    }
    if (this->generated != nullptr){
        this->generated->addPoint(event->pos().toPoint());

        if (this->generated->IsReady()){
            this->addComponent(this->generated);
            this->generated = nullptr;
        }
        else if(this->generated->IsRenderable()){
            this->generated->RenderSelf(vec_raster, this->antialias);
        }
        goto render;
    }
    if (this->active != nullptr) {
        /*moved = active->MoveAll(event->pos().toPoint(), hit_margin);
        if (moved == nullptr){
        }*/
        if (moved != nullptr) {
            //this->active = nullptr;
            goto render;
        }
    }

    for (int i=0; i< vec_items.size(); i++){
        moved = vec_items[i]->CheckClickMove(event->pos().toPoint(), hit_margin);
        if (moved != nullptr){
            //this->active = nullptr;
            this->active = vec_items[i];
            goto render;
        }
    }

    this->generated = new LinePolygon(this->base_thickness, this->base_color);
    this->generated->addPoint(event->pos().toPoint());
    this->active = this->generated;

render:
    this->renderVectorComponents();

    if (moved != nullptr) {
        renderTargetPoints(vec_raster, moved->PointVals(), base_image);
    }

    if (active != nullptr && moved == nullptr){
        renderTargetPoints(vec_raster, this->active->points, base_image);
    }

    this->vec_over->setPixmap(QPixmap::fromImage(*vec_raster));

    this->invalidate();

    emit this->mousePressed(event);

}

void CustomGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mouseMoveEvent(event);
    return;
    if (this->moved != nullptr){
        this->moved->MoveTo(event->pos().toPoint());

        if (vec_raster != nullptr){
            delete vec_raster;
        }
        vec_raster = new QImage(bg_image->pixmap().width(), bg_image->pixmap().height(), QImage::Format_RGBA64);
        vec_raster->fill(Qt::transparent);

        this->renderVectorComponents();

        if (moved != nullptr) {
            renderTargetPoints(vec_raster, moved->PointVals(), base_image);
        }

        if (active != nullptr && moved == nullptr){
            renderTargetPoints(vec_raster, this->active->points, base_image);
        }

        this->vec_over->setPixmap(QPixmap::fromImage(*vec_raster));

        this->invalidate();
    }
}

void CustomGraphicsScene::setBackgroundImage(QImage *image){
    this->base_image = image;
    this->bg_image->setPixmap(QPixmap::fromImage(*image));
    delete this->vec_raster;

    this->vec_raster = new QImage(this->bg_image->boundingRect().size().toSize(), QImage::Format_RGBA64);

    this->renderVectorComponents();

    this->active = nullptr;

    this->invalidate();
    //this->bg_image->
}

void CustomGraphicsScene::renderVectorComponents(){
    for (VectorComponent *c : this->vec_items){
        c->RenderSelf(this->vec_raster, this->antialias);
    }
}

void CustomGraphicsScene::render(){

}

QRgb VectorComponent::getColor() {
    return this->color;
}

void VectorComponent::setColor(unsigned char r, unsigned char g, unsigned char b){
    this->color = qRgb(r, g, b);
}

std::vector<std::vector<int> > VectorComponent::GenerateBrushMask(int n){
    double c = ((double) n)/2;
    std::vector<std::vector<int> > ans;
    for (int i=0; i < n; i++){
        std::vector<int> tmp(n);
        for (int j=0; j < n; j++){
            if( std::pow(c - i, 2) + std::pow(c-j, 2) >= std::pow(c, 2) ){
                tmp[j] = 0;
            }
            else{
                tmp[j] = 1;
            }
        }
        ans.push_back(tmp);
    }

    return ans;
}

void VectorComponent::apply_brush(QImage *image, int x, int y, QRgb color){
    for(int i=0; i < this->thickness; i++) {
        for (int j=0; j < this->thickness; j++){
            if (brush_mask[i][j] == 1){
                image->setPixel(clamp(x + i - this->thickness/2, 0, image->width()), clamp(y + j - this->thickness/2, 0, image->height()), color);
            }
        }
    }
}

int VectorComponent::clamp(int val, int min, int max){
    return std::max(min, std::min(max, val));
}

int CustomGraphicsScene::clamp(int val, int min, int max){
    return std::max(min, std::min(max, val));
}


double VectorComponent::PointDistance(QPoint p1, QPoint p2){
    int x_off = std::abs(p1.x() - p2.x());
    int y_off = std::abs(p1.y() - p2.y());

    return std::sqrt( (double) ((x_off*x_off) + (y_off*y_off)) );
}

double VectorComponent::LineDistance(QPoint l1, QPoint l2, QPoint p) {

    double nominator = std::abs(  (l2.x() - l1.x())
                                * (p.y()  - l1.y())
                                - (p.x()  - l1.x())
                                * (l2.y() - l1.y()) );
    double denominator = std::sqrt(
            std::pow( (l2.x() - l1.x()) ,2)
          + std::pow( (l2.y() - l1.y()), 2) );
    return nominator/denominator;
}

void MidpointLine::RenderSelf(QImage* Image, bool anitalias){
    if (this->points.size() < 2){
        throw std::logic_error("Cannot define a line with less that 2 points");
    }

    QPoint p1 = this->points[0];
    QPoint p2 = this->points[1];

    this->renderLine_M(Image, p1, p2);
}

void MidpointLine::renderLine_M(QImage* Image, QPoint p1, QPoint p2) {
    if ( std::abs(p2.y() - p1.y()) < std::abs(p2.x() - p1.x()) ){
        if (p1.x() > p2.x()){
            renderLineLo(Image, p2, p1);
        }
        else {
            renderLineLo(Image, p1, p2);
        }
    }
    else {
        if(p1.y() > p2.y()){
            renderLineHi(Image, p2, p1);
        }
        else {
            renderLineHi(Image, p1, p2);
        }
    }
}

void MidpointLine::renderLineLo(QImage* image, QPoint p1, QPoint p2){
    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();
    int yi = 1;
    int D;
    int y;

    if(dy < 0){
        yi = -1;
        dy *= (-1);
    }
    D = (2 * dy) - dx;
    y = p1.y();

    for(int x = p1.x(); x<= p2.x(); x++){
        //image->setPixel(x, y, this->color);
        apply_brush(image, x, y, this->color);
        if (D > 0){
            y += yi;
            D += 2*(dy - dx);
        }
        else {
            D += 2 * dy;
        }
    }
}

void MidpointLine::renderLineHi(QImage* image, QPoint p1, QPoint p2){
    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();
    int xi = 1;
    int D;
    int x;

    if(dx < 0){
        xi = -1;
        dx *= (-1);
    }
    D = (2 * dx) - dy;
    x = p1.x();

    for(int y = p1.y(); y<= p2.y(); y++){
        //image->setPixel(x, y, this->color);
        apply_brush(image, x, y, this->color);
        if (D > 0){
            x += xi;
            D += 2*(dx - dy);
        }
        else {
            D += 2 * dx;
        }
    }
}

MovedObject* MidpointLine::CheckClickMove(QPoint clickPos, double tolerance){

    std::vector<QPoint*> movedPoints;

    //points
    if (VectorComponent::PointDistance(clickPos, points[0]) <= tolerance ){
        movedPoints.push_back(&points[0]);
        goto fin;
    }
    if (VectorComponent::PointDistance(clickPos, points[1]) <= tolerance ){
        movedPoints.push_back(&points[1]);
        goto fin;
    }

    //line(s)
    if (VectorComponent::LineDistance(points[0], points[1], clickPos) <= tolerance ) {
        movedPoints.push_back(&points[0]);
        movedPoints.push_back(&points[1]);
        goto fin;
    }
    //no hit
    return nullptr;

fin:
    return new MovedObject(movedPoints, clickPos);
}

MovedObject* MidpointLine::MoveAll(QPoint clickPos, double tolerance){

    std::vector<QPoint*> movedPoints;

    //line(s)
    if (VectorComponent::LineDistance(points[0], points[1], clickPos) <= tolerance ) {
        movedPoints.push_back(&points[0]);
        movedPoints.push_back(&points[1]);
        goto fin;
    }
    //no hit
    return nullptr;

fin:
    return new MovedObject(movedPoints, clickPos);
}

void LinePolygon::RenderSelf(QImage* Image, bool anitalias){
    if (!this->IsRenderable()){
        throw std::logic_error("Cannot define a polygon with less than 2 points");
    }

    for (int i=1; i<this->points.size(); i++){
        this->renderLine_M(Image, this->points[i-1], this->points[i]);
    }
}

void MovedObject::MoveTo(QPoint newPos){
    QPoint offset = newPos - anchor;
    anchor = newPos;
    for (QPoint *p : pts){
        *p += offset;
    }
}

MovedObject* LinePolygon::CheckClickMove(QPoint clickPos, double tolerance) {
    std::vector<QPoint*> movedPoints;

    //points
    for (QPoint &p : points){
        if(VectorComponent::PointDistance(clickPos, p) <= tolerance){
            movedPoints.push_back(&p);
            if (p == points[0]){
                movedPoints.push_back(&points[points.size()-1]);
            }
            goto fin;
        }
    }

    //lines
    for (int i = 1; i < points.size(); i++){
        if(VectorComponent::LineDistance(points[i-1], points[i], clickPos) <= tolerance){
            movedPoints.push_back(&points[i-1]);
            movedPoints.push_back(&points[i]);
            if(i == 1) {
                movedPoints.push_back(&points[points.size()-1]);
            }
            if(i == points.size()-1){
                movedPoints.push_back(&points[0]);
            }
            goto fin;
        }
    }

    //whole polygon?

    // no hit

    return nullptr;
fin:
    return new MovedObject(movedPoints, clickPos);
}


MovedObject* LinePolygon::MoveAll(QPoint clickPos, double tolerance) {
    MovedObject* mobj;
    mobj = this->CheckClickMove(clickPos, tolerance);
    if (mobj == nullptr){
        return nullptr;
    }
    delete mobj;

    std::vector<QPoint*> pts;

    for (int i=0; i < this->points.size(); i++){
        pts.push_back(&points[i]);
    }

    return new MovedObject(pts, clickPos);
    //throw std::runtime_error("Not Implemented");
}

