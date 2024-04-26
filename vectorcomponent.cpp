#include "vectorcomponent.h"

#include <QTextStream>
#include <QWidget>
#include <QPainter>
#include <cmath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

    this->setVectorComponentType(VECTOR_COMPONENT_LINE);
}

void CustomGraphicsScene::renderTargetPoints(QImage *image, std::vector<QPoint> pts, const QImage *background){
    int wp = target.width()/2;
    int hp = target.height()/2;

    bool first = true;

    std::sort(pts.begin(), pts.end(), [](QPoint p1, QPoint p2) -> bool{
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
                continue;void setAntialiasing(bool);
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

void CustomGraphicsScene::setAntialiasing(bool val) {
    this->antialias = val;
    this->render();
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

    this->render();
}

void CustomGraphicsScene::setVectorComponentType(VECTOR_COMPONENT_TYPE vct){
    this->selectedType = vct;

    switch (this->selectedType){
    case VECTOR_COMPONENT_LINE:
        this->generate = [](int thickness, QRgb color) -> VectorComponent* {
            return new MidpointLine(thickness, color);
        };
        break;
    case VECTOR_COMPONENT_POLYGON:
        this->generate = [](int thickness, QRgb color) -> VectorComponent* {
            return new LinePolygon(thickness, color);
        };
        break;
    case VECTOR_COMPONENT_CIRCLE:
        this->generate = [](int thickness, QRgb color) -> VectorComponent* {
            return new VectorCircle(thickness, color);
        };
        break;
    case VECTOR_COMPONENT_HALF_CIRCLES:
        this->generate = [](int thickness, QRgb color) -> VectorComponent* {
            return new HalfCircles(thickness, color);
        };
        break;
    default:
        throw std::runtime_error("Invalid Component type");
    }

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

    this->generated = this->generate(this->base_thickness, this->base_color); //new LinePolygon(this->base_thickness, this->base_color);
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

void CustomGraphicsScene::deleteSelected() {
    if (this->active != nullptr){
        for (int i = 0; i < vec_items.size(); i++){
            if (this->active == vec_items[i]){
                delete vec_items[i];
                vec_items.erase(vec_items.begin()+i);
                this->active = nullptr;
                break;
            }
        }
    }
    this->render();
}

void CustomGraphicsScene::setColorSelected(bool) {
    if (this->active != nullptr){
        this->active->setColor(this->base_color);

        QTextStream(stdout) << QString::fromStdString(this->serializeComponents());
        //QTextStream(stdout) << QString::fromStdString(this->active->SerializeSelf());
    }

    this->render();
}

void CustomGraphicsScene::setThicknessSelected() {
    if (this->active != nullptr) {
        this->active->SetThickness(this->base_thickness);
    }
    this->render();
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

void CustomGraphicsScene::render(){
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

QRgb VectorComponent::getColor() {
    return this->color;
}

void VectorComponent::setColor(unsigned char r, unsigned char g, unsigned char b){
    this->color = qRgb(r, g, b);
}

void VectorComponent::setColor(QRgb col) {
    this->color = col;
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

void MidpointLine::RenderSelf(QImage* Image, bool antialias){
    if (this->points.size() < 2){
        throw std::logic_error("Cannot define a line with less that 2 points");
    }

    QPoint p1 = this->points[0];
    QPoint p2 = this->points[1];
    if (antialias){
        this->renderLineXiaolinWu(Image, p1, p2);
    }
    else {
        this->renderLine_M(Image, p1, p2);
    }
}

void MidpointLine::renderLine_M(QImage* Image, QPoint p1, QPoint p2) {
    // Based on Lecture 5 S. 11
    // and https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
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
        if (D < 0){
            D += 2 * dy;
        }
        else {
            y += yi;
            D += 2*(dy - dx);
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
        if (D < 0){
            D += 2 * dx;
        }
        else {
            x += xi;
            D += 2*(dx - dy);
        }
    }
}

void MidpointLine::renderLineXiaolinWu(QImage* image, QPoint p1, QPoint p2){
    QColor Line(this->color);
    QColor Back = Line;
    Back.setAlpha(0);

    QColor c1, c2;

    bool high = std::abs(p1.y() - p2.y()) > std::abs(p1.x() - p2.x());

    if (high) {
        p1 = QPoint(p1.y(), p1.x());
        p2 = QPoint(p2.y(), p2.x());
    }

    if (p1.x() > p2.x()){
        std::swap(p1, p2);
    }

    float dx = p2.x() - p1.x();
    float dy = p2.y() - p1.y();

    float m = (dx == 0) ? 1 : dy/dx;

    float y = p1.y();

    if (high){
        for (int x = p1.x(); x <= p2.x(); x++){
            c1 = VectorComponent::ColorMeld(Line, Back, y);
            c2 = VectorComponent::ColorMeld(Back, Line, y);
            image->setPixelColor((int)std::floor(y), x, c1);
            image->setPixelColor((int)std::floor(y) + 1, x , c2);
            y += m;
        }
    }
    else{
        for (int x = p1.x(); x <= p2.x(); x++){
            c1 = VectorComponent::ColorMeld(Line, Back, y);
            c2 = VectorComponent::ColorMeld(Back, Line, y);
            image->setPixelColor(x, (int)std::floor(y), c1);
            image->setPixelColor(x, (int)std::floor(y) + 1, c2);
            y += m;
        }
    }

}

QColor VectorComponent::ColorMeld(QColor c1, QColor c2, float pos){
    float* nop = new float;
    int r = c1.red()*(1-std::modf(pos, nop)) + c2.red()*std::modf(pos, nop);
    int g = c1.green()*(1-std::modf(pos, nop)) + c2.green()*std::modf(pos, nop);
    int b = c1.blue()*(1-std::modf(pos, nop)) + c2.blue()*std::modf(pos, nop);
    int a = c1.alpha()*(1-std::modf(pos, nop)) + c2.alpha()*std::modf(pos, nop);
    delete nop;
    return QColor(r, g, b, a);
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

void LinePolygon::RenderSelf(QImage* Image, bool antialias){
    if (!this->IsRenderable()){
        throw std::logic_error("Cannot define a polygon with less than 2 points");
    }
    if(antialias){
        for (int i=1; i<this->points.size(); i++){
            this->renderLineXiaolinWu(Image, this->points[i-1], this->points[i]);
        }
    }
    else{
        for (int i=1; i<this->points.size(); i++){
            this->renderLine_M(Image, this->points[i-1], this->points[i]);
        }
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

void VectorCircle::RenderSelf(QImage* Image, bool antialias) {
    if (!antialias) {
        this->RenderAltMidpointCircle(Image);
    }
    else {
        this->RenderXiaolinWuCircle(Image);
    }
}

void VectorCircle::RenderAltMidpointCircle(QImage *image){
    //Only the 0deg - 45deg octet is calculated; the rest are reflections about the center
    double tmp = VectorComponent::PointDistance(points[0], points[1]);
    int R = (int)( tmp + 0.5f - (tmp < 0) );

    int dE = 3;
    int dSE = 5-2*R;
    int d = 1-R;
    int x = points[0].x();
    int y = points[0].y() + R;
    //apply_brush(image, x, y, this->color);
    ApplyReflectedBrush(image, points[0], x, y, this->color);
    while ( (y - points[0].y()) > (x - points[0].x()))
    {
        if ( d < 0 ) //move to E
        {
            d += dE;
            dE += 2;
            dSE += 2;
        }
        else //move to SE
        {
            d += dSE;
            dE += 2;
            dSE += 4;
            --y;
        }
        ++x;
        ApplyReflectedBrush(image, points[0], x, y, this->color);
        //apply_brush(image, x, y, this->color);
    }
}

void VectorCircle::ApplyReflectedBrush(QImage *image, QPoint c, int x, int y, QRgb color){
    int x_offset = x - c.x();
    int y_offset = y - c.y();

    apply_brush(image, x, y, color);
    apply_brush(image, c.x() - x_offset, y , color);
    apply_brush(image, x, c.y() - y_offset , color);
    apply_brush(image, c.x() - x_offset, c.y() - y_offset , color);

    apply_brush(image, c.x() - y_offset, c.y() - x_offset, color);
    apply_brush(image, c.x() + y_offset, c.y() - x_offset, color);
    apply_brush(image, c.x() - y_offset, c.y() + x_offset, color);
    apply_brush(image, c.x() + y_offset, c.y() + x_offset, color);
}

void VectorCircle::RenderXiaolinWuCircle(QImage *image){

    QColor LineCol(this->color);
    //LineCol.setAlpha(255);

    int R = VectorComponent::PointDistance(points[0], points[1]);

    int x = R;
    int y = 0;

    QColor BgColor = LineCol;
    BgColor.setAlpha(0);

    float T;

    QColor c1;
    QColor c2;

    image->setPixel(points[0].x() + x, points[0].y() + y, LineCol.rgba());
    image->setPixel(points[0].x() + x, points[0].y() - y, LineCol.rgba());
    image->setPixel(points[0].x() - x, points[0].y() + y, LineCol.rgba());
    image->setPixel(points[0].x() - x, points[0].y() - y, LineCol.rgba());

    image->setPixel(points[0].x() + y, points[0].y() + x, LineCol.rgba());
    image->setPixel(points[0].x() + y, points[0].y() - x, LineCol.rgba());
    image->setPixel(points[0].x() - y, points[0].y() + x, LineCol.rgba());
    image->setPixel(points[0].x() - y, points[0].y() - x, LineCol.rgba());

    //BgColor = QColor(255,0,0);

    while ( x > y ) {
        ++y;

        //BgColor = QColor(image->pixelColor(points[0].x() + x, points[0].y() + y));

        x = std::ceil(std::sqrt(R*R - y*y));
        T = std::ceil(std::sqrt((double)(R*R - y*y))) - std::sqrt((double)(R*R - y*y));

        c2 = VectorComponent::ColorMeld(LineCol, BgColor, T);
        c1 = VectorComponent::ColorMeld(BgColor, LineCol, T);

        //90-135
        image->setPixel( points[0].x() + x, points[0].y() + y, c2.rgba());
        image->setPixel( points[0].x() + x-1, points[0].y() + y, c1.rgba());
        // 225-270
        image->setPixel( points[0].x() - x, points[0].y() + y, c2.rgba());
        image->setPixel( points[0].x() - (x-1), points[0].y() + y, c1.rgba());
        //45-90
        image->setPixel( points[0].x() + x, points[0].y() - y, c2.rgba());
        image->setPixel( points[0].x() + x-1, points[0].y() - y, c1.rgba());
        //270-315
        image->setPixel( points[0].x() - x, points[0].y() - y, c2.rgba());
        image->setPixel( points[0].x() - (x-1), points[0].y() - y, c1.rgba());

        //135-180
        image->setPixel( points[0].x() + y, points[0].y() + x, c2.rgba());
        image->setPixel( points[0].x() + y, points[0].y() + x-1, c1.rgba());
        //180-225
        image->setPixel( points[0].x() - y, points[0].y() + x, c2.rgba());
        image->setPixel( points[0].x() - y, points[0].y() + x-1, c1.rgba());
        //0-45
        image->setPixel( points[0].x() + y, points[0].y() - x, c2.rgba());
        image->setPixel( points[0].x() + y, points[0].y() - (x-1), c1.rgba());
        //315-360
        image->setPixel( points[0].x() - y, points[0].y() - x, c2.rgba());
        image->setPixel( points[0].x() - y, points[0].y() - (x-1), c1.rgba());
    }

}

MovedObject* VectorCircle::CheckClickMove(QPoint clickPos, double tolerance) {

    std::vector<QPoint*> pts;
    double R, tmp;

    if (VectorComponent::PointDistance(points[0], clickPos) <= tolerance){
        pts.push_back(&points[0]);
        goto fin;
    }

    R = VectorComponent::PointDistance(points[0], points[1]);
    tmp = VectorComponent::PointDistance(points[0], clickPos);

    if ( std::abs(R - tmp) <= tolerance ){
        points[1] = clickPos;
        pts.push_back(&points[1]);
        goto fin;
    }
    return nullptr;
fin:
    return new MovedObject(pts, clickPos);
    //throw std::logic_error("not implemented");
}
MovedObject* VectorCircle::MoveAll(QPoint clickPos, double tolerance) {
    std::vector<QPoint*> pts;

    MovedObject* tmp = this->CheckClickMove(clickPos, tolerance);

    if (tmp == nullptr){
        return nullptr;
    }
    delete tmp;

    pts.push_back(&points[0]);
    pts.push_back(&points[1]);

    return new MovedObject(pts, clickPos);
}

std::string VectorComponent::SerializeSelf(std::string prepend){
    if (!this->IsReady()){
        throw std::logic_error("Cannot serialize an unfinished object");
    }

    std::string self_rep;

    self_rep += prepend + "{\n";
    self_rep += prepend + "\t\"type\": \"" + this->TypeSelf().toStdString() + "\",\n";
    self_rep += prepend + "\t\"color\": [" + std::to_string(qRed(this->color)) + ", " + std::to_string(qGreen(this->color)) + ", " + std::to_string(qBlue(this->color)) + "],\n";
    self_rep += prepend + "\t\"thickness\": " + std::to_string(this->thickness) + ",\n";

    self_rep += prepend + "\t\"points\": [\n";

    for (QPoint &p : this->points){
        self_rep += prepend + "\t\t{\"x\": " + std::to_string(p.x()) + ", \"y\": " + std::to_string(p.y()) + "},\n";
    }
    self_rep.pop_back(); self_rep.pop_back(); self_rep += "\n";
    self_rep += prepend + "\t]\n";
    self_rep += prepend + "}";

    return self_rep;
}

std::string CustomGraphicsScene::serializeComponents(){
    std::string ser;

    std::string tmp;

    ser += "{\n";
    ser += "\t\"objects\": [\n";
    for (VectorComponent* vc : this->vec_items){
        try{
        tmp = vc->SerializeSelf("\t\t");
        }
        catch (std::logic_error e){
            QTextStream(stderr) << "attempted to serialize an unfinished object\n";
            continue;
        }

        ser += tmp + ",\n";
    }
    ser.pop_back(); ser.pop_back(); ser += "\n";
    ser += "\t]\n";
    ser += "}\n";
    return ser;
}

VECTOR_COMPONENT_TYPE CustomGraphicsScene::QStringToVCT(QString s) {
    if (s == "Line") return VECTOR_COMPONENT_LINE;
    else if (s == "LinePolygon") return VECTOR_COMPONENT_POLYGON;
    else if (s == "VectorCircle") return VECTOR_COMPONENT_CIRCLE;
    else if (s == "HalfCircles") return VECTOR_COMPONENT_HALF_CIRCLES;
    else return INVALID;
}

VectorComponent* (*CustomGraphicsScene::VCTToGenerator(VECTOR_COMPONENT_TYPE vct))(std::vector<QPoint>, int, QRgb) {
    switch (vct) {
    case VECTOR_COMPONENT_LINE:
        return [](std::vector<QPoint> pts, int thickness, QRgb col) -> VectorComponent* {return new MidpointLine(pts, thickness, col);};
    case VECTOR_COMPONENT_POLYGON:
        return [](std::vector<QPoint> pts, int thickness, QRgb col) -> VectorComponent* {return new LinePolygon(pts, thickness, col);};
    case VECTOR_COMPONENT_CIRCLE:
        return [](std::vector<QPoint> pts, int thickness, QRgb col) -> VectorComponent* {return new VectorCircle(pts, thickness, col);};
    case VECTOR_COMPONENT_HALF_CIRCLES:
        return [](std::vector<QPoint> pts, int thickness, QRgb col) -> VectorComponent* {return new HalfCircles(pts, thickness, col);};
    default:
        return nullptr;
    }
}

void CustomGraphicsScene::deserializeComponents(QFile *in){
    if(!in->isOpen()){
        throw std::invalid_argument("The provided file is not open");
    }

    std::vector<VectorComponent*> vc;

    VectorComponent* (*gen)(std::vector<QPoint>, int thickness, QRgb color);

    QString type;
    QRgb col;
    int r, g, b;
    int thickness;
    std::vector<QPoint> pts;
    VECTOR_COMPONENT_TYPE vct;

    QJsonObject base, current;

    QJsonArray objs, col_vals, verts;

    QJsonParseError* jpe = new QJsonParseError;

    QJsonDocument data = QJsonDocument::fromJson(QByteArray::fromRawData(in->readAll(), in->size()), jpe);

    if (jpe->error != QJsonParseError::NoError){
        QTextStream(stderr) << jpe->errorString() <<"\n";
        delete jpe;
        return;
    }
    delete jpe;

    //QTextStream(stdout) << data.toJson() << "\n";

    base = data.object();

    if (!base.contains(QString("objects")) || !base.value(QString("objects")).isArray()) {
        throw std::runtime_error("Invalid document format");
    }

    objs = base.value(QString("objects")).toArray();

    for (int i=0; i<objs.size(); i++){

        pts.clear();

        current = objs[i].toObject();

        if (!current.contains("type")       || !current.value("type").isString()     ||
            !current.contains("color")      || !current.value("color").isArray()     ||
            !current.contains("thickness")  || !current.value("thickness").isDouble()||
            !current.contains("points")     || !current.value("points").isArray()   )
                continue;

        type = current.value("type").toString();

        vct = CustomGraphicsScene::QStringToVCT(type);
        if (vct == INVALID){
            QTextStream(stderr) << "Unrecognised component type. Skipping\n";
            continue;
        }

        gen = CustomGraphicsScene::VCTToGenerator(vct);

        thickness = current.value("thickness").toInt();

        col_vals = current.value("color").toArray();

        if (col_vals.size() != 3) continue;//invalid

        r = col_vals[0].toInt();
        g = col_vals[1].toInt();
        b = col_vals[2].toInt();

        col = qRgb(r, g, b);

        verts = current.value("points").toArray();

        for (int j=0; j < verts.size(); j++){
            QJsonObject p = verts[j].toObject();
            pts.push_back(QPoint(p.value("x").toInt(), p.value("y").toInt()));
        }

        vc.push_back(gen(pts, thickness, col));
    }

    this->ClearVectorComponents();
    this->vec_items = vc;
    this->render();

    return;

err:
    for (VectorComponent *c : vc){
        delete c;
    }
    vc.clear();
    throw std::runtime_error("Invalid document format");

}

void HalfCircles::RenderSelf(QImage* Image, bool anitalias) {
    int R = this->PointDistance(this->points[0], this->points[1])/(this->no_halfcircles * 2);

    int dx = points[1].x() - points[0].x();
    int dy = points[1].y() - points[0].y();

    int Rx, Ry;
    if (dx == 0) {
        Ry = R;
        Rx = 0;
    }
    else if (dy == 0) {
        Ry=0;
        Rx=R;
    }
    else {
        double tmp = std::atan( ((double)dy)/((double)dx) );
        Rx = R * std::cos(tmp);
        Ry = R * std::sin(tmp);

    }

    if (points[0].x() > points[1].x()){
        Rx *= (-1);
        Ry *= (-1);
    }

    std::vector<QPoint> centers;

    for (int i=0; i < no_halfcircles; i++){
        centers.push_back(QPoint(points[0].x() + (i)*(dx/no_halfcircles) + Rx, points[0].y() + (i)*(dy/no_halfcircles) + Ry ));
    }

    for (int i=0; i< no_halfcircles; i++){
        this->RenderAltMidpointCircle(Image, centers[i], R, this->points[0], this->points[1], (bool)(i%2));
    }

}


void HalfCircles::RenderAltMidpointCircle(QImage *image, QPoint center, int R, QPoint l1, QPoint l2, bool side){
    //Only the 0deg - 45deg octet is calculated; the rest are reflections about the center
    //double tmp = VectorComponent::PointDistance(points[0], points[1]);
    //int R = (int)( tmp + 0.5f - (tmp < 0) );

    int dE = 3;
    int dSE = 5-2*R;
    int d = 1-R;
    int x = center.x();
    int y = center.y() + R;
    //apply_brush(image, x, y, this->color);

    //int sidedness = (x - l1.x())*(l2.y()-l1.y()) - (y-l1.y()) * (l2.x() - l1.x());

    ApplyReflectedBrush(image, center, x, y, this->color, l1, l2, side);
    while ( (y - center.y()) > (x - center.x()))
    {
        if ( d < 0 ) //move to E
        {
            d += dE;
            dE += 2;
            dSE += 2;
        }
        else //move to SE
        {
            d += dSE;
            dE += 2;
            dSE += 4;
            --y;
        }
        ++x;
        //sidedness = (x - l1.x())*(l2.y()-l1.y()) - (y-l1.y()) * (l2.x() - l1.x());
        ApplyReflectedBrush(image, center, x, y, this->color, l1, l2, side);
        //apply_brush(image, x, y, this->color);
    }
}

void HalfCircles::ApplyReflectedBrush(QImage *image, QPoint c, int x, int y, QRgb color, QPoint l1, QPoint l2, bool side){
    int x_offset = x - c.x();
    int y_offset = y - c.y();

    int (*sidedness)(int , int, QPoint, QPoint);

    if (side){
        sidedness = [](int x, int y, QPoint l1, QPoint l2) -> int {
            return (x - l1.x())*(l2.y()-l1.y()) - (y-l1.y()) * (l2.x() - l1.x());
        };
    }
    else{
        sidedness = [](int x, int y, QPoint l1, QPoint l2) -> int {
            return (-1)*((x - l1.x())*(l2.y()-l1.y()) - (y-l1.y()) * (l2.x() - l1.x()));
        };
    }


    if (sidedness(x, y, l1, l2) >= 0){
        apply_brush(image, x, y, color);
    }
    if (sidedness(c.x() - x_offset, y, l1, l2) >= 0){
        apply_brush(image, c.x() - x_offset, y , color);
    }
    if (sidedness(x, c.y() - y_offset, l1, l2) >= 0){
        apply_brush(image, x, c.y() - y_offset , color);
    }
    if (sidedness(c.x() - x_offset, c.y() - y_offset, l1, l2) >= 0){
        apply_brush(image, c.x() - x_offset, c.y() - y_offset , color);
    }
    if (sidedness(c.x() - y_offset, c.y() - x_offset, l1, l2) >= 0){
        apply_brush(image, c.x() - y_offset, c.y() - x_offset, color);
    }
    if (sidedness(c.x() + y_offset, c.y() - x_offset, l1, l2) >= 0){
        apply_brush(image, c.x() + y_offset, c.y() - x_offset, color);
    }
    if (sidedness(c.x() - y_offset, c.y() + x_offset, l1, l2) >= 0){
        apply_brush(image, c.x() - y_offset, c.y() + x_offset, color);
    }
    if (sidedness(c.x() + y_offset, c.y() + x_offset, l1, l2) >= 0) {
        apply_brush(image, c.x() + y_offset, c.y() + x_offset, color);
    }
}
