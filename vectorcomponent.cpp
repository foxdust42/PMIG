#include "vectorcomponent.h"

#include <QTextStream>
#include <QWidget>
#include <QPainter>
#include <cmath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <queue>

#include <Qt3DCore/Qt3DCore>


/*
VectorComponent::VectorComponent(std::vector<QPoint> points) {
    this->points = points;
}
*/
void CustomGraphicsScene::init(){
    this->vec_over = new QGraphicsPixmapItem();
    this->bg_image = new QGraphicsPixmapItem();

    this->target = QImage(":/vec/target.png");
    if (target.isNull()){
        throw std::runtime_error("Failed to load resource");
    }

    this->addItem(bg_image);
    this->addItem(vec_over);

    this->setVectorComponentType(VECTOR_COMPONENT_LINE);

    this->scene = new C3DScene(this->width(), this->height(),
                               &MidpointLine::renderLine_M, &VectorComponent::GenerateBrushMask);

    std::vector<QRgb> cols1 = {
        qRgb(0,  255/6,0), qRgb(0,  255/6,0),
        qRgb(0,2*255/6,0), qRgb(0,2*255/6,0),
        qRgb(0,3*255/6,0), qRgb(0,3*255/6,0),
        qRgb(0,4*255/6,0), qRgb(0,4*255/6,0),
        qRgb(0,5*255/6,0), qRgb(0,5*255/6,0),
        qRgb(0,6*255/6,0), qRgb(0,6*255/6,0)
    };

    std::vector<QRgb> cols2 = {
        qRgb(0,  255/6,200), qRgb(0,  255/6,200),
        qRgb(0,2*255/6,200), qRgb(0,2*255/6,200),
        qRgb(0,3*255/6,200), qRgb(0,3*255/6,200),
        qRgb(0,4*255/6,200), qRgb(0,4*255/6,200),
        qRgb(0,5*255/6,200), qRgb(0,5*255/6,200),
        qRgb(0,6*255/6,200), qRgb(0,6*255/6,200)
    };

    CuboidMesh *c2 = new CuboidMesh(this->scene);
    c2->OffsetSelf(QVector3D(1,0,0));
    c2->SetTriagColors(cols1);

    this->scene->AddObject(c2);

    this->cm = new CuboidMesh(this->scene);
    this->cm->OffsetSelf(QVector3D(-1,0,0));
    this->cm->SetTriagColors(cols2);

    this->scene->AddObject(cm);

    QObject::connect(&this->refresh, &QTimer::timeout, this, &CustomGraphicsScene::Tick);
    QObject::connect(this, &QGraphicsScene::sceneRectChanged, this->scene, &C3DScene::Resize);

    refresh.start(updateINtervalMS);
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

                if (c == Qt::transparent) {
                    c = background->pixelColor(curr_point);
                }

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

    for (int i=0; i < floodFills.size(); i++){
        delete floodFills[i];
    }
    floodFills.clear();

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
    case VECTOR_COMPONENT_RECTANGLE:
        this->generate = [](int thickness, QRgb color) -> VectorComponent* {
            return new VectorRectangle(thickness, color);
        };
        break;
    case VECTOR_COMPONENT_FLOOD_FILL:
        this->generate = [](int thickness, QRgb color) -> VectorComponent* {
            return new FloodFill(thickness, color);
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
                if (CustomGraphicsScene::QStringToVCT(this->generated->TypeSelf()) == VECTOR_COMPONENT_FLOOD_FILL ){
                    this->floodFills.push_back((FloodFill*)generated);
                }
                else {
                    this->addComponent(generated);
                }
            }
            else {
                delete generated;
            }
        }
        generated = nullptr;
        moved = nullptr;
        active = nullptr;
        this->is_clipping = false;
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
        if ( ((event->modifiers() & Qt::KeyboardModifier::AltModifier) != 0) || this->is_clipping){
            QTextStream(stdout) << "ALT\n";
            this->is_clipping = false;
            MovedObject* m_tmp;
            active->ClearClip();
            for (int i=0; i< vec_items.size(); i++){
                m_tmp = vec_items[i]->CheckClickMove(event->pos().toPoint(), hit_margin);
                if (m_tmp != nullptr){
                    if (vec_items[i]->CanClipTo()){
                        active->SetClip(vec_items[i]);
                        QTextStream("Set Clip\n");
                    }
                    else{
                        //PMIG::scream(QString("Cannot Clip To object"), QMessageBox::Critical, QString("Cannot clip to this vector component"), QString());
                    }
                    delete m_tmp;
                    break;
                }
            }
            QTextStream(stdout) << (active->GetClip() == nullptr ? "no clip " : "yes clip ") << "\n";
            goto render;
        }
        moved->MoveTo(event->pos().toPoint());
        //
        //QTextStream(stdout) << active->IsConvex() << "\n";
        //
        //if (event->type() == QEvent::Type::MouseButtonRelease){
        //    delete moved;
        //    moved = nullptr;
        //}
        goto render;
    }
    if (this->generated != nullptr){
        this->generated->addPoint(event->pos().toPoint());

        if (this->generated->IsReady()){
            if (CustomGraphicsScene::QStringToVCT(this->generated->TypeSelf()) == VECTOR_COMPONENT_FLOOD_FILL){
                this->floodFills.push_back((FloodFill*)this->generated);
            }
            else {
                this->addComponent(this->generated);
            }
            this->generated = nullptr;
        }
        else if(this->generated->IsRenderable() && CustomGraphicsScene::QStringToVCT(this->generated->TypeSelf()) != VECTOR_COMPONENT_FLOOD_FILL ){
            this->generated->RenderSelf(vec_raster, this->antialias);
        }
        goto render;
    }
    if (this->active != nullptr) {
        //moved = active->MoveAll(event->pos().toPoint(), hit_margin);
        //if (moved == nullptr){
        //}
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

    this->invalidate();

    this->vec_over->setPixmap(QPixmap::fromImage(*vec_raster));

    for (FloodFill* fl : this->floodFills){
        fl->RenderSelf(vec_raster, this->antialias);
    }

    if (moved != nullptr) {
        renderTargetPoints(vec_raster, moved->PointVals(), base_image);
    }

    if (active != nullptr && moved == nullptr){
        renderTargetPoints(vec_raster, this->active->points, base_image);
    }

    //cm->RenderSelf(vec_raster, false);

    //cm->degY = (cm->degY + M_PI * 0.1) >= M_PI * 2 ? 0.0 : (cm->degY + M_PI * 0.1);

    this->vec_over->setPixmap(QPixmap::fromImage(*vec_raster));

    this->invalidate();

    emit this->mousePressed(event);

}

void CustomGraphicsScene::Tick() {
    if (vec_raster == nullptr){
//        delete vec_raster;
        vec_raster = new QImage(bg_image->pixmap().width(), bg_image->pixmap().height(), QImage::Format_RGBA64);
    }
    vec_raster->fill(Qt::transparent);

    scene->RenderOnto(vec_raster);

    //scene->mainCam->OrbitTickX(M_PI/256);

    //scene->mainCam->OffsetPos(QVector3D(0.0, 0.05, 0.0));

    // this->cm->OffsetSelf(QVector3D(0, 0.01, 0));

    this->cm->RotateSelfX(M_PI/256);
    //this->cm->RotateSelfY(M_PI/128);
    this->cm->RotateSelfZ(M_PI/256);

    this->vec_over->setPixmap(QPixmap::fromImage(*vec_raster));

    this->invalidate();
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

bool CustomGraphicsScene::SetClip() {
    if (this->active == nullptr){
        return false;
    }
    this->is_clipping = true;
    return true;
}

void CustomGraphicsScene::setColorSelected(bool) {
    if (this->active != nullptr){
        this->active->setColor(this->base_color);

        //QTextStream(stdout) << QString::fromStdString(this->serializeComponents());
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

void CustomGraphicsScene::FillPolygon(){
    if (this->active == nullptr){
        return;
    }
    if (this->active->GetFill()){
        this->active->ClearFill();
    }
    else {
        this->active->setFillColor(this->base_color);
        this->active->SetFill();
    }
    this->render();
}

bool CustomGraphicsScene::CanSetImage(){
    if (this->active == nullptr || !this->active->CanFill()){
        return false;
    }
    else{
        return true;
    }
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

        // if (moved != nullptr) {
        //     renderTargetPoints(vec_raster, moved->PointVals(), base_image);
        // }

        // if (active != nullptr && moved == nullptr){
        //     renderTargetPoints(vec_raster, this->active->points, base_image);
        // }

        this->vec_over->setPixmap(QPixmap::fromImage(*vec_raster));

        this->invalidate();
    }
}

void CustomGraphicsScene::render(){
    if (vec_raster == nullptr){
//        delete vec_raster;
        vec_raster = new QImage(bg_image->pixmap().width(), bg_image->pixmap().height(), QImage::Format_RGBA64);
    }
    vec_raster->fill(Qt::transparent);

    this->renderVectorComponents();

    this->vec_over->setPixmap(QPixmap::fromImage(*vec_raster));

    for (FloodFill* fl : this->floodFills){
        fl->RenderSelf(vec_raster, this->antialias);
    }


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

void VectorComponent::setClipColor(unsigned char r, unsigned char g, unsigned char b){
    this->clipColor = qRgb(r,g,b);
}
void VectorComponent::setClipColor(QRgb col){
    this->clipColor = col;
}
QRgb VectorComponent::getClipColor(){
    return this->clipColor;
}

void VectorComponent::setFillColor(unsigned char r, unsigned char g, unsigned char b){
    this->fillColor = qRgb(r,g,b);
}
void VectorComponent::setFillColor(QRgb col){
    this->fillColor = col;
}
QRgb VectorComponent::getFillColor(){
    return this->fillColor;
}


void CustomGraphicsScene::SetFillImage(QImage img) {
    if(this->active == nullptr) {return;}
    if(!this->active->CanFill()){return;}
    if(this->active->isFillImage()){
        this->active->clearFillImage();
        this->active->ClearFill();
    }
    this->active->setFillImage(img);
    this->active->SetFill();
    this->render();
}

void CustomGraphicsScene::ClearFillImage() {
    if(this->active == nullptr) {return;}
    if(!this->active->CanFill()){return;}
    this->active->clearFillImage();
    this->active->ClearFill();
    this->render();
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

void VectorComponent::apply_brush(QImage *image, int x, int y, QRgb color, int thickness, std::vector<std::vector<int> > brush_mask){
    for(int i=0; i < thickness; i++) {
        for (int j=0; j < thickness; j++){
            if (brush_mask[i][j] == 1){
                image->setPixel(clamp(x + i - thickness/2, 0, image->width()), clamp(y + j - thickness/2, 0, image->height()), color);
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

void VectorComponent::FillVertexSorting(QImage *Image, VectorComponent *toFill, QRgb fillColor, QImage* fillImage) {
    if (!toFill->CanFill()){
        return;
    }
    AETE (*addAETEntry)(QPoint p1, QPoint p2) = [](QPoint p1, QPoint p2) -> AETE{
        QPoint start, end;
        if (p1.y() > p2.y()){
            start = p2;
            end = p1;
        }
        else {
            start = p1;
            end = p2;
        }
        int dy = end.y() - start.y();
        int dx = end.x() - start.x();
        double m_inv = ((double)dx)/((double)dy);
        return AETE{end.y(), (double)start.x(), m_inv};
    };
    std::vector<AETE> AET;

    std::vector<int> indicies = std::vector<int>(toFill->GetPoints().size()-1);
    std::iota(indicies.begin(), indicies.end(), 0);

    std::sort(indicies.begin(), indicies.end(), [&toFill](int a, int b) -> bool {
        if(toFill->GetPoints()[a].y() < toFill->GetPoints()[b].y()){
            return true;
        }
        return false;
    });

    int k=0;
    int i = indicies[k];
    int y = toFill->GetPoints()[indicies[0]].y();
    //int y_min = y;
    int y_max = toFill->GetPoints()[indicies[indicies.size()-1]].y();

    while (y < y_max) {
        while (toFill->GetPoints()[i].y() == y){
            int tmp = (i-1) < 0 ? toFill->GetPoints().size()-2 : i-1;
            if (toFill->GetPoints()[tmp].y() > toFill->GetPoints()[i].y()) {
                AET.push_back(addAETEntry(toFill->GetPoints()[i], toFill->GetPoints()[tmp]));
            }
            if (toFill->GetPoints()[i+1].y() > toFill->GetPoints()[i].y()) {
                AET.push_back(addAETEntry(toFill->GetPoints()[i], toFill->GetPoints()[(i+1)%(toFill->GetPoints().size()-1)]));
            }
            k++;
            i = indicies[k];
        }
        //addAETEntry inserts sorted
        std::sort(AET.begin(), AET.end(), [](AETE a, AETE b) -> bool {
            if (a.x < b.x){
                return true;
            }
            return false;
        });

        VectorComponent::RenderScanLine(Image, AET, y, fillColor, fillImage);
        y++;
        //remove entries with
        {
            std::vector<AETE> newAET;
            for (int j = 0; j < AET.size(); j++){
                if(AET[j].y_max != y){
                    newAET.push_back(AET[j]);
                }
            }
            AET = newAET;
        }
        for ( int j = 0; j < AET.size(); j++ ){
            AET[j].x += AET[j].inv_m;
        }
    }
}

void VectorComponent::RenderScanLine(QImage *Image, std::vector<AETE> AET, int y, QRgb col, QImage* fillImage){
    int x, next_x;
    if (fillImage != nullptr){
        goto img_fill;
    }
    for (int i=0; i+1<AET.size(); i+=2 ){
        x = AET[i].x;
        next_x = AET[i+1].x;
        // if (x == next_x){
        //     if (i+2 >= AET.size() ){
        //         return;
        //     }
        //     i += 1;
        //     next_x = AET[i+1].x;
        // }
        for (int j = x; j < next_x; j++){
            Image->setPixel(QPoint(j, y), col);
        }
    }
    return;
img_fill:
    for (int i=0; i+1<AET.size(); i+=2 ){
        x = AET[i].x;
        next_x = AET[i+1].x;
        // if (x == next_x){
        //     if (i+2 >= AET.size() ){
        //         return;
        //     }
        //     i += 1;
        //     next_x = AET[i+1].x;
        // }
        for (int j = x; j < next_x; j++){
            Image->setPixel(QPoint(j, y), fillImage->pixel(j % fillImage->width(), y % fillImage->height()));
        }
    }
    return;
}

void MidpointLine::RenderSelf(QImage* Image, bool antialias){
    if (this->points.size() < 2){
        throw std::logic_error("Cannot define a line with less that 2 points");
    }

    std::vector<std::pair<QPoint, QPoint> > lines;
    std::vector<std::pair<QPoint, QPoint> > clipped;
    std::vector<std::pair<QPoint, QPoint> > unclipped;

    lines.push_back(std::pair<QPoint, QPoint>(this->points[0], this->points[1]));

    if( this->clipTo != nullptr && this->clipTo->CanClipTo()){
        clipped = MidpointLine::ClipLineCyrusBeck(lines, this->clipTo, &unclipped);
    } else {
        clipped = lines;
        unclipped.clear();
    }

    if (antialias){
        for (std::pair<QPoint, QPoint> line : clipped) {
            MidpointLine::renderLineXiaolinWu(Image, line.first, line.second, this->color, this->thickness, this->brush_mask);
        }
        for (std::pair<QPoint, QPoint> line : unclipped) {
            MidpointLine::renderLineXiaolinWu(Image, line.first, line.second, this->clipColor, this->thickness, this->brush_mask);
        }
    }
    else {
        for (std::pair<QPoint, QPoint> line : clipped) {
            MidpointLine::renderLine_M(Image, line.first, line.second, this->color, this->thickness, this->brush_mask);
        }
        for (std::pair<QPoint, QPoint> line : unclipped) {
            MidpointLine::renderLine_M(Image, line.first, line.second, this->clipColor, this->thickness, this->brush_mask);
        }
    }

}

std::vector<QPoint> VectorComponent::GetOutwardNormals(std::vector<edge> edges, QPoint center, std::vector<QPoint> *lineAnchors) {
    std::vector<QPoint> normals;
    std::vector<QPoint> anchors;
    QPoint anchor;
    QPoint normal;
    for (int i=0; i < edges.size(); i++){
        normal = VectorComponent::GetOutwardNormal(edges[i].first, edges[i].second, center, &anchor);
        normals.push_back(normal);
        anchors.push_back(anchor);
    }
    if (lineAnchors != nullptr) {
        *lineAnchors = anchors;
    }
    return normals;
}

std::vector<std::pair<QPoint, QPoint> > MidpointLine::ClipLineCyrusBeck(const std::vector<edge> lines, const VectorComponent* const against, std::vector<std::pair<QPoint, QPoint> > *unclipped){

    std::vector<edge> clip;
    std::vector<edge> no_clip;

    std::vector<edge> edges = against->GetEdges();
    std::vector<QPoint> lineAnchors;

    std::vector<QPoint> OutNormals =  VectorComponent::GetOutwardNormals(edges, against->VertexCenterOfMass(), &lineAnchors);

    bool discard = false;

    double (*get_t)(QPoint normal, QPoint P_0, QPoint P_E, QPoint D) = [](QPoint normal, QPoint P_0, QPoint P_E, QPoint D) -> double {
        return ((double)VectorComponent::InnerProduct(normal, (P_0 - P_E))) / ((double)VectorComponent::InnerProduct(-normal, D));
    };

    for (std::pair<QPoint, QPoint> line : lines) {
        if (line.first == line.second) {
            continue;
        }
        discard = false;
        QPoint D = line.second - line.first;
        double tE = 0.0, tL = 1.0;
        for (int i = 0; i < edges.size(); i++) {
            if (VectorComponent::InnerProduct(OutNormals[i], D) == 0){ // parallel lines
                if (VectorComponent::InnerProduct(OutNormals[i], (line.second - lineAnchors[i]) )) {
                    discard = true;
                    break;
                }
            }
            else {
                double t = get_t(OutNormals[i], line.first, lineAnchors[i], D);
                if (VectorComponent::InnerProduct(OutNormals[i], D) < 0) {
                        tE = std::max(tE, t);
                }
                else {
                    tL = std::min(tL, t);
                }
            }
        }
        if (tE > tL || discard) {
            no_clip.push_back(line);
        }
        else {
            QPoint clipStart, clipEnd;
            if (tE > 0.0) {
                //clipStart = line.first + D * tE;
                clipStart.rx() = line.first.x() + D.x() * tE;
                clipStart.ry() = line.first.y() + D.y() * tE;
                no_clip.push_back(edge(line.first, clipStart));
            }
            else {
                clipStart = line.first;
            }
            if (tL < 1.0) {
                clipEnd = line.first + D * tL;
                no_clip.push_back(edge(clipEnd, line.second));
            }
            else {
                clipEnd = line.second;
            }
            clip.push_back(edge(clipStart, clipEnd));
        }
    }
    if (unclipped != nullptr) {
        *unclipped = no_clip;
    }
    return clip;
}


QPoint VectorComponent::GetOutwardNormal(QPoint p1, QPoint p2, QPoint center, QPoint *lineAnchor){
    QPoint anchor = QPoint((p1.x() + p2.x())/2, (p1.y() + p2.y())/2);
    QPoint norm = QPoint(-(p2.y() - p1.y()), p2.x() - p1.x());
    QPoint to_anchor = QPoint((anchor.x() - center.x()), anchor.y() - center.y());
    if (lineAnchor != nullptr) {
        lineAnchor->rx() = anchor.x();
        lineAnchor->ry() = anchor.y();
    }
    if (VectorComponent::InnerProduct(norm, to_anchor) > 0){
        return norm;
    } else {
        return (-1)*norm;
    }
}


void MidpointLine::renderLine_M(QImage* Image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask) {
    // Based on Lecture 5 S. 11
    // and https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    if ( std::abs(p2.y() - p1.y()) < std::abs(p2.x() - p1.x()) ){
        if (p1.x() > p2.x()){
            renderLineLo(Image, p2, p1, c, thickness, brush_mask);
        }
        else {
            renderLineLo(Image, p1, p2, c, thickness, brush_mask);
        }
    }
    else {
        if(p1.y() > p2.y()){
            renderLineHi(Image, p2, p1, c, thickness, brush_mask);
        }
        else {
            renderLineHi(Image, p1, p2, c, thickness, brush_mask);
        }
    }
}

void MidpointLine::renderLineLo(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask){
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
        apply_brush(image, x, y, c, thickness, brush_mask);
        if (D < 0){
            D += 2 * dy;
        }
        else {
            y += yi;
            D += 2*(dy - dx);
        }
    }
}

void MidpointLine::renderLineHi(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask){
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
        apply_brush(image, x, y, c, thickness, brush_mask);
        if (D < 0){
            D += 2 * dx;
        }
        else {
            x += xi;
            D += 2*(dx - dy);
        }
    }
}

void MidpointLine::renderLineXiaolinWu(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask){
    QColor Line(c);
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
    std::vector<std::pair<QPoint, QPoint> > lines;
    std::vector<std::pair<QPoint, QPoint> > clipped;
    std::vector<std::pair<QPoint, QPoint> > unclipped;

    //lines.push_back(std::pair<QPoint, QPoint>(this->points[0], this->points[1]));

    if (this->fill){

        VectorComponent::FillVertexSorting(Image, this, this->fillColor, this->isFillImage() ? &this->FillImage : nullptr);
    }

    lines = this->GetEdges();

    if( this->clipTo != nullptr && this->clipTo->CanClipTo()){
        clipped = MidpointLine::ClipLineCyrusBeck(lines, this->clipTo, &unclipped);
    } else {
        clipped = lines;
        unclipped.clear();
    }

    if (antialias){
        for (std::pair<QPoint, QPoint> line : clipped) {
            MidpointLine::renderLineXiaolinWu(Image, line.first, line.second, this->color, this->thickness, this->brush_mask);
        }
        for (std::pair<QPoint, QPoint> line : unclipped) {
            MidpointLine::renderLineXiaolinWu(Image, line.first, line.second, this->clipColor, this->thickness, this->brush_mask);
        }
    }
    else {
        for (std::pair<QPoint, QPoint> line : clipped) {
            MidpointLine::renderLine_M(Image, line.first, line.second, this->color, this->thickness, this->brush_mask);
        }
        for (std::pair<QPoint, QPoint> line : unclipped) {
            MidpointLine::renderLine_M(Image, line.first, line.second, this->clipColor, this->thickness, this->brush_mask);
        }
    }

}

bool LinePolygon::IsConvex() {
    //Note: A self intersecting polygon is not convex, even if all "interior" angles are less than 180 deg
    // https://stackoverflow.com/a/45372025
    if (this->points.size() < 3) {
        return false;
    }
    double angle_sum = 0.0;
    double angle;
    QPoint p0, p1;
    p0 = points[0];
    p1 = points[1];
    double o_dir;
    double n_dir = std::atan2(p1.y() - p0.y(), p1.x() - p0.x());
    double orientation;
    for (int i=2; i< points.size() + 1; i++){
        p0 = p1;
        p1 = points[(i)%(points.size()-1)];
        o_dir = n_dir;
        n_dir = std::atan2(p1.y() - p0.y(), p1.x() - p0.x());
        angle = n_dir - o_dir;
        if (angle < -M_PI) {
            angle += 2 * M_PI;
        }
        else if (angle > M_PI) {
            angle -= 2 * M_PI;
        }
        if (i == 2) {
            if (angle == 0.0) {return false;}
            orientation = angle > 0.0 ? 1.0 : -1.0;
        }
        else {
            if (orientation * angle <= 0.0) {return false;}
        }
        angle_sum += angle;
    }

    return std::abs((int)std::round(angle_sum/(2*M_PI))) == 1;

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

    apply_brush(image, x, y, color, thickness, brush_mask);
    apply_brush(image, c.x() - x_offset, y , color, thickness, brush_mask);
    apply_brush(image, x, c.y() - y_offset , color, thickness, brush_mask);
    apply_brush(image, c.x() - x_offset, c.y() - y_offset , color, thickness, brush_mask);

    apply_brush(image, c.x() - y_offset, c.y() - x_offset, color, thickness, brush_mask);
    apply_brush(image, c.x() + y_offset, c.y() - x_offset, color, thickness, brush_mask);
    apply_brush(image, c.x() - y_offset, c.y() + x_offset, color, thickness, brush_mask);
    apply_brush(image, c.x() + y_offset, c.y() + x_offset, color, thickness, brush_mask);
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

QString VectorComponent::SerializeSelf(QString prepend){
    if (!this->IsReady()){
        throw std::logic_error("Cannot serialize an unfinished object");
    }

    QString self_rep;

    self_rep += prepend + "{\n";
    self_rep += prepend + "\t\"type\": \"" + QString(this->TypeSelf()) + "\",\n";
    self_rep += prepend + "\t\"color\": [" + QString::number(qRed(this->color)) + ", " + QString::number(qGreen(this->color)) + ", " + QString::number(qBlue(this->color)) + "],\n";
    self_rep += prepend + "\t\"thickness\": " + QString::number(this->thickness) + ",\n";
    self_rep += prepend + "\t\"fill\": " + QString::number(this->fill) + ",\n";
    self_rep += prepend + "\t\"fill_color\": [" + QString::number(qRed(this->fillColor)) + ", " + QString::number(qGreen(this->fillColor)) + ", " + QString::number(qBlue(this->fillColor)) + "],\n";
    self_rep += prepend + "\t\"points\": [\n";

    for (QPoint &p : this->points){
        self_rep += prepend + "\t\t{\"x\": " + QString::number(p.x()) + ", \"y\": " + QString::number(p.y()) + "},\n";
    }
    self_rep.chop(2);
    self_rep += "\n";
    self_rep += prepend + "\t]";
    if(this->imgFill){
        QByteArray bArr;
        self_rep += ",\n";
        QTextStream ts(&bArr);
        self_rep += prepend + "\t\"image\": {\n";
        self_rep += prepend + "\t\t\"width\": "  + QString::number(this->FillImage.width()) + ",\n";
        self_rep += prepend + "\t\t\"height\": " + QString::number(this->FillImage.height()) + ",\n";
        self_rep += prepend + "\t\t\"format\": " + QString::number(this->FillImage.format()) + ",\n";
        self_rep += prepend + "\t\t\"data\": [";
        //iod.open(QIODevice::WriteOnly);
        //QTextStream(stdout) << this->FillImage.save(&iod, "PNG") << "\n";
        //iod.close();
        //iod.open(QIODevice::ReadOnly);
        //QTextStream(stdout) << QString(iod.readAll()) << "\n";
        //self_rep += QString(iod.readAll());
        //uchar *bits = this->FillImage.scanLine(2);
        for (int i = 0; i < this->FillImage.height(); i++){
            QRgb* line = (QRgb*)this->FillImage.scanLine(i);
            for (int j=0; j < this->FillImage.width(); j++){
                QString r = QString::number(qRed(line[j]),16);
                QString g = QString::number(qGreen(line[j]),16);
                QString b = QString::number(qBlue(line[j]), 16);
                if (r.size() == 1){r = "0" + r;}
                if (g.size() == 1){g = "0" + g;}
                if (b.size() == 1){b = "0" + b;}
                self_rep += "\"" + r + g + b + "\", ";
            }
        }
        self_rep.chop(2);
        self_rep += "]\n";
        self_rep += prepend + "\t}\n";
    }
    else{
        self_rep += "\n";
    }
    self_rep += prepend + "}";

    //QTextStream(stdout) << self_rep << "\n";
    return self_rep;
}

QString CustomGraphicsScene::serializeComponents(){
    QString ser;

    QString tmp;

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
    ser.chop(2); ser += "\n";
    ser += "\t]\n";
    ser += "}\n";
    //QTextStream(stdout) << ser << "\n";
    return ser;
}

VECTOR_COMPONENT_TYPE CustomGraphicsScene::QStringToVCT(QString s) {
    if (s == "Line") return VECTOR_COMPONENT_LINE;
    else if (s == "LinePolygon") return VECTOR_COMPONENT_POLYGON;
    else if (s == "VectorCircle") return VECTOR_COMPONENT_CIRCLE;
    else if (s == "HalfCircles") return VECTOR_COMPONENT_HALF_CIRCLES;
    else if (s == "Rectangle") return VECTOR_COMPONENT_RECTANGLE;
    else if (s == "FloodFill") return VECTOR_COMPONENT_FLOOD_FILL;
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
    case VECTOR_COMPONENT_RECTANGLE:
        return [](std::vector<QPoint> pts, int thickenss, QRgb col) -> VectorComponent* {return new VectorRectangle(pts, thickenss, col);};
    case VECTOR_COMPONENT_FLOOD_FILL:
        return [](std::vector<QPoint> pts, int thickenss, QRgb col) -> VectorComponent* {return new FloodFill(pts, thickenss, col);};
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
    QRgb col, fill_col;
    int r, g, b;
    int thickness, fill;
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

        if (!current.contains("type")       || !current.value("type").isString()        ||
            !current.contains("color")      || !current.value("color").isArray()        ||
            !current.contains("thickness")  || !current.value("thickness").isDouble()   ||
            !current.contains("points")     || !current.value("points").isArray()       ||
            !current.contains("fill")       || !current.value("fill").isDouble()        ||
            !current.contains("fill_color") || !current.value("fill_color").isArray()    )
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

        col_vals = current.value("fill_color").toArray();

        if (col_vals.size() != 3) continue;//invalid

        r = col_vals[0].toInt();
        g = col_vals[1].toInt();
        b = col_vals[2].toInt();

        fill_col = qRgb(r, g, b);

        fill = current.value("fill").toInt();

        verts = current.value("points").toArray();

        for (int j=0; j < verts.size(); j++){
            QJsonObject p = verts[j].toObject();
            pts.push_back(QPoint(p.value("x").toInt(), p.value("y").toInt()));
        }
        VectorComponent* vec = gen(pts, thickness, col);

        vec->setFillColor(fill_col);

        if (fill) { vec->SetFill(); }
        else{ vec->ClearFill(); }

        if (current.contains("image") && current.value("image").isObject()){
            QJsonObject imgObj = current.value("image").toObject();
            if( imgObj.contains("width") && imgObj.value("width").isDouble() &&
                imgObj.contains("height")&& imgObj.value("height").isDouble()&&
                imgObj.contains("format")&& imgObj.value("format").isDouble() &&
                imgObj.contains("data")  && imgObj.value("data").isArray()){

                int width = imgObj.value("width").toInt();
                int height = imgObj.value("height").toInt();
                int format = imgObj.value("format").toInt();

                QImage img(width, height, (QImage::Format)format);

                QJsonArray data = imgObj.value("data").toArray();

                for (int i = 0; i < height; i++){
                    QRgb* line = (QRgb*)img.scanLine(i);
                    for (int j=0; j < width; j++){
                        QString val = data[i*width+j].toString();
                        QString sr, sg, sb;
                        sr = val.left(2);
                        sg = val.left(4).right(2);
                        sb = val.right(2);
                        int r,g,b;
                        r = sr.toInt(nullptr, 16);
                        g = sg.toInt(nullptr, 16);
                        b = sb.toInt(nullptr, 16);
                        line[j] = qRgb(r,g,b);
                    }
                }
                vec->setFillImage(img);
            }
        }

        vc.push_back(vec);
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
        apply_brush(image, x, y, color, thickness, brush_mask);
    }
    if (sidedness(c.x() - x_offset, y, l1, l2) >= 0){
        apply_brush(image, c.x() - x_offset, y , color, thickness, brush_mask);
    }
    if (sidedness(x, c.y() - y_offset, l1, l2) >= 0){
        apply_brush(image, x, c.y() - y_offset , color, thickness, brush_mask);
    }
    if (sidedness(c.x() - x_offset, c.y() - y_offset, l1, l2) >= 0){
        apply_brush(image, c.x() - x_offset, c.y() - y_offset , color, thickness, brush_mask);
    }
    if (sidedness(c.x() - y_offset, c.y() - x_offset, l1, l2) >= 0){
        apply_brush(image, c.x() - y_offset, c.y() - x_offset, color, thickness, brush_mask);
    }
    if (sidedness(c.x() + y_offset, c.y() - x_offset, l1, l2) >= 0){
        apply_brush(image, c.x() + y_offset, c.y() - x_offset, color, thickness, brush_mask);
    }
    if (sidedness(c.x() - y_offset, c.y() + x_offset, l1, l2) >= 0){
        apply_brush(image, c.x() - y_offset, c.y() + x_offset, color, thickness, brush_mask);
    }
    if (sidedness(c.x() + y_offset, c.y() + x_offset, l1, l2) >= 0) {
        apply_brush(image, c.x() + y_offset, c.y() + x_offset, color, thickness, brush_mask);
    }
}

void VectorRectangle::RenderSelf(QImage* Image, bool antialias) {

    if (this->fill){
        VectorComponent::FillVertexSorting(Image, this, this->fillColor, &this->FillImage);
    }

    if (antialias) {
        MidpointLine::renderLineXiaolinWu(Image, this->points[0], this->phantom_points[0], this->color, thickness, brush_mask);
        MidpointLine::renderLineXiaolinWu(Image, this->points[0], this->phantom_points[1], this->color, thickness, brush_mask);
        MidpointLine::renderLineXiaolinWu(Image, this->points[1], this->phantom_points[0], this->color, thickness, brush_mask);
        MidpointLine::renderLineXiaolinWu(Image, this->points[1], this->phantom_points[1], this->color, thickness, brush_mask);
    }
    else {
        MidpointLine::renderLine_M(Image, this->points[0], this->phantom_points[0], this->color, thickness, brush_mask);
        MidpointLine::renderLine_M(Image, this->points[0], this->phantom_points[1], this->color, thickness, brush_mask);
        MidpointLine::renderLine_M(Image, this->points[1], this->phantom_points[0], this->color, thickness, brush_mask);
        MidpointLine::renderLine_M(Image, this->points[1], this->phantom_points[1], this->color, thickness, brush_mask);
    }
}

bool VectorRectangle::genPhantom() {
    if(!this->IsReady()){
        return false;
    }
    this->phantom_points.clear();
    this->phantom_points.push_back(QPoint(this->points[0].x(), this->points[1].y()));
    this->phantom_points.push_back(QPoint(this->points[1].x(), this->points[0].y()));
    return true;
}

void VectorRectangle::addPoint(QPoint p) {
    if(this->points.size() < 1) {
        VectorComponent::addPoint(p);
        return;
    }
    if (this->points.size() == 1) {
        VectorComponent::addPoint(p);
        if(!this->genPhantom()) {throw std::runtime_error("VectorRectangle::addPoint : this should be unreachable!");}
        return;
    }
}

bool VectorRectangle::IsReady(){
    if(this->points.size() < 2) {
        return false;
    }
    return true;
}

bool VectorRectangle::ForceReady(){
    return this->IsReady();
}

MovedObject* VectorRectangle::CheckClickMove(QPoint clickPos, double tolerance) {
    std::vector<QPoint*> all;
    std::vector<QPoint*> mOX;
    std::vector<QPoint*> mOY;

    //points

    ///Real
    if (VectorComponent::PointDistance(clickPos, points[0]) <= tolerance){
        all.push_back(&points[0]);
        mOX.push_back(&phantom_points[0]);
        mOY.push_back(&phantom_points[1]);
        return new RectangleMoveObject(all, mOX, mOY, clickPos);
    }
    if (VectorComponent::PointDistance(clickPos, points[1]) <= tolerance){
        all.push_back(&points[1]);
        mOX.push_back(&phantom_points[1]);
        mOY.push_back(&phantom_points[0]);
        return new RectangleMoveObject(all, mOX, mOY, clickPos);
    }
    ///Phantom
    if (VectorComponent::PointDistance(clickPos, phantom_points[0]) <= tolerance){
        all.push_back(&phantom_points[0]);
        mOX.push_back(&points[0]);
        mOY.push_back(&points[1]);
        return new RectangleMoveObject(all, mOX, mOY, clickPos);
    }
    if (VectorComponent::PointDistance(clickPos, phantom_points[1]) <= tolerance){
        all.push_back(&phantom_points[1]);
        mOX.push_back(&points[1]);
        mOY.push_back(&points[0]);
        return new RectangleMoveObject(all, mOX, mOY, clickPos);
    }

    //edges

    if (VectorComponent::LineDistance(points[0], phantom_points[0], clickPos) <= tolerance){
        mOX.push_back(&points[0]);
        mOX.push_back(&phantom_points[0]);
        return new RectangleMoveObject(all, mOX, mOY, clickPos);
    }
    if (VectorComponent::LineDistance(points[1], phantom_points[1], clickPos) <= tolerance){
        mOX.push_back(&points[1]);
        mOX.push_back(&phantom_points[1]);
        return new RectangleMoveObject(all, mOX, mOY, clickPos);
    }

    if (VectorComponent::LineDistance(points[0], phantom_points[1], clickPos) <= tolerance){
        mOY.push_back(&points[0]);
        mOY.push_back(&phantom_points[1]);
        return new RectangleMoveObject(all, mOX, mOY, clickPos);
    }
    if (VectorComponent::LineDistance(points[1], phantom_points[0], clickPos) <= tolerance){
        mOY.push_back(&points[1]);
        mOY.push_back(&phantom_points[0]);
        return new RectangleMoveObject(all, mOX, mOY, clickPos);
    }
    return nullptr;
}

MovedObject* VectorRectangle::MoveAll(QPoint clickPos, double tolerance) {
    MovedObject* mobj = this->CheckClickMove(clickPos, tolerance);
    if (mobj == nullptr) {
        return nullptr;
    }
    delete mobj;

    std::vector<QPoint*> all;
    std::vector<QPoint*> mOX;
    std::vector<QPoint*> mOY;

    for (QPoint &p : points){
        all.push_back(&p);
    }
    for (QPoint &p : phantom_points){
        all.push_back(&p);
    }
    return new RectangleMoveObject(all, mOX, mOY, clickPos);
}

void RectangleMoveObject::MoveTo(QPoint newPos){
    QPoint offset = newPos - anchor;
    anchor = newPos;
    for (QPoint *p : pts){
        *p += offset;
    }
    for (QPoint *p : movedOX){
        p->rx() += offset.x();
    }
    for (QPoint *p : movedOY){
        p->ry() += offset.y();
    }
}

std::vector<QPoint> RectangleMoveObject::PointVals(){
    std::vector<QPoint> res;
    for (QPoint *p : pts) {
        res.push_back(QPoint(p->x(), p->y()));
    }
    for (QPoint *p : movedOX) {
        res.push_back(QPoint(p->x(), p->y()));
    }
    for (QPoint *p : movedOY) {
        res.push_back(QPoint(p->x(), p->y()));
    }
    return res;
}

MovedObject* FloodFill::CheckClickMove(QPoint clickPos, double tolreance){
    std::vector<QPoint*> res;
    if (VectorComponent::PointDistance(clickPos, this->points[0]) < tolreance) {
        res.push_back(&this->points[0]);
        return new MovedObject(res, clickPos);
    }
    return nullptr;
}

void FloodFill::RenderSelf(QImage* Image, bool anitalias) {
    std::queue<QPoint> fillQueue;
    QRgb baseColor = Image->pixel(this->points[0]);
    QPoint n;

    fillQueue.push(this->points[0]);
    while (!fillQueue.empty()){
        n = fillQueue.front();
        fillQueue.pop();
        if ( FloodFill::ColorDiff(Image->pixel(n), baseColor) > thickness){
            continue;
        }
        if( Image->pixel(n) == this->color ){
            continue;
        }
        //if ( Image->pixel(n) != baseColor ){
            Image->setPixel(n, this->color);
            if (n.x()-1 >= 0) {fillQueue.push(QPoint(n.x()-1, n.y()));}
            if (n.x()+1 <= Image->width()) {fillQueue.push(QPoint(n.x()+1, n.y()));}
            if (n.y()-1 >= 0) {fillQueue.push(QPoint(n.x(), n.y()-1));}
            if (n.y()+1 <= Image->height() && n.y()+1 <= Image->height() ) {fillQueue.push(QPoint(n.x(), n.y()+1));}

    }

}

