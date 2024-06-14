#ifndef C3DOBJECT_H
#define C3DOBJECT_H

#include <vector>
#include <Qt3DCore/Qt3DCore>

template <class T>
class Array2D {
private:
    std::vector<T> *buffer;
    size_t _w;
    size_t _h;
public:

    size_t Width() {return _w;}
    size_t Height() {return _h;}

    Array2D(size_t w, size_t h, T fill){
        _w = w;
        _h = h;
        buffer = new std::vector<T>(_w * _h, fill);
    }

    Array2D() = delete;

    ~Array2D(){
        delete buffer;
    }
    //std::map<int, int> m;

    bool in(size_t w, size_t h) const {
        if ( w >= 0 && w < _w && h >= 0 && h < _h ){
            return true;
        }
        return false;
    }

    T& at(size_t w, size_t h){
        return (*buffer)[h*_w + w];
    }

};

typedef Array2D<double> ZBuffer;

class vert {
public:
    vert(QVector4D vertex, QVector4D normal) {
        this->v = vertex; this->n = normal;
    }
    QVector4D v;
    QVector4D n;

    void affine_normalize() {
        v = v.w() == 0 ? v : v/v.w();
        n = n.w() == 0 ? n : n/n.w();
    }

    static vert matrixMult(QMatrix4x4 m, vert v){
        return vert(m * v.v, m*v.n);
    }
};

static QVector4D matrixMult(QMatrix4x4 m, QVector4D v){
    return m * v;
}

typedef std::vector<vert> triag;
typedef std::vector<triag> polytope;

typedef QVector3D vec3;

class C3DCameraController;
class C3DScene;
class C3DObject;
//std::vector<std::vector<int> > C3DObject::genBrush(int thickness);

namespace TriagFill {
    typedef struct AETEntry {
        int y_max;
        double x;
        double inv_m;
        double z;
        double m_zdiff;
        //double dz;
    } AETE;

    class CPoint3D {
    private:
        int xp;
        int yp;
        double zf;
    public:
        constexpr CPoint3D(int x, int y, double z) : xp(x), yp(y), zf(z) {}
        CPoint3D() {}
        Q_DECL_CONSTEXPR inline int x() const {return xp;}
        Q_DECL_CONSTEXPR inline int y() const {return yp;}
        Q_DECL_CONSTEXPR inline double z() const {return zf;}

        Q_DECL_CONSTEXPR static inline CPoint3D fromQVector4D(QVector4D vec){
            return CPoint3D(qRound(vec.x()), qRound(vec.y()), vec.z());
        }
    };

    void RenderScanLine(QImage *Image, std::vector<AETE> *AET, int y, QRgb col, ZBuffer *zbuf, QImage* fillImage);
    void FillVertexSorting(QImage *Image, triag *toFill, QRgb fillColor, ZBuffer *zbuf, QImage* fillImage = nullptr);
}

class C3DScene : public QObject
{
private:

    std::vector<C3DObject*> objects;
    int Sw, Sh;
    static constexpr double fov = M_PI/3;
    static constexpr int thickness = 5;

    std::vector<std::vector<int> > (*brushGenerator)(int n);
    void (*lineRenderer)(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask);

    QMatrix4x4 perspProjMatrix;

    friend C3DObject;

public:
    C3DCameraController *mainCam;

    C3DScene(int width, int height, void (*newLineRenderer)(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask),
             std::vector<std::vector<int> > (*newBrushGenerator)(int n));
    ~C3DScene();

    static QMatrix4x4 IdMatrix();
    static QMatrix4x4 PerspProj(double Sx, double Sy, double fov);
    static QMatrix4x4 CamViewMatrix(QVector3D camPos, QVector3D objPos, QVector3D upVec);


    QMatrix4x4 CamViewMatrix(QVector3D targetPos, QVector3D upVec = QVector3D(0,-1,0));

    QMatrix4x4 GetPerspProjection() {return perspProjMatrix;}

    void Resize(QRectF newSize) {
        this->Sw = newSize.width();
        this->Sh = newSize.height();
        this->perspProjMatrix = PerspProj(this->Sw, this->Sh, this->fov);
    }

    void SetBrushGenerator(std::vector<std::vector<int> > (*newBrushGenerator)(int n)){brushGenerator = newBrushGenerator;}
    void SetLineRenderer(void (*newLineRenderer)(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask)){lineRenderer = newLineRenderer;}

    void SetRenderingFunctions(void (*newLineRenderer)(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask),
                                      std::vector<std::vector<int> > (*newBrushGenerator)(int n)){
        C3DScene::SetLineRenderer(newLineRenderer);
        C3DScene::SetBrushGenerator(newBrushGenerator);
    }

    void SetCamera(C3DCameraController *cam) {mainCam = cam;}
    void AddObject(C3DObject *newObj){
        objects.push_back(newObj);
    }

    void RenderOnto(QImage *Image);

};

class C3DCameraController {
private:
    static constexpr double eventAngleOffset = M_PI/90;

    QMatrix4x4 rot = C3DScene::IdMatrix();

    C3DScene *parent;

    QVector3D upVec = QVector3D(0,1,0);

    QVector3D baseCamPos = QVector3D(12, 0, 0);
    QVector3D camPos = baseCamPos; //QVector3D(-5, 1, 0);

    double theta = 0.0l;
    double alpha = 0.0l;

    void init_self();

public:

    C3DCameraController(C3DScene *parent);
    C3DCameraController(C3DScene *parent, QVector3D initPos);

    void SetPos(QVector3D newPos) {camPos = newPos;};
    void OffsetPos(QVector3D offset) {camPos += offset;};
    QVector3D GetPos() {return camPos;}

    void KeyEventHandler(QKeyEvent *event);

    void OrbitTickX(double dt);
    void OrbitTickY(double dt);
    void OrbitTickZ(double dt);
    //QVector3D UpVecAgainst(QVector3D target);
};

class C3DObject
{
protected:

    C3DScene *parent;
    polytope mesh;
    QMatrix4x4 rot = C3DScene::IdMatrix();

    std::vector<std::vector<int> > bm;

    vec3 offset = vec3(0,0,0);

    std::vector<QRgb> triagColors;

    std::vector<std::vector<int> > genBrush(int thickness){
        return parent->brushGenerator(thickness);
    }
    void renderLine(QImage* image, QPoint p1, QPoint p2, QRgb c, int thickness, std::vector<std::vector<int> > brush_mask){
        parent->lineRenderer(image, p1, p2, c, thickness, brush_mask);
    }

    //friend C3DScene;

public:
    C3DObject() = delete;
    C3DObject(C3DScene *parent) {
        this->parent = parent;
        bm = genBrush(parent->thickness);
    }
    virtual ~C3DObject() {};

    static QMatrix4x4 RotationX(double alpha);
    static QMatrix4x4 RotationY(double alpha);
    static QMatrix4x4 RotationZ(double alpha);
    static QMatrix4x4 Translation(QVector3D t);

    virtual void ResetOffset() {this->offset = vec3(0, 0, 0);}
    virtual void ResetRotation() {this->rot = C3DScene::IdMatrix();}

    virtual void OffsetSelf(QVector3D off){ this->offset += off; }
    virtual void RotateSelfX(double alpha){ this->rot = this->rot * RotationX(alpha); }
    virtual void RotateSelfY(double alpha){ this->rot = this->rot * RotationY(alpha); }
    virtual void RotateSelfZ(double alpha){ this->rot = this->rot * RotationZ(alpha); }

    virtual void RenderSelf(QImage *Image, ZBuffer *zbuf);

    virtual void SetTriagColors(std::vector<QRgb> cols);

    //C3DObject();
};

class CuboidMesh : public C3DObject{
protected:

    static const constexpr QVector3D default_cuboid = QVector3D(1,1,1);
    static polytope GenCuboidMeshCentered(double h, double w, double d);

public:
    CuboidMesh(C3DScene *parent) : C3DObject(parent) {
        mesh = GenCuboidMeshCentered(1,1,1);
    }
    //static polytope GenCuboidMesh(double h, double w, double d);

    CuboidMesh(C3DScene *parent, QVector3D size) : C3DObject(parent) {
        mesh = GenCuboidMeshCentered(size.x(), size.y(), size.z());
    }

    CuboidMesh(C3DScene *parent, double w, double h, double d) : C3DObject(parent) {
        mesh = GenCuboidMeshCentered(w, h, d);
    }


};



#endif // C3DOBJECT_H
