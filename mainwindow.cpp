#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <math.h>

void MainWindow::on_pushButton_chargement_clicked(){
    // fenêtre de sélection des fichiers
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Mesh"), "", tr("Mesh Files (*.obj)"));

    // chargement du fichier .obj dans la variable globale "mesh"
    OpenMesh::IO::read_mesh(mesh, fileName.toUtf8().constData());

    // initialisation des couleurs et épaisseurs (sommets et arêtes) du mesh
    resetAllColorsAndThickness(&mesh);

    // on affiche le maillage
    displayMesh(&mesh);

    //init du combobox
    ui->comboBox_plans->addItem("Corps");
    ui->comboBox_plans->addItem("Droite");
    ui->comboBox_plans->addItem("Bas");
    ui->comboBox_plans->addItem("Gauche");
    ui->comboBox_plans->addItem("Haut");

}

//grace à la récuperation de certaines valeurs de notre maillage, on va proposer un premier corps
//sujet ensuite à modification de l'utilisateur
void MainWindow::on_pushButton_beginSelection_clicked(){

    // initialisation de valeurs globales
    ESP = 0;
    faceLabeling.clear();
    for(int i=0 ; i<(int)mesh.n_faces() ; i++) faceLabeling.push_back(-1);

    srand(time(NULL));
    colors.clear();
    for(int i=0 ; i<20 ; i++) colors.push_back(MyMesh::Color(rand() % 255, rand() % 255, rand() % 255));

    isCut = false;

    cout << "starting first body generation ..." << endl;

    MyMesh::VertexIter curVert = mesh.vertices_begin();
    VertexHandle first = *curVert;
    MyMesh::Point Pf = mesh.point(first);
    double max_X = Pf[0];
    double min_X = Pf[0];
    double max_Z = Pf[2];
    double min_Z = Pf[2];
    double max_Y = Pf[1];
    double min_Y = Pf[1];
    ++curVert;

    for( curVert ; curVert != mesh.vertices_end() ; ++curVert){
        VertexHandle vh = *curVert;
        MyMesh::Point P = mesh.point(vh);
        if(P[0] < min_X) min_X = P[0];
        if(P[0] > max_X) max_X = P[0];
        if(P[2] < min_Z) min_Z = P[2];
        if(P[2] > max_Z) max_Z = P[2];
        if(P[1] < min_Y) min_Y = P[1];
        if(P[1] > max_Y) max_Y = P[1];
    }

    double x_length;
    double z_length;
    x_length = max_X - min_X;
    z_length = max_Z - min_Z;

    if( x_length < z_length ) PAS = z_length/100;
    else PAS = x_length/100;

    if( x_length < z_length ) corps.L = x_length/3;
    else corps.L = z_length/3;

    double ecart_x = (x_length - corps.L)/2;
    double ecart_z = (z_length - corps.L)/2;

    cout << "X : " << min_X << " -> " << max_X << " | Z : " << min_Z << " -> " << max_Z << endl;
    cout << "x_lenght : " << x_length << " z_length : " << z_length << " corps.L = " << corps.L << endl;
    cout << "ecart_x = " << ecart_x << " ecart_z = " << ecart_z << endl;

    // concept du calcul :
    // ---------- minX ecart_X corps.xGauche corps.L corps.xDroit ecart_x max_x ---------

    corps.xBG = min_X + ecart_x;
    corps.xHG = min_X + ecart_x;
    corps.xBD = min_X + ecart_x + corps.L;
    corps.xHD = min_X + ecart_x + corps.L;
    corps.zHG = min_Z + ecart_z;
    corps.zHD = min_Z + ecart_z;
    corps.zBG = min_Z + ecart_z + corps.L;
    corps.zBD = min_Z + ecart_z + corps.L;
    corps.minY = min_Y;
    corps.maxY = max_Y;

    p_droite.orientation = "vertical";
    p_droite.coor = corps.xHD;
    p_droite.minY = min_Y;
    p_droite.maxY = max_Y;

    p_bas.orientation = "horizontal";
    p_bas.coor = corps.zBD;
    p_bas.minY = min_Y;
    p_bas.maxY = max_Y;

    p_gauche.orientation = "vertical";
    p_gauche.coor = corps.xBG;
    p_gauche.minY = min_Y;
    p_gauche.maxY = max_Y;

    p_haut.orientation = "horizontal";
    p_haut.coor = corps.zHD;
    p_haut.minY = min_Y;
    p_haut.maxY = max_Y;

    displayObjectsValues();

    piecesFlaggingAndColoration(&mesh);
    cout << "done" << endl;
}

void MainWindow::displayObjectsValues(){
    cout << "corps coord : " << endl;
    cout << "BG x:" << corps.xBG << " z:" << corps.zBG << endl;
    cout << "BD x:" << corps.xBD << " z:" << corps.zBD << endl;
    cout << "HG x:" << corps.xHG << " z:" << corps.zHG << endl;
    cout << "HD x:" << corps.xHD << " z:" << corps.zHD << endl;
    cout << "plan droite : " << p_droite.orientation << " coor " << p_droite.coor << endl;
    cout << "plan bas : " << p_bas.orientation << " coor " << p_bas.coor << endl;
    cout << "plan gauche : " << p_gauche.orientation << " coor " << p_gauche.coor << endl;
    cout << "plan haut : " << p_haut.orientation << " coor " << p_haut.coor << endl;
}


// ---------------- modifs de la selection avant découpe --------------------

void MainWindow::on_pushButton_LMoins_clicked(){
    corps.L -= PAS; corps.zBD -= PAS; corps.zBG -= PAS; corps.xHD -= PAS; corps.xBD -= PAS;
    p_bas.coor -= PAS; p_droite.coor -= PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

void MainWindow::on_pushButton_LPlus_clicked(){
    corps.L += PAS; corps.zBD += PAS; corps.zBG += PAS; corps.xHD += PAS; corps.xBD += PAS;
    p_bas.coor += PAS; p_droite.coor += PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

void MainWindow::on_pushButton_xMoins_clicked(){
    corps.xBD -= PAS; corps.xBG -= PAS; corps.xHD -= PAS; corps.xHG -= PAS;
    p_droite.coor -= PAS; p_gauche.coor -= PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

void MainWindow::on_pushButton_xPlus_clicked(){
    corps.xBD += PAS; corps.xBG += PAS; corps.xHD += PAS; corps.xHG += PAS;
    p_droite.coor += PAS; p_gauche.coor += PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

void MainWindow::on_pushButton_zMoins_clicked(){
    corps.zBD -= PAS; corps.zBG -= PAS; corps.zHD -= PAS; corps.zHG -= PAS;
    p_bas.coor -= PAS; p_haut.coor -= PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

void MainWindow::on_pushButton_zPlus_clicked(){
    corps.zBG += PAS; corps.zBD += PAS; corps.zHD += PAS; corps.zHG += PAS;
    p_bas.coor += PAS; p_haut.coor += PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

void MainWindow::on_pushButton_yMinMoins_clicked(){
    if(ui->comboBox_plans->currentText() == "Corps") corps.minY-=PAS;
    if(ui->comboBox_plans->currentText() == "Droite") p_droite.minY-=PAS;
    if(ui->comboBox_plans->currentText() == "Bas") p_bas.minY-=PAS;
    if(ui->comboBox_plans->currentText() == "Gauche") p_gauche.minY-=PAS;
    if(ui->comboBox_plans->currentText() == "Haut") p_haut.minY-=PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

void MainWindow::on_pushButton_yMinPlus_clicked(){
    if(ui->comboBox_plans->currentText() == "Corps") corps.minY+=PAS;
    if(ui->comboBox_plans->currentText() == "Droite") p_droite.minY+=PAS;
    if(ui->comboBox_plans->currentText() == "Bas") p_bas.minY+=PAS;
    if(ui->comboBox_plans->currentText() == "Gauche") p_gauche.minY+=PAS;
    if(ui->comboBox_plans->currentText() == "Haut") p_haut.minY+=PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

void MainWindow::on_pushButton_yMaxMoins_clicked(){
    if(ui->comboBox_plans->currentText() == "Corps") corps.maxY-=PAS;
    if(ui->comboBox_plans->currentText() == "Droite") p_droite.maxY-=PAS;
    if(ui->comboBox_plans->currentText() == "Bas") p_bas.maxY-=PAS;
    if(ui->comboBox_plans->currentText() == "Gauche") p_gauche.maxY-=PAS;
    if(ui->comboBox_plans->currentText() == "Haut") p_haut.maxY-=PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

void MainWindow::on_pushButton_yMaxPlus_clicked(){
    if(ui->comboBox_plans->currentText() == "Corps") corps.maxY+=PAS;
    if(ui->comboBox_plans->currentText() == "Droite") p_droite.maxY+=PAS;
    if(ui->comboBox_plans->currentText() == "Bas") p_bas.maxY+=PAS;
    if(ui->comboBox_plans->currentText() == "Gauche") p_gauche.maxY+=PAS;
    if(ui->comboBox_plans->currentText() == "Haut") p_haut.maxY+=PAS;
    displayObjectsValues();
    piecesFlaggingAndColoration(&mesh);
}

// ------------------------------------ flaggings & colorations ------------------------

void MainWindow::registerFacesAndVertices(MyMesh *_mesh){

    vector<VertexHandle> v1, v2, v3, v4, v5, v6, v7, v8, v9;

    for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; ++v_it){
        if(_mesh->data(*v_it).label == 1) v1.push_back(*v_it);
        if(_mesh->data(*v_it).label == 2) v2.push_back(*v_it);
        if(_mesh->data(*v_it).label == 3) v3.push_back(*v_it);
        if(_mesh->data(*v_it).label == 4) v4.push_back(*v_it);
        if(_mesh->data(*v_it).label == 5) v5.push_back(*v_it);
        if(_mesh->data(*v_it).label == 6) v6.push_back(*v_it);
        if(_mesh->data(*v_it).label == 7) v7.push_back(*v_it);
        if(_mesh->data(*v_it).label == 8) v8.push_back(*v_it);
        if(_mesh->data(*v_it).label == 9) v9.push_back(*v_it);
    }

    piecesVertices.push_back(v1);
    piecesVertices.push_back(v2);
    piecesVertices.push_back(v3);
    piecesVertices.push_back(v4);
    piecesVertices.push_back(v5);
    piecesVertices.push_back(v6);
    piecesVertices.push_back(v7);
    piecesVertices.push_back(v8);
    piecesVertices.push_back(v9);

    vector<FaceHandle> f1, f2, f3, f4, f5, f6, f7, f8, f9;

    for(auto f_it = _mesh->faces_begin() ; f_it != _mesh->faces_end() ; ++f_it){
        if(_mesh->data(*f_it).label == 1) f1.push_back(*f_it);
        if(_mesh->data(*f_it).label == 2) f2.push_back(*f_it);
        if(_mesh->data(*f_it).label == 3) f3.push_back(*f_it);
        if(_mesh->data(*f_it).label == 4) f4.push_back(*f_it);
        if(_mesh->data(*f_it).label == 5) f5.push_back(*f_it);
        if(_mesh->data(*f_it).label == 6) f6.push_back(*f_it);
        if(_mesh->data(*f_it).label == 7) f7.push_back(*f_it);
        if(_mesh->data(*f_it).label == 8) f8.push_back(*f_it);
        if(_mesh->data(*f_it).label == 9) f9.push_back(*f_it);
    }

    piecesFaces.push_back(f1);
    piecesFaces.push_back(f2);
    piecesFaces.push_back(f3);
    piecesFaces.push_back(f4);
    piecesFaces.push_back(f5);
    piecesFaces.push_back(f6);
    piecesFaces.push_back(f7);
    piecesFaces.push_back(f8);
    piecesFaces.push_back(f9);

}

void MainWindow::piecesFlaggingAndColoration(MyMesh *_mesh){
    resetAllColorsAndThickness(_mesh);
    faceLabeling.clear();
    for(int i=0 ; i<(int)_mesh->n_faces() ; i++) faceLabeling.push_back(-1);

    // -- FLAGGING DU CORPS
    //parcour de toutes les faces pour déterminer si elles sont dans le corps (completement)
    for(int current_face=0 ; current_face < (int)_mesh->n_faces() ; current_face++){
        FaceHandle fh = _mesh->face_handle(current_face);
        bool is_inside = true;
        MyMesh::FaceVertexIter fv_it = _mesh->fv_iter(fh);
        for(fv_it ; fv_it.is_valid() ; ++fv_it){
            int v_voisin_ID = (*fv_it).idx();
            MyMesh::Point P = _mesh->point(_mesh->vertex_handle(v_voisin_ID));
            if(P[0] < corps.xHG || P[0] > corps.xHD || P[2] < corps.zHG ||
               P[2] > corps.zBG || P[1] < corps.minY || P[1] > corps.maxY){
                is_inside = false;
                break;
            }
        }
        if(is_inside){
            faceLabeling.at(current_face) = 0;
            //cout << "is inside" << endl;
        }
    }

    // -- SI IL FAUT ENCORE CUT ON COLORIE LES FACES QUI VONT L'ETRE
    if(!isCut){
        //on commence par colorer le corps et les faces non etiquettées
        for(int i=0 ; i<(int)faceLabeling.size() ; i++){
            if(faceLabeling.at(i) == 0) _mesh->set_color(_mesh->face_handle(i), colors.at(0));
            else if(faceLabeling.at(i) == -1) _mesh->set_color(_mesh->face_handle(i), MyMesh::Color( 150, 150, 150));

        }
        //puis pour chaque face on vérifie sa candidature à être cut
        int nb_faces_plans = 0;

        for(MyMesh::FaceIter curFace = _mesh->faces_begin(); curFace != _mesh->faces_end(); curFace++){
            if(isCutByPlan(_mesh, (*curFace).idx(), p_droite) || isCutByPlan(_mesh, (*curFace).idx(), p_bas) ||
               isCutByPlan(_mesh, (*curFace).idx(), p_gauche) || isCutByPlan(_mesh, (*curFace).idx(), p_haut)){
                _mesh->set_color(*curFace, colors.at(9));
                nb_faces_plans++;
                //cout << "face is in plan" << endl;
            }
        }
        cout << "nombre de faces dans les plans : " << nb_faces_plans << "sur " << faceLabeling.size() << endl;
    }

    // -- FLAGGING DES PIECES PUIS COLORATION APRES CUT
    //coloration avec couleurs générées aléatoirement selon les labels de faces
    else{
        //flagging
        correctLabelisation(_mesh);
        //enregistrement dans les tableaux
        registerFacesAndVertices(_mesh);
        //coloration
        for(auto f_it = _mesh->faces_begin(); f_it != _mesh->faces_end(); f_it++){
            if(_mesh->data(*f_it).label == 0) _mesh->set_color(*f_it, MyMesh::Color( 150, 150, 150));
            else _mesh->set_color(*f_it, colors.at(_mesh->data(*f_it).label));
        }
     }

    //for(int i=0 ; i<(int)faceLabeling.size() ; i++) cout << faceLabeling.at(i) << " ";
    //cout << endl;

    displayMesh(_mesh);
}

void MainWindow::trajectoryColorationUpdate(MyMesh *_mesh){
    resetAllColorsAndThickness(_mesh);

    for(auto f_it = _mesh->faces_begin(); f_it != _mesh->faces_end(); f_it++){
        if(_mesh->data(*f_it).label == 0) _mesh->set_color(*f_it, MyMesh::Color( 150, 150, 150));
        else _mesh->set_color(*f_it, colors.at(_mesh->data(*f_it).label));
    }


    //trajectory vertices
    for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; ++v_it){
        if(_mesh->data(*v_it).label >= 10 && _mesh->data(*v_it).label != 20){
            qDebug() << "vertice trajectoire trouve";
            _mesh->set_color(*v_it, colors.at(_mesh->data(*v_it).label - 10));
            _mesh->data(*v_it).thickness = 6;
        }
        if(_mesh->data(*v_it).label == 20){
            _mesh->set_color(*v_it, MyMesh::Color(225, 50, 50));
            _mesh->data(*v_it).thickness = 12;
        }
    }

    displayMesh(_mesh);
}

// --------------------------- cutting the mesh and math ---------------------------

//renvoie un bool si la face est coupée par le plan donné en argument
bool MainWindow::isCutByPlan(MyMesh *_mesh, int faceID, Plan plan){
    FaceHandle fh = _mesh->face_handle(faceID);
    MyMesh::FaceVertexIter fv_it = mesh.fv_iter(fh);
    MyMesh::Point A = mesh.point(mesh.vertex_handle((*fv_it).idx()));
    ++fv_it;
    MyMesh::Point B = mesh.point(mesh.vertex_handle((*fv_it).idx()));
    ++fv_it;
    MyMesh::Point C = mesh.point(mesh.vertex_handle((*fv_it).idx()));

    //check pour la hauteur axe Y
    if((A[1] > plan.maxY && B[1] > plan.maxY && C[1] > plan.maxY) ||
       (A[1] < plan.minY && B[1] < plan.minY && C[1] < plan.minY))
        return false;

    //on cherche si on travaille sur les X ou les Z
    int ind;
    //si l'axe est vertical
    if(plan.orientation == "vertical") ind = 0; //X
    else ind = 2; //Z

    //check intersection
    if((A[ind] <= plan.coor && B[ind] >= plan.coor) || (A[ind] >= plan.coor && B[ind] <= plan.coor) ||
       (A[ind] <= plan.coor && C[ind] >= plan.coor) || (A[ind] >= plan.coor && C[ind] <= plan.coor) ||
       (B[ind] <= plan.coor && C[ind] >= plan.coor) || (B[ind] >= plan.coor && C[ind] <= plan.coor))
        return true;

    return false;
}

void MainWindow::generateCenterCoordinates(){
    center_Y = (corps.minY + corps.maxY)/2;
    center_X = (p_droite.coor + p_gauche.coor)/2;
    center_Z = (p_haut.coor + p_bas.coor)/2;
}

float MainWindow::scalar_product(QVector3D a, QVector3D b) {
    float p;
    p = a.x()*b.x()+a.y()*b.y()+a.z()*b.z();
    //produit scalaire (dot product)
    return p;
}

//Renvoie le point d'intersection entre le plan et le segment
//plane_point un point sur le plan ; plane_normal la normale du plan ; a le vecteur origine du segment AB ; b le vecteur point du segment AB
QVector3D MainWindow::intersection_calculation(QVector3D plane_point, QVector3D plane_normal, QVector3D a, QVector3D b){
    QVector3D difference;
    difference = b - plane_point;
    double p1 = scalar_product(difference, plane_normal);
    double p2 = scalar_product(a, plane_normal);
    double p3 = p1 / p2;
    return b - a * p3;
}

QVector3D edge_plan_intersection(MyMesh *_mesh, const MyMesh::EdgeHandle &eh, QVector3D &p, QVector3D &n){
    auto heh = _mesh->halfedge_handle(eh, 0);
    auto vert_a = _mesh->from_vertex_handle(heh);
    auto vert_b = _mesh->to_vertex_handle(heh);

    QVector3D a = QVector3D(_mesh->point(vert_a)[0], _mesh->point(vert_a)[1], _mesh->point(vert_a)[2]);
    QVector3D b = QVector3D(_mesh->point(vert_b)[0], _mesh->point(vert_b)[1], _mesh->point(vert_b)[2]);

    QVector3D ap = p-a;
    QVector3D pb = b-p;

    float da = QVector3D::dotProduct(ap, n);
    float db = QVector3D::dotProduct(pb, n);

    if(da == -db)
        return a;

    float ca = db/(da+db);
    if(ca < 0) ca = -ca;
    float cb = da/(da+db);
    if(cb < 0) cb = -cb;

    return (ca*a + cb*b);
}

//renvoie un bool si l'edge est coupé par le plan donnée en argument
bool MainWindow::edgeIsCutByPlan(MyMesh *_mesh, int edgeID, Plan plan){
    EdgeHandle eh = _mesh->edge_handle(edgeID);

    HalfedgeHandle ah = _mesh->halfedge_handle(eh, 0);
    VertexHandle v1 = _mesh->from_vertex_handle(ah);
    VertexHandle v2 = _mesh->to_vertex_handle(ah);
    MyMesh::Point a = _mesh->point(v1);
    MyMesh::Point b = _mesh->point(v2);

    if(plan.orientation == "horizontal"){
        if((a[2] < plan.coor && b[2] > plan.coor) || (a[2] > plan.coor && b[2] < plan.coor))
            return true;
        else return false;
    }

    else{
        if((a[0] < plan.coor && b[0] > plan.coor) || (a[0] > plan.coor && b[0] < plan.coor))
            return true;
        else return false;
    }
}


QVector3D MainWindow::getCenterOfFace(MyMesh *_mesh, int faceID){
    FaceHandle fh = _mesh->face_handle(faceID);
    float x = 0;    float y = 0;    float z = 0;

    MyMesh::FaceVertexIter fv_it = _mesh->fv_iter(fh);
    for( ; fv_it.is_valid() ; ++fv_it){
        MyMesh::Point p = _mesh->point(fv_it);
        x += p[0];
        y += p[1];
        z += p[2];
    }

    return QVector3D( x/3, y/3, z/3 );
}



bool edge_intersects(MyMesh *_mesh, const MyMesh::EdgeHandle &eh, QVector3D &p, QVector3D &n){
    auto heh = _mesh->halfedge_handle(eh, 0);
    auto vert_a = _mesh->from_vertex_handle(heh);
    auto vert_b = _mesh->to_vertex_handle(heh);

    QVector3D a = QVector3D(_mesh->point(vert_a)[0], _mesh->point(vert_a)[1], _mesh->point(vert_a)[2]);
    QVector3D b = QVector3D(_mesh->point(vert_b)[0], _mesh->point(vert_b)[1], _mesh->point(vert_b)[2]);

    QVector3D pa = a-p;
    QVector3D pb = b-p;

    return( (QVector3D::dotProduct(pa, n) * QVector3D::dotProduct(pb, n)) < -0.00000001);
}

void cut_mesh(MyMesh *_mesh, QVector3D p, QVector3D n, Plan plan){
    qDebug() << "cutting mesh along a plane";
    _mesh->request_vertex_status();
    _mesh->request_edge_status();
    _mesh->request_face_status();

    n.normalize();
    std::vector<MyMesh::VertexHandle> added_vertices;
    for(auto e_it = _mesh->edges_begin(); e_it != _mesh->edges_end(); e_it++){
        HalfedgeHandle ah = _mesh->halfedge_handle(e_it, 0);
        VertexHandle v1 = _mesh->from_vertex_handle(ah);
        VertexHandle v2 = _mesh->to_vertex_handle(ah);
        MyMesh::Point p1 = _mesh->point(v1);
        MyMesh::Point p2 = _mesh->point(v2);


        if( (p1[1] < plan.maxY && p1[1] > plan.minY) &&
            (p2[1] < plan.maxY && p2[1] > plan.minY)){
            if(edge_intersects(_mesh, *e_it, p, n)){
                qDebug() << "Edge intersection spotted";
                QVector3D nv = edge_plan_intersection(_mesh, *e_it, p, n);
                added_vertices.push_back(_mesh->split_copy(*e_it, MyMesh::Point(nv.x(), nv.y(), nv.z())));
            }
        }
    }

    qDebug() << added_vertices.size() << " new vertices added ! ";
    int count = 0;
    for(auto vert:added_vertices){
        qDebug() << count++;
        auto vert_copy = _mesh->add_vertex(MyMesh::Point(_mesh->point(vert)[0], _mesh->point(vert)[1], _mesh->point(vert)[2]));

        vector<MyMesh::FaceHandle> faces;
        for(auto vf_it = _mesh->vf_begin(vert) ; vf_it.is_valid(); vf_it++){
            for(auto fv_it = _mesh->fv_begin(*vf_it); fv_it.is_valid(); fv_it++){
            QVector3D vert_as_vect = QVector3D(_mesh->point(*fv_it)[0], _mesh->point(*fv_it)[1], _mesh->point(*fv_it)[2]);
                if(QVector3D::dotProduct((vert_as_vect-p), n) > 0.0001){
                    faces.push_back(*vf_it);
                    break;
                }else if(QVector3D::dotProduct((vert_as_vect-p), n) < -0.0001){
                    break;
                }
            }
        }
        for(auto fh:faces){
            //qDebug() << "aled";
            bool update_face = false;
            std::vector<VertexHandle> face_vertices;
            for(auto fv_it = _mesh->fv_begin(fh); fv_it.is_valid(); fv_it++){
                if(*fv_it == vert){
                    face_vertices.push_back(vert_copy);
                }else{
                    face_vertices.push_back(*fv_it);
                }
            }

            if(face_vertices.size() == 3){
                _mesh->delete_face(fh, false);
                _mesh->add_face(face_vertices);
                _mesh->garbage_collection();
                _mesh->request_vertex_status();
                _mesh->request_edge_status();
                _mesh->request_face_status();
            }
        }
    }
}

void MainWindow::on_pushButton_decoupage_clicked(){
    cout << "start of decoupage ... ";
    //request for delete
    mesh.request_vertex_status();
    mesh.request_edge_status();
    mesh.request_face_status();

    generateCenterCoordinates();

    cut_mesh(&mesh,
             QVector3D(p_droite.coor, 0, 0),
             QVector3D(1, 0, 0),
             p_droite
    );

    cut_mesh(&mesh,
             QVector3D(p_gauche.coor, 0, 0),
             QVector3D(-1, 0, 0),
             p_gauche
    );

    cut_mesh(&mesh,
             QVector3D(0, 0, p_haut.coor),
             QVector3D(0, 0, 1),
             p_haut
    );

    cut_mesh(&mesh,
             QVector3D(0, 0, p_bas.coor),
             QVector3D(0, 0, -1),
             p_bas
    );


    //creation de la puzzleState pour futures opérations
    puzzle.HG = 2; puzzle.H = 3; puzzle.HD = 4;
    puzzle.G  = 9; puzzle.M = 1; puzzle.D  = 5;
    puzzle.BG = 8; puzzle.B = 7; puzzle.BD = 6;
    resetTrajectoryChecking();


    //delete all elements marked as deleted
    mesh.garbage_collection();
    cout << "done" << endl;
    isCut = true;
    resetAllColorsAndThickness(&mesh);
    piecesFlaggingAndColoration(&mesh);
    displayMesh(&mesh);
}

// ---------------------- spacing between pieces --------------------

void MainWindow::on_pushButton_espMoins_clicked(){
    if(ESP-PAS >= 0.0) ESP -= PAS;
    else ESP = 0.0;
    ui->label_esp->setText(QString::number(ESP));

    MyMesh::VertexIter curVertex = mesh.vertices_begin();
    for( ; curVertex != mesh.vertices_end() ; ++curVertex){
        //HG
        if( mesh.data(curVertex).label == 2 ){
            mesh.point(curVertex)[0] += PAS;
            mesh.point(curVertex)[2] += PAS;
        }
        //H
        if( mesh.data(curVertex).label == 3 ){
            mesh.point(curVertex)[2] += PAS;
        }
        //HD
        if( mesh.data(curVertex).label == 4 ){
            mesh.point(curVertex)[0] -= PAS;
            mesh.point(curVertex)[2] += PAS;
        }
        //D
        if( mesh.data(curVertex).label == 5 ){
            mesh.point(curVertex)[0] -= PAS;
        }
        //BD
        if( mesh.data(curVertex).label == 6 ){
            mesh.point(curVertex)[0] -= PAS;
            mesh.point(curVertex)[2] -= PAS;
        }
        //B
        if( mesh.data(curVertex).label == 7 ){
            mesh.point(curVertex)[2] -= PAS;
        }
        //BG
        if( mesh.data(curVertex).label == 8 ){
            mesh.point(curVertex)[0] += PAS;
            mesh.point(curVertex)[2] -= PAS;
        }
        //G
        if( mesh.data(curVertex).label == 9 ){
            mesh.point(curVertex)[0] += PAS;
        }
    }

    piecesFlaggingAndColoration(&mesh);
    displayMesh(&mesh);
}

void MainWindow::on_pushButton_espPlus_clicked(){
    ESP += PAS;
    ui->label_esp->setText(QString::number(ESP));

    MyMesh::VertexIter curVertex = mesh.vertices_begin();
    for( ; curVertex != mesh.vertices_end() ; ++curVertex){
        //HG
        if( mesh.data(curVertex).label == 2 ){
            mesh.point(curVertex)[0] -= PAS;
            mesh.point(curVertex)[2] -= PAS;
        }
        //H
        if( mesh.data(curVertex).label == 3 ){
            mesh.point(curVertex)[2] -= PAS;
        }
        //HD
        if( mesh.data(curVertex).label == 4 ){
            mesh.point(curVertex)[0] += PAS;
            mesh.point(curVertex)[2] -= PAS;
        }
        //D
        if( mesh.data(curVertex).label == 5 ){
            mesh.point(curVertex)[0] += PAS;
        }
        //BD
        if( mesh.data(curVertex).label == 6 ){
            mesh.point(curVertex)[0] += PAS;
            mesh.point(curVertex)[2] += PAS;
        }
        //B
        if( mesh.data(curVertex).label == 7 ){
            mesh.point(curVertex)[2] += PAS;
        }
        //BG
        if( mesh.data(curVertex).label == 8 ){
            mesh.point(curVertex)[0] -= PAS;
            mesh.point(curVertex)[2] += PAS;
        }
        //G
        if( mesh.data(curVertex).label == 9 ){
            mesh.point(curVertex)[0] -= PAS;
        }
    }


    piecesFlaggingAndColoration(&mesh);
    displayMesh(&mesh);
}

// ----------------------- labels -----------------------

void MainWindow::connexCompoFlagging(MyMesh* _mesh, int currentLabel, int currentFace){
    FaceHandle fh = _mesh->face_handle(currentFace);
    _mesh->data(fh).label = currentLabel;
    MyMesh::FaceFaceIter ff_it = _mesh->ff_begin(fh);
    for(ff_it ; ff_it.is_valid() ; ++ff_it){
        if(_mesh->data(*ff_it).label == 0){
            connexCompoFlagging(_mesh, currentLabel, (*ff_it).idx());
        }
    }
}

void MainWindow::resetFacesLabels(MyMesh *_mesh){
    MyMesh::FaceIter curFace = _mesh->faces_begin();
    for( ; curFace != _mesh->faces_end() ; ++curFace){
        _mesh->data(curFace).label = 0;
    }
}

void MainWindow::resetVertexLabels(MyMesh *_mesh){
    MyMesh::VertexIter curVertex = _mesh->vertices_begin();
    for( ; curVertex != _mesh->vertices_end() ; ++curVertex){
        _mesh->data(curVertex).label = 0;
    }
}

void MainWindow::correctLabelisation(MyMesh *_mesh){

    resetAllColorsAndThickness(_mesh);
    resetFacesLabels(_mesh);
    resetVertexLabels(_mesh);

    for(int currentFace = 0 ; currentFace < _mesh->n_faces() ; currentFace++){
        FaceHandle fh = _mesh->face_handle(currentFace);
        if( _mesh->data(fh).label == 0 ){
            QVector3D centerOfFace = getCenterOfFace(_mesh, fh.idx());

            //HG
            if(centerOfFace.x() < p_gauche.coor && centerOfFace.z() < p_haut.coor ){
                connexCompoFlagging(_mesh, 2, currentFace);
            }

            //H
            if(centerOfFace.x() > p_gauche.coor && centerOfFace.x() < p_droite.coor &&
               centerOfFace.z() < p_haut.coor ){
                connexCompoFlagging(_mesh, 3, currentFace);
            }

            //HD
            if(centerOfFace.x() > p_droite.coor && centerOfFace.z() < p_haut.coor ){
                connexCompoFlagging(_mesh, 4, currentFace);
            }

            //D
            if(centerOfFace.x() > p_droite.coor && centerOfFace.z() > p_haut.coor &&
               centerOfFace.z() < p_bas.coor ){
                connexCompoFlagging(_mesh, 5, currentFace);
            }

            //BD
            if(centerOfFace.x() > p_droite.coor && centerOfFace.z() > p_bas.coor ){
                connexCompoFlagging(_mesh, 6, currentFace);
            }

            //B
            if(centerOfFace.x() > p_gauche.coor && centerOfFace.x() < p_droite.coor
               && centerOfFace.z() > p_bas.coor ){
                connexCompoFlagging(_mesh, 7, currentFace);
            }

            //BG
            if(centerOfFace.x() < p_gauche.coor && centerOfFace.z() > p_bas.coor ){
                connexCompoFlagging(_mesh, 8, currentFace);
            }

            //G
            if(centerOfFace.x() < p_gauche.coor && centerOfFace.z() > p_haut.coor &&
               centerOfFace.z() < p_bas.coor){
                connexCompoFlagging(_mesh, 9, currentFace);
            }

            //CENTER
            if(centerOfFace.x() > p_gauche.coor && centerOfFace.x() < p_droite.coor &&
               centerOfFace.z() > p_haut.coor && centerOfFace.z() < p_bas.coor){
                connexCompoFlagging(_mesh, 1, currentFace);
            }
        }
    }


    //labelisation des vertex

    for(int currentFace = 0 ; currentFace < _mesh->n_faces() ; currentFace++){
        FaceHandle fh = _mesh->face_handle(currentFace);
        MyMesh::FaceVertexIter fv_it = _mesh->fv_iter(fh);
        for( ; fv_it.is_valid() ; ++fv_it){
            _mesh->data(fv_it).label = _mesh->data(fh).label;
        }
    }
}



// --------------------------- trajectories and collisions ------------------------

vector<QVector3D> MainWindow::calcTrajectoryCoordinates(QVector3D M, QVector3D O, string orientation){

    vector<QVector3D> trajectoire;

    if(orientation == "x_axis"){
        //premiere position
        trajectoire.push_back(M);

        //puis tout les 30 degres
        for(int i=1 ; i<=6 ; i++){
            float xM, yM, x, y;

            float angle = (i*30) * M_PI/180;
            xM = M.x() - O.x();
            yM = M.y() - O.y();
            x = xM * cos(angle) + yM * sin(angle) + O.x();
            y = - xM * sin(angle) + yM * cos(angle) + O.y();
            trajectoire.push_back( QVector3D( x, y, M.z() ) );
        }
    }

    else if(orientation == "z_axis"){
        //premiere position
        trajectoire.push_back(M);

        //puis tout les 30 degres
        for(int i=1 ; i<=6 ; i++){
            float zM, yM, z, y;

            float angle = (i*30) * M_PI/180;
            zM = M.z() - O.z();
            yM = M.y() - O.y();
            z = zM * cos(angle) + yM * sin(angle) + O.z();
            y = - zM * sin(angle) + yM * cos(angle) + O.y();
            trajectoire.push_back( QVector3D( M.x(), y, z ) );
        }
    }

    return trajectoire;

}

void MainWindow::resetTrajectoryChecking(){
    puzzle.checkingH = false;
    puzzle.checkingG = false;
    puzzle.checkingD = false;
    puzzle.checkingB = false;
}

void MainWindow::addTrajectory(MyMesh *_mesh){
    //buffer doit être clear !
    qDebug() << "add trajectory requested";
    if(puzzle.checkingB){
        for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; v_it++ ){
            if(_mesh->data(*v_it).label == puzzle.BG || _mesh->data(*v_it).label == puzzle.BD || _mesh->data(*v_it).label == puzzle.B){
                vector<QVector3D> traj = calcTrajectoryCoordinates( QVector3D( _mesh->point(v_it)[0], _mesh->point(v_it)[1], _mesh->point(v_it)[2] ),
                                                                    QVector3D(center_X, center_Y, center_Z),
                                                                    "x_axis");
                //qDebug() << "adding vertex ..";
                for(int i=0 ; i<(int)traj.size() ; i++){
                    //qDebug() << "vertex added";
                    VertexHandle vh = _mesh->add_vertex(MyMesh::Point( traj.at(i).x(), traj.at(i).y(), traj.at(i).z() ));
                    if(_mesh->data(*v_it).label == puzzle.BG) trajectoryBuffer1.push_back(vh);
                    else if(_mesh->data(*v_it).label == puzzle.B) trajectoryBuffer2.push_back(vh);
                    else if(_mesh->data(*v_it).label == puzzle.BD) trajectoryBuffer3.push_back(vh);

                    _mesh->data(vh).label = _mesh->data(*v_it).label + 10; //ref in header
                }
            }
        }
        //qDebug() << "done adding vertexes";
    }

    if(puzzle.checkingD){
        for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; v_it++ ){
            if(_mesh->data(*v_it).label == puzzle.BD || _mesh->data(*v_it).label == puzzle.HD || _mesh->data(*v_it).label == puzzle.D){
                vector<QVector3D> traj = calcTrajectoryCoordinates( QVector3D( _mesh->point(v_it)[0], _mesh->point(v_it)[1], _mesh->point(v_it)[2] ),
                                                                    QVector3D(center_X, center_Y, center_Z),
                                                                    "z_axis");
                //qDebug() << "adding vertex ..";
                for(int i=0 ; i<(int)traj.size() ; i++){
                    //qDebug() << "vertex added";
                    VertexHandle vh = _mesh->add_vertex(MyMesh::Point( traj.at(i).x(), traj.at(i).y(), traj.at(i).z() ));
                    if(_mesh->data(*v_it).label == puzzle.BD) trajectoryBuffer1.push_back(vh);
                    else if(_mesh->data(*v_it).label == puzzle.D) trajectoryBuffer2.push_back(vh);
                    else if(_mesh->data(*v_it).label == puzzle.HD) trajectoryBuffer3.push_back(vh);

                    _mesh->data(vh).label = _mesh->data(*v_it).label + 10; //ref in header
                }
            }
        }
        //qDebug() << "done adding vertexes";
    }

    if(puzzle.checkingH){
        for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; v_it++ ){
            if(_mesh->data(*v_it).label == puzzle.HD || _mesh->data(*v_it).label == puzzle.HG || _mesh->data(*v_it).label == puzzle.H){
                vector<QVector3D> traj = calcTrajectoryCoordinates( QVector3D( _mesh->point(v_it)[0], _mesh->point(v_it)[1], _mesh->point(v_it)[2] ),
                                                                    QVector3D(center_X, center_Y, center_Z),
                                                                    "x_axis");
                //qDebug() << "adding vertex ..";
                for(int i=0 ; i<(int)traj.size() ; i++){
                    //qDebug() << "vertex added";
                    VertexHandle vh = _mesh->add_vertex(MyMesh::Point( traj.at(i).x(), traj.at(i).y(), traj.at(i).z() ));
                    if(_mesh->data(*v_it).label == puzzle.HG) trajectoryBuffer1.push_back(vh);
                    else if(_mesh->data(*v_it).label == puzzle.H) trajectoryBuffer2.push_back(vh);
                    else if(_mesh->data(*v_it).label == puzzle.HD) trajectoryBuffer3.push_back(vh);

                    _mesh->data(vh).label = _mesh->data(*v_it).label + 10; //ref in header
                }
            }
        }
        //qDebug() << "done adding vertexes";
    }

    if(puzzle.checkingG){
        for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; v_it++ ){
            if(_mesh->data(*v_it).label == puzzle.HG || _mesh->data(*v_it).label == puzzle.BG || _mesh->data(*v_it).label == puzzle.G){
                vector<QVector3D> traj = calcTrajectoryCoordinates( QVector3D( _mesh->point(v_it)[0], _mesh->point(v_it)[1], _mesh->point(v_it)[2] ),
                                                                    QVector3D(center_X, center_Y, center_Z),
                                                                    "z_axis");
                //qDebug() << "adding vertex ..";
                for(int i=0 ; i<(int)traj.size() ; i++){
                    //qDebug() << "vertex added";
                    VertexHandle vh = _mesh->add_vertex(MyMesh::Point( traj.at(i).x(), traj.at(i).y(), traj.at(i).z() ));
                    if(_mesh->data(*v_it).label == puzzle.HG) trajectoryBuffer1.push_back(vh);
                    else if(_mesh->data(*v_it).label == puzzle.G) trajectoryBuffer2.push_back(vh);
                    else if(_mesh->data(*v_it).label == puzzle.BG) trajectoryBuffer3.push_back(vh);

                    _mesh->data(vh).label = _mesh->data(*v_it).label + 10; //ref in header
                }
            }
        }
        //qDebug() << "done adding vertexes";
    }
}

void MainWindow::removeTrajectory(MyMesh *_mesh){
    qDebug() << "removeTrajectory requested";
    _mesh->request_vertex_status();

    for(int i=0 ; i<(int)trajectoryBuffer1.size() ; i++){
        qDebug() << "vertex deleted";
        _mesh->delete_vertex(trajectoryBuffer1.at(i));
    }

    for(int i=0 ; i<(int)trajectoryBuffer2.size() ; i++){
        qDebug() << "vertex deleted";
        _mesh->delete_vertex(trajectoryBuffer2.at(i));
    }

    for(int i=0 ; i<(int)trajectoryBuffer3.size() ; i++){
        qDebug() << "vertex deleted";
        _mesh->delete_vertex(trajectoryBuffer3.at(i));
    }

    _mesh->garbage_collection();

    trajectoryBuffer1.clear();
    trajectoryBuffer2.clear();
    trajectoryBuffer3.clear();

}

// -- collisions

float MainWindow::getDistanceBtw2Vertices(MyMesh *_mesh, QVector3D a, QVector3D b){
    float distance = pow( b.x() - a.x(), 2.0 );
    distance += pow( b.y() - a.y(), 2.0 );
    distance += pow( b.z() - a.z(), 2.0 );
    return sqrt(distance);
}

/*void MainWindow::displayTrajectoryOnPlane(MyMesh *_mesh){

    if(puzzle.checkingB){

        for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; ++v_it){

            if(_mesh->data(*v_it).label == puzzle.BG){
                MyMesh::Point Porigine = _mesh->point(v_it);
                //le z reste inchangé
                float z = Porigine[2];
                //le y du point projeté sur le plan va correspondre a la distance
                //entre le point d'origine et la droite Oz passant par le centre de rotation
                float y = getDistanceBtw2Vertices( _mesh,
                                                   QVector3D(Porigine[0], Porigine[1], Porigine[2]),
                                                   QVector3D( center_X, center_Y, Porigine[2] ));
                //pour le moment centré
                float x = center_X;
            }
        }
    }

    if(puzzle.checkingD){

    }

    if(puzzle.checkingH){

    }

    if(puzzle.checkingG){

    }
}*/

float sign(QVector3D p1, QVector3D p2, QVector3D p3, string axis){
    if(axis == "z_axis") return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.x() - p3.y());
    else if (axis == "x_axis") return (p1.z() - p3.z()) * (p2.y() - p3.y()) - (p2.z() - p3.z()) * (p1.z() - p3.y());
}

bool MainWindow::pointIsInTriangle(MyMesh *_mesh, QVector3D p, FaceHandle fh){
    float d1, d2, d3;
    bool has_neg, has_pos;

    MyMesh::FaceVertexIter fv_it = _mesh->fv_iter(fh);
    MyMesh::Point a = _mesh->point(fv_it); fv_it++;
    MyMesh::Point b = _mesh->point(fv_it); fv_it++;
    MyMesh::Point c = _mesh->point(fv_it);

    if(puzzle.checkingD || puzzle.checkingG){
        d1 = sign( p,
                   QVector3D( a[0], a[1], a[2] ),
                   QVector3D( b[0], b[1], b[2] ),
                   "z_axis");
        d2 = sign( p,
                   QVector3D( b[0], b[1], b[2] ),
                   QVector3D( c[0], c[1], c[2] ),
                   "z_axis");
        d3 = sign( p,
                   QVector3D( c[0], c[1], c[2] ),
                   QVector3D( a[0], a[1], a[2] ),
                   "z_axis");
    }

    if(puzzle.checkingH || puzzle.checkingB){
        d1 = sign( p,
                   QVector3D( a[0], a[1], a[2] ),
                   QVector3D( b[0], b[1], b[2] ),
                   "x_axis");
        d2 = sign( p,
                   QVector3D( b[0], b[1], b[2] ),
                   QVector3D( c[0], c[1], c[2] ),
                   "x_axis");
        d3 = sign( p,
                   QVector3D( c[0], c[1], c[2] ),
                   QVector3D( a[0], a[1], a[2] ),
                   "x_axis");
    }

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

void MainWindow::checkingTrajectoryCollisions(MyMesh *_mesh){

    if(puzzle.checkingB){

        for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; ++v_it){
            //pour chacun des points de la trajectoire des 3 pieces en rotation
            if( _mesh->data(*v_it).label == puzzle.BG+10 || _mesh->data(*v_it).label == puzzle.B+10 || _mesh->data(*v_it).label == puzzle.BD+10 ){
                MyMesh::Point P = _mesh->point(v_it);

                //on check chacune des faces des 3 pieces directement liées

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.G ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.G ).at(i).idx());

                    if(c.z() > P[2]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.G ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.M ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.M ).at(i).idx());

                    if(c.z() > P[2]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.M ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.D ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.D ).at(i).idx());

                    if(c.z() > P[2]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.D ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }
            }
        }
    }

    if(puzzle.checkingD){

        for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; ++v_it){
            //pour chacun des points de la trajectoire des 3 pieces en rotation
            if( _mesh->data(*v_it).label == puzzle.BD+10 || _mesh->data(*v_it).label == puzzle.D+10 || _mesh->data(*v_it).label == puzzle.HD+10 ){
                MyMesh::Point P = _mesh->point(v_it);

                //on check chacune des faces des 3 pieces directement liées

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.B ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.B ).at(i).idx());

                    if(c.x() > P[0]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.B ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.M ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.M ).at(i).idx());

                    if(c.x() > P[0]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.M ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.H ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.H ).at(i).idx());

                    if(c.x() > P[0]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.H ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }
            }
        }

    }

    if(puzzle.checkingH){

        for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; ++v_it){
            //pour chacun des points de la trajectoire des 3 pieces en rotation
            if( _mesh->data(*v_it).label == puzzle.HG+10 || _mesh->data(*v_it).label == puzzle.H+10 || _mesh->data(*v_it).label == puzzle.HD+10 ){
                MyMesh::Point P = _mesh->point(v_it);

                //on check chacune des faces des 3 pieces directement liées

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.G ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.G ).at(i).idx());

                    if(c.z() < P[2]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.G ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.M ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.M ).at(i).idx());

                    if(c.z() < P[2]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.M ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.D ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.D ).at(i).idx());

                    if(c.z() < P[2]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.D ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }
            }
        }
    }

    if(puzzle.checkingG){

        for(auto v_it = _mesh->vertices_begin() ; v_it != _mesh->vertices_end() ; ++v_it){
            //pour chacun des points de la trajectoire des 3 pieces en rotation
            if( _mesh->data(*v_it).label == puzzle.HG+10 || _mesh->data(*v_it).label == puzzle.G+10 || _mesh->data(*v_it).label == puzzle.BG+10 ){
                MyMesh::Point P = _mesh->point(v_it);

                //on check chacune des faces des 3 pieces directement liées

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.B ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.B ).at(i).idx());

                    if(c.x() < P[0]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.B ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.M ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.M ).at(i).idx());

                    if(c.x() < P[0]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.M ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }

                for(int i=0 ; i<(int)piecesFaces.at( puzzle.H ).size() ; i++){
                    //on vérifie si le vertice se trouve entre la face et le centre du puzzle
                    //si c'est le cas alors il est possible qu'il soit dans une autre piece
                    QVector3D c = getCenterOfFace(_mesh, piecesFaces.at( puzzle.H ).at(i).idx());

                    if(c.x() < P[0]){
                        //il y a possibilité d'être à l'interieur d'une piece
                        //on trace une droite vers l'exterieur, si on traverse un nombre impair de faces,
                        //le vertice courant est bel et bien a l'intérieur d'une piece
                        if( pointIsInTriangle(_mesh,
                                              QVector3D( P[0], P[1], P[2] ),
                                              piecesFaces.at( puzzle.H ).at(i)) ){
                            _mesh->data(*v_it).label = 20;
                            break; //une seule intersection suffit, on peut passer au point suivant
                        }
                    }
                }
            }
        }
    }

}

// ----------------------------- rotations -----------------------------

void MainWindow::on_pushButton_RB_clicked(){
    cout << "RB clicked" << endl;

    //affichage des trajectoires des pieces
    if(!puzzle.checkingB){
        removeTrajectory(&mesh);
        resetTrajectoryChecking();
        puzzle.checkingB = true;
        addTrajectory(&mesh);
        if(ui->checkBox_collisionsCheck->isChecked())
            checkingTrajectoryCollisions(&mesh);

        trajectoryColorationUpdate(&mesh);
    }

    else if(puzzle.checkingB){
        removeTrajectory(&mesh);
        for(int i=0 ; i<(int)mesh.n_vertices() ; i++){
            VertexHandle vh = mesh.vertex_handle(i);
            //si on a un vertice labeled en bas gauche ou droite
            if( mesh.data(vh).label == puzzle.BG  || mesh.data(vh).label == puzzle.BD || mesh.data(vh).label == puzzle.B ){
                QVector3D center = QVector3D( center_X, center_Y, mesh.point(vh)[2] );
                QVector3D original = QVector3D( mesh.point(vh)[0], mesh.point(vh)[1], mesh.point(vh)[2] );
                QVector3D dist = original - center;
                QVector3D target = center - dist;
                mesh.point(vh)[0] = target.x();
                mesh.point(vh)[1] = target.y();
                mesh.point(vh)[2] = target.z();
            }
        }

        //update puzzle state
        int temp = puzzle.BD;
        puzzle.BD = puzzle.BG;
        puzzle.BG = temp;
        resetTrajectoryChecking();
    }

    displayMesh(&mesh);
}

void MainWindow::on_pushButton_RD_clicked(){
    cout << "RD clicked" << endl;

    //affichage des trajectoires des pieces
    if(!puzzle.checkingD){
        removeTrajectory(&mesh);
        resetTrajectoryChecking();
        puzzle.checkingD = true;
        addTrajectory(&mesh);
        if(ui->checkBox_collisionsCheck->isChecked())
            checkingTrajectoryCollisions(&mesh);
        trajectoryColorationUpdate(&mesh);
    }

    else if(puzzle.checkingD){
        removeTrajectory(&mesh);
        for(int i=0 ; i<(int)mesh.n_vertices() ; i++){
            VertexHandle vh = mesh.vertex_handle(i);
            //si on a un vertice labele en bas gauche
            if( mesh.data(vh).label == puzzle.HD  || mesh.data(vh).label == puzzle.BD || mesh.data(vh).label == puzzle.D ){
                QVector3D center = QVector3D( mesh.point(vh)[0], center_Y, center_Z );
                QVector3D original = QVector3D( mesh.point(vh)[0], mesh.point(vh)[1], mesh.point(vh)[2] );
                QVector3D dist = original - center;
                QVector3D target = center - dist;
                mesh.point(vh)[0] = target.x();
                mesh.point(vh)[1] = target.y();
                mesh.point(vh)[2] = target.z();
            }
        }

        //update puzzle state
        int temp = puzzle.BD;
        puzzle.BD = puzzle.HD;
        puzzle.HD = temp;
        resetTrajectoryChecking();
    }

    displayMesh(&mesh);
}

void MainWindow::on_pushButton_RH_clicked(){
    cout << "RH clicked" << endl;

    //affichage des trajectoires des pieces
    if(!puzzle.checkingH){
        removeTrajectory(&mesh);
        resetTrajectoryChecking();
        puzzle.checkingH = true;
        addTrajectory(&mesh);
        if(ui->checkBox_collisionsCheck->isChecked())
            checkingTrajectoryCollisions(&mesh);
        trajectoryColorationUpdate(&mesh);
    }

    else if(puzzle.checkingH){
        removeTrajectory(&mesh);
        for(int i=0 ; i<(int)mesh.n_vertices() ; i++){
            VertexHandle vh = mesh.vertex_handle(i);
            //si on a un vertice labele en bas gauche
            if( mesh.data(vh).label == puzzle.HG  || mesh.data(vh).label == puzzle.HD || mesh.data(vh).label == puzzle.H ){
                QVector3D center = QVector3D( center_X, center_Y, mesh.point(vh)[2] );
                QVector3D original = QVector3D( mesh.point(vh)[0], mesh.point(vh)[1], mesh.point(vh)[2] );
                QVector3D dist = original - center;
                QVector3D target = center - dist;
                mesh.point(vh)[0] = target.x();
                mesh.point(vh)[1] = target.y();
                mesh.point(vh)[2] = target.z();
            }
        }

        //update puzzle state
        int temp = puzzle.HD;
        puzzle.HD = puzzle.HG;
        puzzle.HG = temp;
        resetTrajectoryChecking();
    }

    displayMesh(&mesh);
}

void MainWindow::on_pushButton_RG_clicked(){
    cout << "RG clicked" << endl;

    //affichage des trajectoires des pieces
    if(!puzzle.checkingG){
        removeTrajectory(&mesh);
        resetTrajectoryChecking();
        puzzle.checkingG = true;
        addTrajectory(&mesh);
        if(ui->checkBox_collisionsCheck->isChecked())
            checkingTrajectoryCollisions(&mesh);
        trajectoryColorationUpdate(&mesh);
    }

    else if(puzzle.checkingG){
        removeTrajectory(&mesh);
        for(int i=0 ; i<(int)mesh.n_vertices() ; i++){
            VertexHandle vh = mesh.vertex_handle(i);
            //si on a un vertice labele en bas gauche
            if( mesh.data(vh).label == puzzle.BG  || mesh.data(vh).label == puzzle.HG ){
                QVector3D center = QVector3D( mesh.point(vh)[0], center_Y, center_Z );
                QVector3D original = QVector3D( mesh.point(vh)[0], mesh.point(vh)[1], mesh.point(vh)[2] );
                QVector3D dist = original - center;
                QVector3D target = center - dist;
                mesh.point(vh)[0] = target.x();
                mesh.point(vh)[1] = target.y();
                mesh.point(vh)[2] = target.z();
            }
        }

        //update puzzle state
        int temp = puzzle.BG;
        puzzle.BG = puzzle.HG;
        puzzle.HG = temp;
        resetTrajectoryChecking();
    }

    displayMesh(&mesh);
}




/* **** fin de la partie boutons et IHM **** */

// permet d'initialiser les couleurs et les épaisseurs des élements du maillage
void MainWindow::resetAllColorsAndThickness(MyMesh* _mesh){
    for (MyMesh::VertexIter curVert = _mesh->vertices_begin(); curVert != _mesh->vertices_end(); curVert++)
    {
        _mesh->data(*curVert).thickness = 1;
        _mesh->set_color(*curVert, MyMesh::Color(0, 0, 0));
    }

    for (MyMesh::FaceIter curFace = _mesh->faces_begin(); curFace != _mesh->faces_end(); curFace++)
    {
        _mesh->set_color(*curFace, MyMesh::Color(150, 150, 150));
    }

    for (MyMesh::EdgeIter curEdge = _mesh->edges_begin(); curEdge != _mesh->edges_end(); curEdge++)
    {
        _mesh->data(*curEdge).thickness = 1;
        _mesh->set_color(*curEdge, MyMesh::Color(0, 0, 0));
    }
}

// charge un objet MyMesh dans l'environnement OpenGL
void MainWindow::displayMesh(MyMesh* _mesh, DisplayMode mode){
    GLuint* triIndiceArray = new GLuint[_mesh->n_faces() * 3];
    GLfloat* triCols = new GLfloat[_mesh->n_faces() * 3 * 3];
    GLfloat* triVerts = new GLfloat[_mesh->n_faces() * 3 * 3];

    int i = 0;

    if(mode == DisplayMode::TemperatureMap)
    {
        QVector<float> values;
        for (MyMesh::VertexIter curVert = _mesh->vertices_begin(); curVert != _mesh->vertices_end(); curVert++)
            values.append(fabs(_mesh->data(*curVert).value));
        qSort(values);

        float range = values.at(values.size()*0.8);

        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;

        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            if(_mesh->data(*fvIt).value > 0){triCols[3*i+0] = 255; triCols[3*i+1] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+2] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            else{triCols[3*i+2] = 255; triCols[3*i+1] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+0] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            if(_mesh->data(*fvIt).value > 0){triCols[3*i+0] = 255; triCols[3*i+1] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+2] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            else{triCols[3*i+2] = 255; triCols[3*i+1] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+0] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            if(_mesh->data(*fvIt).value > 0){triCols[3*i+0] = 255; triCols[3*i+1] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+2] = 255 - std::min((_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            else{triCols[3*i+2] = 255; triCols[3*i+1] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0); triCols[3*i+0] = 255 - std::min((-_mesh->data(*fvIt).value/range) * 255.0, 255.0);}
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }

    if(mode == DisplayMode::Normal)
    {
        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            triCols[3*i+0] = _mesh->color(*fIt)[0]; triCols[3*i+1] = _mesh->color(*fIt)[1]; triCols[3*i+2] = _mesh->color(*fIt)[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->color(*fIt)[0]; triCols[3*i+1] = _mesh->color(*fIt)[1]; triCols[3*i+2] = _mesh->color(*fIt)[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->color(*fIt)[0]; triCols[3*i+1] = _mesh->color(*fIt)[1]; triCols[3*i+2] = _mesh->color(*fIt)[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }

    if(mode == DisplayMode::ColorShading)
    {
        MyMesh::ConstFaceIter fIt(_mesh->faces_begin()), fEnd(_mesh->faces_end());
        MyMesh::ConstFaceVertexIter fvIt;
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = _mesh->cfv_iter(*fIt);
            triCols[3*i+0] = _mesh->data(*fvIt).faceShadingColor[0]; triCols[3*i+1] = _mesh->data(*fvIt).faceShadingColor[1]; triCols[3*i+2] = _mesh->data(*fvIt).faceShadingColor[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->data(*fvIt).faceShadingColor[0]; triCols[3*i+1] = _mesh->data(*fvIt).faceShadingColor[1]; triCols[3*i+2] = _mesh->data(*fvIt).faceShadingColor[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++; ++fvIt;
            triCols[3*i+0] = _mesh->data(*fvIt).faceShadingColor[0]; triCols[3*i+1] = _mesh->data(*fvIt).faceShadingColor[1]; triCols[3*i+2] = _mesh->data(*fvIt).faceShadingColor[2];
            triVerts[3*i+0] = _mesh->point(*fvIt)[0]; triVerts[3*i+1] = _mesh->point(*fvIt)[1]; triVerts[3*i+2] = _mesh->point(*fvIt)[2];
            triIndiceArray[i] = i;

            i++;
        }
    }


    ui->displayWidget->loadMesh(triVerts, triCols, _mesh->n_faces() * 3 * 3, triIndiceArray, _mesh->n_faces() * 3);

    delete[] triIndiceArray;
    delete[] triCols;
    delete[] triVerts;

    GLuint* linesIndiceArray = new GLuint[_mesh->n_edges() * 2];
    GLfloat* linesCols = new GLfloat[_mesh->n_edges() * 2 * 3];
    GLfloat* linesVerts = new GLfloat[_mesh->n_edges() * 2 * 3];

    i = 0;
    QHash<float, QList<int> > edgesIDbyThickness;
    for (MyMesh::EdgeIter eit = _mesh->edges_begin(); eit != _mesh->edges_end(); ++eit)
    {
        float t = _mesh->data(*eit).thickness;
        if(t > 0)
        {
            if(!edgesIDbyThickness.contains(t))
                edgesIDbyThickness[t] = QList<int>();
            edgesIDbyThickness[t].append((*eit).idx());
        }
    }
    QHashIterator<float, QList<int> > it(edgesIDbyThickness);
    QList<QPair<float, int> > edgeSizes;
    while (it.hasNext())
    {
        it.next();

        for(int e = 0; e < it.value().size(); e++)
        {
            int eidx = it.value().at(e);

            MyMesh::VertexHandle vh1 = _mesh->to_vertex_handle(_mesh->halfedge_handle(_mesh->edge_handle(eidx), 0));
            linesVerts[3*i+0] = _mesh->point(vh1)[0];
            linesVerts[3*i+1] = _mesh->point(vh1)[1];
            linesVerts[3*i+2] = _mesh->point(vh1)[2];
            linesCols[3*i+0] = _mesh->color(_mesh->edge_handle(eidx))[0];
            linesCols[3*i+1] = _mesh->color(_mesh->edge_handle(eidx))[1];
            linesCols[3*i+2] = _mesh->color(_mesh->edge_handle(eidx))[2];
            linesIndiceArray[i] = i;
            i++;

            MyMesh::VertexHandle vh2 = _mesh->from_vertex_handle(_mesh->halfedge_handle(_mesh->edge_handle(eidx), 0));
            linesVerts[3*i+0] = _mesh->point(vh2)[0];
            linesVerts[3*i+1] = _mesh->point(vh2)[1];
            linesVerts[3*i+2] = _mesh->point(vh2)[2];
            linesCols[3*i+0] = _mesh->color(_mesh->edge_handle(eidx))[0];
            linesCols[3*i+1] = _mesh->color(_mesh->edge_handle(eidx))[1];
            linesCols[3*i+2] = _mesh->color(_mesh->edge_handle(eidx))[2];
            linesIndiceArray[i] = i;
            i++;
        }
        edgeSizes.append(qMakePair(it.key(), it.value().size()));
    }

    ui->displayWidget->loadLines(linesVerts, linesCols, i * 3, linesIndiceArray, i, edgeSizes);

    delete[] linesIndiceArray;
    delete[] linesCols;
    delete[] linesVerts;

    GLuint* pointsIndiceArray = new GLuint[_mesh->n_vertices()];
    GLfloat* pointsCols = new GLfloat[_mesh->n_vertices() * 3];
    GLfloat* pointsVerts = new GLfloat[_mesh->n_vertices() * 3];

    i = 0;
    QHash<float, QList<int> > vertsIDbyThickness;
    for (MyMesh::VertexIter vit = _mesh->vertices_begin(); vit != _mesh->vertices_end(); ++vit)
    {
        float t = _mesh->data(*vit).thickness;
        if(t > 0)
        {
            if(!vertsIDbyThickness.contains(t))
                vertsIDbyThickness[t] = QList<int>();
            vertsIDbyThickness[t].append((*vit).idx());
        }
    }
    QHashIterator<float, QList<int> > vitt(vertsIDbyThickness);
    QList<QPair<float, int> > vertsSizes;

    while (vitt.hasNext())
    {
        vitt.next();

        for(int v = 0; v < vitt.value().size(); v++)
        {
            int vidx = vitt.value().at(v);

            pointsVerts[3*i+0] = _mesh->point(_mesh->vertex_handle(vidx))[0];
            pointsVerts[3*i+1] = _mesh->point(_mesh->vertex_handle(vidx))[1];
            pointsVerts[3*i+2] = _mesh->point(_mesh->vertex_handle(vidx))[2];
            pointsCols[3*i+0] = _mesh->color(_mesh->vertex_handle(vidx))[0];
            pointsCols[3*i+1] = _mesh->color(_mesh->vertex_handle(vidx))[1];
            pointsCols[3*i+2] = _mesh->color(_mesh->vertex_handle(vidx))[2];
            pointsIndiceArray[i] = i;
            i++;
        }
        vertsSizes.append(qMakePair(vitt.key(), vitt.value().size()));
    }

    ui->displayWidget->loadPoints(pointsVerts, pointsCols, i * 3, pointsIndiceArray, i, vertsSizes);

    delete[] pointsIndiceArray;
    delete[] pointsCols;
    delete[] pointsVerts;
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}


/*
//check intersection au niveau de l'axe X
if((A[0] <= plan.xA && B[0] >= plan.xA) || (B[0] <= plan.xA && A[0] >= plan.xA) ||
   (A[0] <= plan.xB && B[0] >= plan.xB) || (B[0] <= plan.xB && A[0] >= plan.xB) ||
   (A[0] <= plan.xA && C[0] >= plan.xA) || (C[0] <= plan.xA && A[0] >= plan.xA) ||
   (A[0] <= plan.xB && C[0] >= plan.xB) || (C[0] <= plan.xB && A[0] >= plan.xB) ||
   (B[0] <= plan.xA && C[0] >= plan.xA) || (C[0] <= plan.xA && B[0] >= plan.xA) ||
   (B[0] <= plan.xB && C[0] >= plan.xB) || (C[0] <= plan.xB && B[0] >= plan.xB))
    return true;

//check intersection au niveau de l'axe Z
if((A[2] <= plan.zA && B[2] >= plan.zA) || (B[2] <= plan.zA && A[2] >= plan.zA) ||
   (A[2] <= plan.zB && B[2] >= plan.zB) || (B[2] <= plan.zB && A[2] >= plan.zB) ||
   (A[2] <= plan.zA && C[2] >= plan.zA) || (C[2] <= plan.zA && A[2] >= plan.zA) ||
   (A[2] <= plan.zB && C[2] >= plan.zB) || (C[2] <= plan.zB && A[2] >= plan.zB) ||
   (B[2] <= plan.zA && C[2] >= plan.zA) || (C[2] <= plan.zA && B[2] >= plan.zA) ||
   (B[2] <= plan.zB && C[2] >= plan.zB) || (C[2] <= plan.zB && B[2] >= plan.zB))
    return true;
 */


//on veut eviter d'avoir un ensemble non connexe
/*void MainWindow::reflagging(MyMesh *_mesh, CoorPieces &p){
    //pour rendre la vérification plus facile, on fait un tableau de flags de la taille
    //du nombre de faces de la mesh, de façon à savoir facilement si une face à été traitée ou non
    vector<bool> isToProcess;
    for(long unsigned int i=0 ; i<_mesh->n_faces() ; i++) isToProcess.push_back(false);
    //on marque les faces de notre selection à traiter
    for(long unsigned int i=0 ; i<p.faces.size() ; i++) isToProcess.at(p.faces.at(i)) = true;

    vector<vector<int>> subPieces;
    for(long unsigned int i=0 ; i<_mesh->n_faces() ; i++){
        if(isToProcess.at(i) == true){
            vector<int> currentSubPiece;
            recursiveLookForPiece(_mesh, isToProcess, currentSubPiece, i);
            subPieces.push_back(currentSubPiece);
        }
    }

    cout << "on a une selection de " << subPieces.size() << " ensembles de taille :" << endl;
    for(long unsigned int i=0 ; i<subPieces.size() ; i++) cout << subPieces.at(i).size() << endl;
    cout << "on garde la sous piece la plus grande" << endl;
    int max_size = 0;
    int indice = -1;
    for(long unsigned int i=0 ; i<subPieces.size() ; i++){
        if(subPieces.at(i).size() > max_size){
            max_size = subPieces.at(i).size();
            indice = i;
            cout << "assigning ..." << endl;
        }
    }
    if(indice == -1) cout << "/!/ erreur lors de recherche de plus grand sous ensemble" << endl;
    p.faces.clear();
    for(long unsigned int i=0 ; i<subPieces.at(indice).size() ; i++){
        p.faces.push_back(subPieces.at(indice).at(i));
    }
}

void MainWindow::recursiveLookForPiece(MyMesh *_mesh, vector<bool> &isToProcess, vector<int> &subPiece, int currentFace){
    //on marque comme processed
    isToProcess.at(currentFace) = false;
    //on note la face dans la sous-piece courante
    subPiece.push_back(currentFace);
    FaceHandle fh = _mesh->face_handle(currentFace);
    MyMesh::FaceFaceIter ff_it = _mesh->ff_iter(fh);
    for(ff_it ; ff_it.is_valid() ; ++ff_it){
        int f_voisin_ID = (*ff_it).idx();
        if(isToProcess.at(f_voisin_ID)){
            recursiveLookForPiece(_mesh, isToProcess, subPiece, f_voisin_ID);
        }
    }
}*/

/*
FaceHandle fh = mesh.face_handle(i);
MyMesh::FaceVertexIter fv_it = mesh.fv_iter(fh);
MyMesh::Point A = mesh.point(mesh.vertex_handle((*fv_it).idx()));
++fv_it;
MyMesh::Point B = mesh.point(mesh.vertex_handle((*fv_it).idx()));
++fv_it;
MyMesh::Point C = mesh.point(mesh.vertex_handle((*fv_it).idx()));

//verification si on se trouve à la bonne hauteur de l'axe Y (ex probleme des oreilles lapin)
if((A[1] > corps.maxY || A[1] < corps.minY) ||
   (B[1] > corps.maxY || B[1] < corps.minY) ||
   (C[1] > corps.maxY || C[1] < corps.minY)){

    continue;
    //cout << "continue " << endl;
}

//vérification de coupure des plans pour AB générés par le corps
if((A[0] < corps.xBG && B[0] > corps.xBG) || (A[0] < corps.xBG && B[0] > corps.xBG) ||
   (A[0] < corps.xBD && B[0] > corps.xBD) || (A[0] < corps.xBD && B[0] > corps.xBD) ||
   (A[2] < corps.zHG && B[2] > corps.zHG) || (A[2] < corps.zHG && B[2] > corps.zHG) ||
   (A[2] < corps.zBG && B[2] > corps.zBG) || (A[2] < corps.zBG && B[2] > corps.zBG)){
   //marquer a delete
   //cout << "marked to delete" << endl;
   mesh.delete_face(fh, false);
   continue;
}
//vérification de coupure des plans pour AC générés par le corps
if((A[0] < corps.xBG && C[0] > corps.xBG) || (A[0] < corps.xBG && B[0] > corps.xBG) ||
   (A[0] < corps.xBD && C[0] > corps.xBD) || (A[0] < corps.xBD && B[0] > corps.xBD) ||
   (A[2] < corps.zHG && C[2] > corps.zHG) || (A[2] < corps.zHG && B[2] > corps.zHG) ||
   (A[2] < corps.zBG && C[2] > corps.zBG) || (A[2] < corps.zBG && B[2] > corps.zBG)){
    //marquer a delete
    //cout << "marked to delete" << endl;
    mesh.delete_face(fh, false);
    continue;
}
//vérification de coupure des plans pour BC générés par le corps
if((C[0] < corps.xBG && B[0] > corps.xBG) || (C[0] < corps.xBG && B[0] > corps.xBG) ||
   (C[0] < corps.xBD && B[0] > corps.xBD) || (C[0] < corps.xBD && B[0] > corps.xBD) ||
   (C[2] < corps.zHG && B[2] > corps.zHG) || (C[2] < corps.zHG && B[2] > corps.zHG) ||
   (C[2] < corps.zBG && B[2] > corps.zBG) || (C[2] < corps.zBG && B[2] > corps.zBG)){
    //marquer a delete
    //cout << "marked to delete" << endl;
    mesh.delete_face(fh, false);
    continue;
}
*/


//pour chaque edge, verifier si le plan intersecte et identifier le côté avec le edge
//et le côté avec le vertex et les edges coupés
//pour chaque edge coupee, creer deux points à l'intersection
//pour l'un creer une face avec l'edge, pour l'autre avec le vertex et l'un des autres points
//generes a la coupure
/*void MainWindow::generateIntersectVertexFaces(MyMesh *_mesh, int faceID, Plan p){
    FaceHandle fh = _mesh->face_handle(faceID);

    // -- partie 1, identification des edges
    int edge_not_cut, first_edge_cut, second_edge_cut;

    MyMesh::FaceEdgeIter fe_it = _mesh->fe_iter(fh);
    if(edgeIsCutByPlan(_mesh, (*fe_it).idx(), p) == false){
        edge_not_cut = (*fe_it).idx();
        ++fe_it;
        first_edge_cut = (*fe_it).idx();
        ++fe_it;
        second_edge_cut = (*fe_it).idx();
    }
    else{
        int tmp = (*fe_it).idx();
        ++fe_it;
        if(edgeIsCutByPlan(_mesh, (*fe_it).idx(), p)){
            first_edge_cut = tmp;
            second_edge_cut = (*fe_it).idx();
            ++fe_it;
            edge_not_cut = (*fe_it).idx();
        }
        else{
            edge_not_cut = (*fe_it).idx();
            ++fe_it;
            first_edge_cut = (*fe_it).idx();
            second_edge_cut = tmp;
        }
    }

    // -- partie 2, identification du vertex commun aux edges cut
    int vertex_commun;

    EdgeHandle eh = _mesh->edge_handle(edge_not_cut);

    HalfedgeHandle ah = _mesh->halfedge_handle(eh, 0);
    VertexHandle v1 = _mesh->from_vertex_handle(ah);
    VertexHandle v2 = _mesh->to_vertex_handle(ah);
    //int a = (*v1).idx();
    int b = (*v2).idx();

    MyMesh::FaceVertexIter fv_it = _mesh->fv_iter(fh);
    for( ; fv_it.is_valid() ; ++fv_it){
        if( (*fv_it).idx() != a && (*fv_it).idx() != b ){
            vertex_commun = (*fv_it).idx();
            break;
        }
    }

    // -- partie 3, creation de points
    eh = _mesh->edge_handle(first_edge_cut);
    ah = _mesh->halfedge_handle(eh, 0);
    v1 = _mesh->from_vertex_handle(ah);r
}

void MainWindow::on_pushButton_split_decoupe_clicked(){
    //request for delete
    mesh.request_vertex_status();
    mesh.request_edge_status();
    mesh.request_face_status();

    //droite
    vector<VertexHandle> addedVertices;

    //parcour de tout les edges et split ceux qui sont sur le plan de droite
    MyMesh::EdgeIter curEdge = mesh.edges_begin();
    for( ; curEdge != mesh.edges_end() ; ++curEdge){
        if( edgeIsCutByPlan(&mesh, (*curEdge).idx(), p_droite) ){

            QVector3D planeNormal = QVector3D(1.0, 0.0, 0.0);
            QVector3D planePoint = QVector3D(p_droite.coor, 0.0, 0.0);

            //QVector3D insertedVertex = intersection_calculation(planePoint, planeNormal, a, b);
            QVector3D insertedVertex = edge_plan_intersection(&mesh, curEdge, planePoint, planeNormal);
            VertexHandle ivh = mesh.add_vertex( MyMesh::Point(insertedVertex.x(), insertedVertex.y(), insertedVertex.z()) );
            addedVertices.push_back(ivh);

            mesh.split(curEdge, ivh);
        }
    }



    //dédoublement et reatribution des faces droites et gauches
    for(int i=0 ; i<(int)addedVertices.size() ; i++){

        VertexHandle duplicate = mesh.add_vertex( mesh.point(addedVertices.at(i)) );

        MyMesh::VertexFaceIter vf_it = mesh.vf_iter(addedVertices.at(i));
        for( ; vf_it.is_valid() ; ++vf_it){
            QVector3D centerOfFace = getCenterOfFace(&mesh, (*vf_it).idx());
            //si la face est a droite
            if( centerOfFace.x() > p_droite.coor ){

                //pour tous les vertex de la face on recupere les deux vertices appartenant
                //à la future face (face de droite avec le duplicate)
                vector<VertexHandle> new_face_vertices;
                MyMesh::FaceVertexIter fv_it = mesh.fv_iter(vf_it);
                for( ; fv_it.is_valid() ; ++fv_it){
                    //si on parcoure l'ancien vertice, on mets le duplicate à la place
                    if( (*fv_it).idx() == addedVertices.at(i).idx() )
                        new_face_vertices.push_back(duplicate);
                    //sinon on add le vertice au vector (de façon a garder un ordre correct)
                    else new_face_vertices.push_back(fv_it);
                }

                //ajout de la nouvelle face de droite
                mesh.add_face( new_face_vertices );
            }
        }
    }

    //dernier passage pour suppression des faces originales

    for(int i=0 ; i<(int)addedVertices.size() ; i++){
        MyMesh::VertexFaceIter vf_it = mesh.vf_iter(addedVertices.at(i));
        for( ; vf_it.is_valid() ; ++vf_it){
            QVector3D centerOfFace = getCenterOfFace(&mesh, (*vf_it).idx());
            if( centerOfFace.x() > p_droite.coor ){
                mesh.delete_face(mesh.face_handle(vf_it));
            }
        }
    }

    //delete all elements marked as deleted
    mesh.garbage_collection();



    isCut = true;
    piecesFlaggingAndColoration(&mesh);

    displayMesh(&mesh);
}


void MainWindow::smoothingCutBorder(MyMesh *_mesh, Plan plan){

    //step 1: recuperation de la bordure
    if(plan.orientation == "vertical"){

        // -- droite
        vector<int> v_bord_droite;

        //recuperation du premier point
        for(int i=0 ; i<(int)_mesh->n_vertices() ; i++){
            VertexHandle vh = _mesh->vertex_handle(i);
            MyMesh::Point v = _mesh->point(vh);
            //si on est à droite
            if(v[0] > plan.coor){
                //si on est en bordure (check avec les edges si on en a un en bordure)
                bool is_border = false;
                MyMesh::VertexEdgeIter ve_it = _mesh->ve_iter(vh);
                for( ; ve_it.is_valid() ; ++ve_it){
                    if(_mesh->is_boundary(ve_it)){
                        is_border = true;
                        break;
                    }
                }
                //si on a trouvé le premier point on l'enregistre et arrête de chercher
                if(is_border){
                    _mesh->set_color(vh, MyMesh::Color(200, 100, 100));
                    _mesh->data(vh).thickness = 10;
                    v_bord_droite.push_back(i);
                    break;
                }
            }
        }

        //cout << "first element found : " << v_bord_droite.size() << endl;

        //parcour de la suite des elements
        int current_vertex = v_bord_droite.at(0);

        while(true){

            VertexHandle vh = _mesh->vertex_handle(current_vertex);
            //pour chercher le prochain point, on souhaite en premier recuperer le deuxieme edge
            //il faut que celui ci soit directement le prochain
            int secondEdgeIndex;
            bool first_border_edge_found = false;

            MyMesh::VertexEdgeIter ve_it = _mesh->ve_iter(vh);
            vector<int> edges_neighbour;
            for( ; ve_it.is_valid() ; ++ve_it){
                if(_mesh->is_boundary(ve_it)){
                    if(!first_border_edge_found) first_border_edge_found = true;
                    else{
                        secondEdgeIndex = (*ve_it).idx();
                        break;
                    }
                }
            }

            //recuperation du prochain point avec l'edge
            int nextVertexIndex;
            EdgeHandle eh = _mesh->edge_handle(secondEdgeIndex);
            HalfedgeHandle ah = _mesh->halfedge_handle(eh, 0);

            int from_index = _mesh->from_vertex_handle(ah).idx();
            int to_index = _mesh->to_vertex_handle(ah).idx();

            if( current_vertex != from_index )
                nextVertexIndex = from_index;
            else nextVertexIndex = to_index;

            //check si on a fait le tour complet
            if(v_bord_droite.back() != nextVertexIndex )
                v_bord_droite.push_back( nextVertexIndex );
            else{
                cout << "contour bouclé" << endl;
                break;
            }
        }

        cout << "nombre de points dans la bordure : " << v_bord_droite.size() << endl;

        //test show border
        for(int i=0 ; i<(int)v_bord_droite.size() ; i++){
            VertexHandle vh = _mesh->vertex_handle(i);
            _mesh->set_color(vh, MyMesh::Color(200, 100, 100));
            _mesh->data(vh).thickness = 10;
        }

        // -- gauche
    }
    //step 2: création des points sur le plan
    //step 3: couture
}
*/
