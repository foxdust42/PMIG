#include "c3dobject.h"

//C3DObject::C3DObject() {}

// QVector3D C3DCameraController::UpVecAgainst(QVector3D target){
//     QVector3D offset = QVector3D(target.x() - this->camPos.x(),
//                                  target.y() - this->camPos.y(),
//                                  target.z() - this->camPos.z());

//     return QVector3D();
// }

C3DScene::C3DScene(int width, int height, void (*newLineRenderer)(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask),
         std::vector<std::vector<int> > (*newBrushGenerator)(int n)){
    Sw = width;
    Sh = height;
    SetRenderingFunctions(newLineRenderer, newBrushGenerator);
    perspProjMatrix = PerspProj(Sw, Sh, fov);
    this->mainCam = new C3DCameraController(this);
}

C3DScene::~C3DScene(){
    delete this->mainCam;
    for (C3DObject *obj : objects) {
        delete obj;
    }
}

void C3DScene::RenderOnto(QImage *Image){
    /* TODO:: Z-buffer algorithm */

    ZBuffer zbuf = ZBuffer(Image->width(), Image->height(), std::numeric_limits<double>::min());

    //zbuf.at(2,3) = 0.2;

    for ( C3DObject* obj : this->objects ){
        obj->RenderSelf(Image, &zbuf);
    }
}

QMatrix4x4 C3DScene::CamViewMatrix(QVector3D objPos, QVector3D upVec){
    return C3DScene::CamViewMatrix(this->mainCam->GetPos(), objPos, upVec);
}

QMatrix4x4 C3DScene::CamViewMatrix(QVector3D camPos, QVector3D objPos, QVector3D upVec){
    QVector3D cZ = (camPos - objPos).normalized();///(camPos - objPos).length();
    QVector3D cX = QVector3D::crossProduct(upVec, cZ).normalized();///QVector3D::crossProduct(upVec, cZ).length();
    QVector3D cY = QVector3D::crossProduct(cZ, cX).normalized();///QVector3D::crossProduct(cZ, cX).length();

    return QMatrix4x4(  cX.x(), cX.y(), cX.z(), QVector3D::dotProduct(cX, camPos),
                      cY.x(), cY.y(), cY.z(), QVector3D::dotProduct(cX, camPos),
                      cZ.x(), cZ.y(), cZ.z(), QVector3D::dotProduct(cZ, camPos),
                      0,0,0,1);
}

QMatrix4x4 C3DScene::PerspProj(double Sx, double Sy, double fov) {
    double cot = std::cos(fov/2) / std::sin(fov/2);
    return QMatrix4x4( (-Sx/2)*cot, 0, (Sx/2), 0,
                      0, (Sx/2)*cot, (Sy/2) ,0,
                      0,0,0,1,
                      0,0,1,0 );
}

QMatrix4x4 C3DScene::IdMatrix() {
    return QMatrix4x4 (
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        );
}

C3DCameraController::C3DCameraController(C3DScene *parent){
    this->parent = parent;
}

C3DCameraController::C3DCameraController(C3DScene *parent, QVector3D initPos){
    this->parent = parent;
    this->camPos = initPos;
}


void C3DCameraController::KeyEventHandler(QKeyEvent *event){
    switch (event->key()){
    case Qt::Key_Left:
        this->OrbitTickY(-eventAngleOffset);
        break;
    case Qt::Key_Right:
        this->OrbitTickY(eventAngleOffset);
        break;
    case Qt::Key_Up:
        this->OrbitTickZ(eventAngleOffset);
        break;
    case Qt::Key_Down:
        this->OrbitTickZ(-eventAngleOffset);
        break;
    default:
        break;
    }

}

void C3DCameraController::OrbitTickX(double dt){
    alpha += dt;

    alpha = std::fmod(alpha, M_PI*2);

    rot = C3DObject::RotationX(alpha);

    camPos = (rot * QVector4D(baseCamPos, 0)).toVector3D();

    // camPos = QVector3D(baseCamPos.x(), baseCamPos.y(), a);
}

void C3DCameraController::OrbitTickY(double dt){
    alpha += dt;

    alpha = std::fmod(alpha, M_PI*2);

    rot = C3DObject::RotationZ(theta);
    rot *= C3DObject::RotationY(alpha);

    camPos = (rot * QVector4D(baseCamPos, 0)).toVector3D();

    // camPos = QVector3D(baseCamPos.x(), baseCamPos.y(), a);
}

void C3DCameraController::OrbitTickZ(double dt){
    theta += dt;

    theta = std::fmod(theta, M_PI*2);

    rot = C3DObject::RotationZ(theta);
    rot *= C3DObject::RotationY(alpha);

    camPos = (rot * QVector4D(baseCamPos, 0)).toVector3D();

    // camPos = QVector3D(baseCamPos.x(), baseCamPos.y(), a);
}

QMatrix4x4 C3DObject::RotationX(double alpha){
    QMatrix4x4 rot = QMatrix4x4 (
        1,0,0,0,
        0, std::cos(alpha), -std::sin(alpha), 0,
        0, std::sin(alpha), std::cos(alpha), 0,
        0,0,0,1
        );
    return rot;
}
QMatrix4x4 C3DObject::RotationY(double alpha){
    QMatrix4x4 rot = QMatrix4x4 (
        std::cos(alpha), 0,  std::sin(alpha), 0,
        0,1,0,0,
        -std::sin(alpha), 0, std::cos(alpha), 0,
        0,0,0,1
        );
    return rot;
}
QMatrix4x4 C3DObject::RotationZ(double alpha){
    QMatrix4x4 rot = QMatrix4x4 (
        std::cos(alpha), -std::sin(alpha), 0, 0,
        std::sin(alpha), std::cos(alpha), 0, 0,
        0,0,1,0,
        0,0,0,1
        );
    return rot;
}

QMatrix4x4 C3DObject::Translation(QVector3D t){
    QMatrix4x4 tr = QMatrix4x4 (
        1, 0, 0, t.x(),
        0, 1, 0, t.y(),
        0, 0, 1, t.z(),
        0, 0, 0, 1
        );
    return tr;
}


void C3DObject::RenderSelf(QImage* Image, ZBuffer *zbuf) {

    // triag tri_new;

    //renderLine(Image, QPoint(0,0), QPoint(255, 255), qRgb(0,255,0), 3, this->bm);
    int i = 0;

    for (int i=0; i < mesh.size(); i++) { //(triag &t : mesh){
        triag n;
        for (vert p : mesh[i]){
            vert v = p;

            v = vert::matrixMult(rot , v);

            //v = vert::matrixMult(RotationZ(M_PI/8), v);

            v = vert::matrixMult(Translation(offset) , v);

            // v = (Translation(QVector3D(-1,-1,1)) * v);
            v = vert::matrixMult(parent->CamViewMatrix(QVector3D(0,0,0)), v);
            v = vert::matrixMult(parent->GetPerspProjection(), v);
            v.affine_normalize(); //v.w() == 0 ? v : v/(v.w());
            n.push_back(v);
        }

        if ( QVector3D::crossProduct(
                QVector3D( n[1].v.x() - n[0].v.x(), n[1].v.y() - n[0].v.y(), 0 ),
                QVector3D( n[2].v.x() - n[0].v.x(), n[2].v.y() - n[0].v.y() ,0 )
                ).z() <= 0){
            continue;
        }

        QRgb c = this->triagColors.size() == 0 ? qRgb(255,0,0) : this->triagColors[i % this->triagColors.size()];

        TriagFill::FillVertexSorting(Image, &n, c, zbuf, nullptr);
        renderLine(Image, n[0].v.toPoint() , n[1].v.toPoint()  , qRgb(255,0,0), 3, this->bm );
        renderLine(Image, n[1].v.toPoint() , n[2].v.toPoint()  , qRgb(255,0,0), 3, this->bm );
        renderLine(Image, n[2].v.toPoint() , n[0].v.toPoint()  , qRgb(255,0,0), 3, this->bm );
    }
}

void C3DObject::SetTriagColors(std::vector<QRgb> cols){
    this->triagColors = cols;
}


namespace TriagFill {
    void RenderScanLine(QImage *Image, std::vector<AETE> *AET, int y, QRgb col, ZBuffer *zbuf, QImage* fillImage){
        int x, next_x;
        double z, next_z;
        double z_diff;
        if (fillImage != nullptr){
            goto img_fill;
        }
        for (int i=0; i+1<AET->size(); i+=2 ){
            x = (*AET)[i].x;
            next_x = (*AET)[i+1].x;

            z = (*AET)[i].z;
            next_z = (*AET)[i+1].z;

            z_diff = std::max(std::numeric_limits<double>::epsilon(), std::abs(next_z - z));
            if (next_z > z){
                z_diff *= -1;
            }

            z_diff /= (double)(next_x-x);

            //z_diff =( (next_z - z) / ((double)(next_x - x)) );


            for (int j = x; j < next_x; j++){
                if (zbuf->in(j,y) && z >= zbuf->at(j, y) && z != 0){
                    Image->setPixel(QPoint(j, y), col);
                    zbuf->at(j, y) = z;
                }
                z += z_diff;
            }
        }
        return;
    img_fill:
        for (int i=0; i+1<AET->size(); i+=2 ){
            x = (*AET)[i].x;
            next_x = (*AET)[i+1].x;

            for (int j = x; j < next_x; j++){
                Image->setPixel(QPoint(j, y), fillImage->pixel(j % fillImage->width(), y % fillImage->height()));
            }
        }
        return;
    }

    void FillVertexSorting(QImage *Image, triag *toFill, QRgb fillColor, ZBuffer *zbuf, QImage* fillImage) {
        if (toFill->size() < 3){
            return;
        }

        std::vector<CPoint3D> pts;

        //QPoint3D s;

        for (vert v : *toFill){
            pts.push_back( CPoint3D::fromQVector4D(v.v));
        }

        pts.push_back(CPoint3D::fromQVector4D((*toFill)[0].v));

        AETE (*addAETEntry)(CPoint3D p1, CPoint3D p2) = [](CPoint3D p1, CPoint3D p2) -> AETE{
            CPoint3D start, end;
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
            double dz = end.z() - start.z();
            double m_inv = ((double)dx)/((double)dy);
            double m_zdiff = (dz)/((double)dy);
            return AETE{end.y(), (double)start.x(), m_inv, start.z(), m_zdiff};
        };
        std::vector<AETE> AET;

        std::vector<int> indicies = std::vector<int>(pts.size()-1);
        std::iota(indicies.begin(), indicies.end(), 0);

        std::sort(indicies.begin(), indicies.end(), [&pts](int a, int b) -> bool {
            if(pts[a].y() < pts[b].y()){
                return true;
            }
            return false;
        });

        int k=0;
        int i = indicies[k];
        int y = pts[indicies[0]].y();
        //int y_min = y;
        int y_max = pts[indicies[indicies.size()-1]].y();

        while (y < y_max) {
            while (pts[i].y() == y){
                int tmp = (i-1) < 0 ? pts.size()-2 : i-1;
                if (pts[tmp].y() > pts[i].y()) {
                    AET.push_back(addAETEntry(pts[i], pts[tmp]));
                }
                if (pts[i+1].y() > pts[i].y()) {
                    AET.push_back(addAETEntry(pts[i], pts[(i+1)%(pts.size()-1)]));
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

            RenderScanLine(Image, &AET, y, fillColor, zbuf, fillImage);
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

}

// polytope CuboidMesh::GenCuboidMesh(double h, double w, double d){
//     polytope res;
//     //face 1 & -1
//     {
//         QVector4D p1, p2, p3, p4, n;
//         p1 = QVector4D(0,0,d,1);
//         p2 = QVector4D(w,0,d,1);
//         p3 = QVector4D(w,h,d,1);
//         p4 = QVector4D(0,h,d,1);

//         n = QVector4D(0,0,1,0);

//         triag t1, t2;
//         t1.push_back(vert(p1,n));
//         t1.push_back(vert(p2,n));
//         t1.push_back(vert(p3,n));

//         t2.push_back(vert(p1,n));
//         t2.push_back(vert(p3,n));
//         t2.push_back(vert(p4,n));

//         res.push_back(t1);
//         res.push_back(t2);
//     }
//     {
//         QVector4D p1, p2, p3, p4;
//         p1 = QVector4D(0,0,0,1);
//         p2 = QVector4D(w,0,0,1);
//         p3 = QVector4D(w,h,0,1);
//         p4 = QVector4D(0,h,0,1);

//         triag t1, t2;
//         t1.push_back(p1);
//         t1.push_back(p2);
//         t1.push_back(p3);

//         t2.push_back(p1);
//         t2.push_back(p3);
//         t2.push_back(p4);

//         res.push_back(t1);
//         res.push_back(t2);
//     }
//     {
//         QVector4D p1, p2, p3, p4;
//         p1 = QVector4D(w,0,0,1);
//         p2 = QVector4D(w,h,0,1);
//         p3 = QVector4D(w,h,d,1);
//         p4 = QVector4D(w,0,d,1);

//         triag t1, t2;
//         t1.push_back(p1);
//         t1.push_back(p2);
//         t1.push_back(p3);

//         t2.push_back(p1);
//         t2.push_back(p3);
//         t2.push_back(p4);

//         res.push_back(t1);
//         res.push_back(t2);
//     }
//     {
//         QVector4D p1, p2, p3, p4;
//         p1 = QVector4D(0,0,0,1);
//         p2 = QVector4D(0,h,0,1);
//         p3 = QVector4D(0,h,d,1);
//         p4 = QVector4D(0,0,d,1);

//         triag t1, t2;
//         t1.push_back(p1);
//         t1.push_back(p2);
//         t1.push_back(p3);

//         t2.push_back(p1);
//         t2.push_back(p3);
//         t2.push_back(p4);

//         res.push_back(t1);
//         res.push_back(t2);
//     }
//     {
//         QVector4D p1, p2, p3, p4;
//         p1 = QVector4D(0,h,0,1);
//         p2 = QVector4D(w,h,0,1);
//         p3 = QVector4D(w,h,d,1);
//         p4 = QVector4D(0,h,d,1);

//         triag t1, t2;
//         t1.push_back(p1);
//         t1.push_back(p2);
//         t1.push_back(p3);

//         t2.push_back(p1);
//         t2.push_back(p3);
//         t2.push_back(p4);

//         res.push_back(t1);
//         res.push_back(t2);
//     }
//     {
//         QVector4D p1, p2, p3, p4;
//         p1 = QVector4D(0,0,0,1);
//         p2 = QVector4D(w,0,0,1);
//         p3 = QVector4D(w,0,d,1);
//         p4 = QVector4D(0,0,d,1);

//         triag t1, t2;
//         t1.push_back(p1);
//         t1.push_back(p2);
//         t1.push_back(p3);

//         t2.push_back(p1);
//         t2.push_back(p3);
//         t2.push_back(p4);

//         res.push_back(t1);
//         res.push_back(t2);
//     }
//     return res;
// }

polytope CuboidMesh::GenCuboidMeshCentered(double h, double w, double d){
    polytope res;
    //face 1 & -1
    h /= 2;
    w /= 2;
    d /= 2;
    {
        QVector4D p1, p2, p3, p4, n;
        p1 = QVector4D(-w,-h, d,1);
        p2 = QVector4D( w,-h, d,1);
        p3 = QVector4D( w, h, d,1);
        p4 = QVector4D(-w, h, d,1);

        n = QVector4D(0,0,1,0);

        triag t1, t2;
        t1.push_back(vert(p1,n));
        t1.push_back(vert(p2,n));
        t1.push_back(vert(p3,n));

        t2.push_back(vert(p1,n));
        t2.push_back(vert(p3,n));
        t2.push_back(vert(p4,n));

        res.push_back(t1);
        res.push_back(t2);
    }
    {
        QVector4D p1, p2, p3, p4, n;
        p1 = QVector4D(-w,-h,-d,1);
        p2 = QVector4D( w,-h,-d,1);
        p3 = QVector4D( w, h,-d,1);
        p4 = QVector4D(-w, h,-d,1);

        n = QVector4D(0,0,-1,0);

        triag t1, t2;
        t1.push_back(vert(p3,n));
        t1.push_back(vert(p2,n));
        t1.push_back(vert(p1,n));

        t2.push_back(vert(p4,n));
        t2.push_back(vert(p3,n));
        t2.push_back(vert(p1,n));

        res.push_back(t1);
        res.push_back(t2);
    }
    {
        QVector4D p1, p2, p3, p4, n;
        p1 = QVector4D(w,-h,-d,1);
        p2 = QVector4D(w, h,-d,1);
        p3 = QVector4D(w, h, d,1);
        p4 = QVector4D(w,-h, d,1);

        n = QVector4D(1,0,0,0);

        triag t1, t2;
        t1.push_back(vert(p1,n));
        t1.push_back(vert(p2,n));
        t1.push_back(vert(p3,n));

        t2.push_back(vert(p1,n));
        t2.push_back(vert(p3,n));
        t2.push_back(vert(p4,n));

        res.push_back(t1);
        res.push_back(t2);
    }
    {
        QVector4D p1, p2, p3, p4, n;
        p1 = QVector4D(-w,-h,-d,1);
        p2 = QVector4D(-w, h,-d,1);
        p3 = QVector4D(-w, h, d,1);
        p4 = QVector4D(-w,-h, d,1);

        n = QVector4D(-1,0,0,0);

        triag t1, t2;
        t1.push_back(vert(p3,n));
        t1.push_back(vert(p2,n));
        t1.push_back(vert(p1,n));

        t2.push_back(vert(p4,n));
        t2.push_back(vert(p3,n));
        t2.push_back(vert(p1,n));

        res.push_back(t1);
        res.push_back(t2);
    }
    {
        QVector4D p1, p2, p3, p4, n;
        p1 = QVector4D(-w, h,-d,1);
        p2 = QVector4D( w, h,-d,1);
        p3 = QVector4D( w, h, d,1);
        p4 = QVector4D(-w, h, d,1);

        n = QVector4D(0,1,0,0);

        triag t1, t2;
        t1.push_back(vert(p3,n));
        t1.push_back(vert(p2,n));
        t1.push_back(vert(p1,n));

        t2.push_back(vert(p4,n));
        t2.push_back(vert(p3,n));
        t2.push_back(vert(p1,n));

        res.push_back(t1);
        res.push_back(t2);
    }
    {
        QVector4D p1, p2, p3, p4, n;
        p1 = QVector4D(-w,-h,-d,1);
        p2 = QVector4D( w,-h,-d,1);
        p3 = QVector4D( w,-h, d,1);
        p4 = QVector4D(-w,-h, d,1);

        n = QVector4D(0,-1,0,0);

        triag t1, t2;
        t1.push_back(vert(p1,n));
        t1.push_back(vert(p2,n));
        t1.push_back(vert(p3,n));

        t2.push_back(vert(p1,n));
        t2.push_back(vert(p3,n));
        t2.push_back(vert(p4,n));

        res.push_back(t1);
        res.push_back(t2);
    }
    return res;
}
