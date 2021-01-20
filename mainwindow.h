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
    int  G, M,  D;
    int BG, B, BD;

    /* positionnement de labels initial
     *
     * 2, 3, 4,
     * 9, 1, 5,
     * 8, 7, 6,
     *
     * */

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

    void displayObjectsValues();

    //decoupage
    bool isCutByPlan(MyMesh *_mesh, int faceID, Plan plan);
    void generateCenterCoordinates();
    float scalar_product(QVector3D a, QVector3D b);
    QVector3D intersection_calculation(QVector3D plane_point, QVector3D plane_normal, QVector3D a, QVector3D b);
    bool edgeIsCutByPlan(MyMesh *_mesh, int edgeID, Plan plan);

    //labelisation & coloration
    QVector3D getCenterOfFace(MyMesh *_mesh, int faceID);
    void connexCompoFlagging(MyMesh *_mesh, int currentLabel, int currentFace);
    void piecesFlaggingAndColoration(MyMesh *_mesh);
    void resetFacesLabels(MyMesh *_mesh);
    void resetVertexLabels(MyMesh *_mesh);
    void correctLabelisation(MyMesh *_mesh);
    void registerFacesAndVertices(MyMesh *_mesh);

    //trajectories calculations
    vector<QVector3D> calcTrajectoryCoordinates(QVector3D M, QVector3D O, string orientation);
    void trajectoryColorationUpdate(MyMesh *_mesh);
    void resetTrajectoryChecking();
    void addTrajectory(MyMesh *_mesh);
    void removeTrajectory(MyMesh *_mesh);

    //collisions checking
    float getDistanceBtw2Vertices(MyMesh *_mesh, QVector3D a, QVector3D b);
    void displayTrajectoryOnPlane(MyMesh *_mesh); //or not
    bool pointIsInTrianglev1(MyMesh *_mesh, QVector3D p, FaceHandle fh);
    bool pointIsInTrianglev2(MyMesh *_mesh, QVector3D p, FaceHandle fh);
    void checkingTrajectoryCollisions(MyMesh *_mesh);

    //prototype reduction
    QVector3D getPieceCenter(MyMesh *_mesh, int piece_label);
    void reducePiece(MyMesh *_mesh, int piece_label);

    //squellette
    void displayMesh(MyMesh *_mesh, DisplayMode mode = DisplayMode::Normal);
    void resetAllColorsAndThickness(MyMesh* _mesh);

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
    void on_pushButton_reduction_clicked();


private:

    MyMesh mesh;

    CoorPieces corps;

    PuzzleState puzzle;

    vector<vector<VertexHandle>> piecesVertices;
    vector<vector<FaceHandle>> piecesFaces;

    vector<VertexHandle> trajectoryBuffer1;
    vector<VertexHandle> trajectoryBuffer2;
    vector<VertexHandle> trajectoryBuffer3;

    /* vertex labels when they are part of
     * a piece trajectory :
     *
     * 12, 13, 14
     * 19,  x, 15
     * 18, 17, 16
     *
     * otherwise 0
     * */


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
