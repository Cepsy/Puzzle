#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QMainWindow>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <vector>
#include <string>
#include <QVector3D>
using namespace std;

namespace Ui {
class MainWindow;
}

using namespace OpenMesh;
using namespace OpenMesh::Attributes;

struct MyTraits : public OpenMesh::DefaultTraits
{
    // use vertex normals and vertex colors
    VertexAttributes( OpenMesh::Attributes::Normal | OpenMesh::Attributes::Color );
    // store the previous halfedge
    HalfedgeAttributes( OpenMesh::Attributes::PrevHalfedge );
    // use face normals face colors
    FaceAttributes( OpenMesh::Attributes::Normal | OpenMesh::Attributes::Color );
    EdgeAttributes( OpenMesh::Attributes::Color );
    // vertex thickness
    VertexTraits{float thickness; float value; Color faceShadingColor; int label=0;};
    // edge thickness
    EdgeTraits{float thickness; int label=0;};

    FaceTraits{int label=0;};
};
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> MyMesh;

struct CoorPieces{
    double xBG, zBG, xBD, zBD, xHG, zHG, xHD, zHD, L;
    double minY, maxY;
};

struct Plan{
    string orientation;
    double coor;
    double minY, maxY;
};

struct PuzzleState{
    int HG, H, HD;
    int MG, M, MD;
    int BG, B, BD;

    bool checkingH;
    bool checkingG;
    bool checkingD;
    bool checkingB;
};

enum DisplayMode {Normal, TemperatureMap, ColorShading};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    void displayMesh(MyMesh *_mesh, DisplayMode mode = DisplayMode::Normal);
    void resetAllColorsAndThickness(MyMesh* _mesh);
    void resetFacesLabels(MyMesh *_mesh);
    void resetVertexLabels(MyMesh *_mesh);

    void connexCompoFlagging(MyMesh *_mesh, int currentLabel, int currentFace);
    void piecesColoration(MyMesh *_mesh);

    bool isCutByPlan(MyMesh *_mesh, int faceID, Plan plan);
    bool edgeIsCutByPlan(MyMesh *_mesh, int edgeID, Plan plan);
    QVector3D getCenterOfFace(MyMesh *_mesh, int faceID);


    void displayObjectsValues();
    void generateCenterCoordinates();
    float scalar_product(QVector3D a, QVector3D b);
    QVector3D intersection_calculation(QVector3D plane_point, QVector3D plane_normal, QVector3D a, QVector3D b);

    vector<QVector3D> calcTrajectoryCoordinates(QVector3D M, QVector3D O, string orientation);
    void trajectoryColorationUpdate(MyMesh *_mesh);
    void resetTrajectoryChecking();
    void addTrajectory(MyMesh *_mesh);
    void removeTrajectory(MyMesh *_mesh);

    void correctLabelisation(MyMesh *_mesh);

    void applySpacing(MyMesh *_mesh);

    void generateIntersectVertexFaces(MyMesh* _mesh, int faceID, Plan p);
    void generateMeshCompletion(MyMesh *_mesh);

private slots:

    void on_pushButton_chargement_clicked();
    void on_pushButton_beginSelection_clicked();
    void on_pushButton_LMoins_clicked();
    void on_pushButton_LPlus_clicked();
    void on_pushButton_xMoins_clicked();
    void on_pushButton_xPlus_clicked();
    void on_pushButton_zMoins_clicked();
    void on_pushButton_zPlus_clicked();
    void on_pushButton_yMinMoins_clicked();
    void on_pushButton_yMinPlus_clicked();
    void on_pushButton_yMaxMoins_clicked();
    void on_pushButton_yMaxPlus_clicked();
    void on_pushButton_decoupage_clicked();
    void on_pushButton_espMoins_clicked();
    void on_pushButton_espPlus_clicked();
    void on_pushButton_RH_clicked();
    void on_pushButton_RB_clicked();
    void on_pushButton_RG_clicked();
    void on_pushButton_RD_clicked();


private:

    MyMesh mesh;

    CoorPieces corps;

    PuzzleState puzzle;
    vector<VertexHandle> trajectoryBuffer;

    vector<int> faceLabeling;
    Plan p_droite;
    Plan p_bas;
    Plan p_gauche;
    Plan p_haut;
    vector<MyMesh::Color> colors;

    float center_X;
    float center_Y;
    float center_Z;

    double PAS;
    double ESP;
    bool isCut;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
