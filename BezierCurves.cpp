#include <GL/glut.h>
#include <GL/gl.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

const float zPlane = -1.0f;
const float rPoint = 0.025f;
const float borderThickness = rPoint/3.0f;

int windowWidth = 800;
int windowHeight = 800;

std::string OBJfilePath = "./OBJfile.obj";

float mouseX = 0.0;
float mouseY = 0.0;

std::vector<glm::vec2> control_Points;
std::vector<bool> point_Hovered;

std::vector<glm::vec2> bezierPoints;

std::vector<glm::vec2> rotationAxes;

std::vector<std::vector<glm::vec3>> surfaceOfRotation;

std::vector<std::vector<int>> facesOfSurfaceOfRotation;

int curSelectedPoint;

float paraStep = 0.01f;

void toRGB(int r, int g, int b){
    glColor3f((float)r/255.0, (float)g/255.0, (float)b/255.0);
}

void drawCircle(float x, float y, bool isHovered){
    float theta = 0.0f;
    float thetaStep = 0.1f;

    float prevX = x + rPoint;
    float prevY = y;

    while(theta < 2*3.24){
        float newX = x + rPoint * std::cos(theta);
        float newY = y + rPoint * std::sin(theta);

        glVertex3f(x, y, zPlane);
        glVertex3f(prevX, prevY, zPlane);
        glVertex3f(newX, newY, zPlane);

        prevX = newX;
        prevY = newY;

        theta += thetaStep;
    }

    if(!isHovered)
        return;

    theta = 0.0f;

    prevX = x + rPoint;
    prevY = y;

    float prevXFurther = x + rPoint + borderThickness;
    float prevYFurther = y;

    toRGB(255, 34, 0);

    while(theta < 2*3.24){
        float newX = x + rPoint * std::cos(theta);
        float newY = y + rPoint * std::sin(theta);

        float newXFurther = x + (rPoint + borderThickness) * std::cos(theta);
        float newYFurther = y + (rPoint + borderThickness) * std::sin(theta);

        glVertex3f(prevX, prevY, zPlane);
        glVertex3f(prevXFurther, prevYFurther, zPlane);
        glVertex3f(newX, newY, zPlane);

        glVertex3f(newX, newY, zPlane);
        glVertex3f(newXFurther, newYFurther, zPlane);
        glVertex3f(prevXFurther, prevYFurther, zPlane);

        prevX = newX;
        prevY = newY;

        prevXFurther = newXFurther;
        prevYFurther = newYFurther;

        theta += thetaStep;
    }
}

float mulFromTo(int from, int to){
    if(from == to){
        return to;
    }

    if(from < to){
        return -1;
    }

    float ret = 1;

    for(int i = from; i >= to; i--){
        ret *= i;
    }

    return ret;
}

float nChooseI(int n, int i){
    if(i > n - i){
        return mulFromTo(n, i+1)/mulFromTo(n-i, 1);
    }else{
        return mulFromTo(n, n-i+1)/mulFromTo(i, 1);
    }

    return 1;
}

void makeBezier(float tStep){
    bezierPoints.clear();
    if(control_Points.size() < 2)
        return;
    
    float t = 0;

    int n = control_Points.size() - 1;

    while(t <= 1){
        float curX = 0;
        float curY = 0;
        for(int i = 0; i < control_Points.size(); i++){
            float nCi = nChooseI(n, i);
            float coeff = nCi * (float)std::pow(1-t, n-i) * (float)std::pow(t, i);
            
            // std::cout << n << "C" << i/2 << " = " << nCi << "\n";
            // std::cout << "Point: " << i/2 << " Coeff: " << coeff << "\n";


            curX += coeff * control_Points.at(i).x;
            curY += coeff * control_Points.at(i).y;
        }

        bezierPoints.push_back(glm::vec2(curX, curY));

        // std::cout << t << ": " << curX << ", " << curY << "\n==========================\n";
        t+=tStep;
    }

    if(tStep == 0.1f){
        float curX = 0;
        float curY = 0;
        for(int i = 0; i < control_Points.size(); i++){
            float nCi = nChooseI(n, i);
            float coeff = nCi * (float)std::pow(1-t, n-i) * (float)std::pow(t, i);
            
            // std::cout << n << "C" << i/2 << " = " << nCi << "\n";
            // std::cout << "Point: " << i/2 << " Coeff: " << coeff << "\n";


            curX += coeff * control_Points.at(i).x;
            curY += coeff * control_Points.at(i).y;
        }

        bezierPoints.push_back(glm::vec2(curX, curY));
    }
}

void rotatePointsAboutLine(float thetaFinal, float thetaStep){
    if(rotationAxes.size() != 2 || thetaStep == 0.0f)
        return;

    surfaceOfRotation.clear(); // Clears a preexisting surface of rotation
    
    glm::vec2 vecOfRotation = rotationAxes.at(1) - rotationAxes.at(0);
    vecOfRotation = glm::normalize(vecOfRotation);

    // std::cout << "cp_0" <<  glm::to_string(control_Points.at(0)) << "\n";
    // std::cout << "rotAx_0" <<  glm::to_string(rotationAxes.at(0)) << "\n";

    makeBezier(0.1f);

    float theta = 0.0f;
    for(;theta < thetaFinal; theta+=thetaStep){
        std::vector<glm::vec3> thisEdge;

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(rotationAxes.at(0), 0));
        transform = glm::rotate(transform, glm::radians(theta), glm::vec3(vecOfRotation, 0));
        transform = glm::translate(transform, glm::vec3(-rotationAxes.at(0), 0));

        for(glm::vec2 bezierPoint: bezierPoints){
            glm::vec4 output = transform * glm::vec4(bezierPoint, 0.0, 1.0);
            thisEdge.push_back(glm::vec3(output.x, output.y, output.z));

            // std::cout << glm::to_string(output) << "\n";
        }

        surfaceOfRotation.push_back(thisEdge);
    }

    // std::cout << "NUmber of edges in surface of rotation: " << surfaceOfRotation.size() << "\n";
}

void makeFacesOfSurfaceOfRotation(){
    if(surfaceOfRotation.size() < 2)
        return;
    
    int edgeSize = surfaceOfRotation.at(0).size();

    for(int i = 1; i < surfaceOfRotation.size(); i++){
        for(int j = 0; j < edgeSize - 1; j++){
            std::vector<int> face0;
            std::vector<int> face1;

            face0.push_back(i*edgeSize + j);
            face0.push_back(i*edgeSize + j + 1);
            face0.push_back((i-1)*edgeSize + j);        // Creates top triangle

            face1.push_back((i-1)*edgeSize + j);
            face1.push_back(i*edgeSize + j + 1);
            face1.push_back((i-1)*edgeSize + j + 1);    // Creates Bottom Triangle

            facesOfSurfaceOfRotation.push_back(face0);
            facesOfSurfaceOfRotation.push_back(face1);
        }
    }

    int lastEdge = surfaceOfRotation.size() - 1;

    for(int j = 0; j < edgeSize - 1; j++){
            std::vector<int> face0;
            std::vector<int> face1;

            face0.push_back(lastEdge*edgeSize + j);
            face0.push_back(lastEdge*edgeSize + j + 1);
            face0.push_back(j);        // Creates top triangle

            face1.push_back(j);
            face1.push_back(lastEdge*edgeSize + j + 1);
            face1.push_back(j + 1);    // Creates Bottom Triangle

            facesOfSurfaceOfRotation.push_back(face0);
            facesOfSurfaceOfRotation.push_back(face1);
        }

    // for(std::vector<int> face: facesOfSurfaceOfRotation){
    //     std::cout << face.at(0) << ", " << face.at(1) << ", " << face.at(2) << "\n";
    // }
}

void writeToOBJFile(std::string filename){
    std::ofstream objFile(filename, std::ios::out | std::ios::trunc);
    if (!objFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // Write vertices
    for (std::vector<glm::vec3> vertices : surfaceOfRotation) {
        for(glm::vec3 vertex: vertices){
            objFile << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
        }
    }

    // Write faces
    for (std::vector<int> face: facesOfSurfaceOfRotation) {
        objFile << "f";
        for (int vertexIndex : face) {
            objFile << " " << vertexIndex + 1;  // OBJ format uses 1-based indexing
        }
        objFile << "\n";
    }

    objFile.close();
}

static void resize(int width, int height)
{
    const float ar = (float) width / (float) height;
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
    glOrtho(-1, 1, -1, 1, 0, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;

    // glutSwapBuffers();
}

static void display(void)
{
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    const double a = t*90.0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Control Points
    glBegin(GL_TRIANGLES);
        for(int i = 0; i < control_Points.size(); i++){
            toRGB(255, 115, 0);
            drawCircle(control_Points.at(i).x, control_Points.at(i).y, point_Hovered.at(i));
        }
    glEnd();

    // Lines between control Points
    toRGB(0, 87, 16);
    glBegin(GL_LINE_STRIP);
        for(int i = 0; i < control_Points.size(); i++){
            glVertex3f(control_Points.at(i).x, control_Points.at(i).y, zPlane);
        }
    glEnd();

    // Bezier Curve
    toRGB(58, 54, 168);
    glBegin(GL_LINE_STRIP);
        for(int i = 0; i < bezierPoints.size(); i++){
            glVertex3f(bezierPoints.at(i).x, bezierPoints.at(i).y, zPlane);
        }
    glEnd();

    // Rotation Axis
    if(rotationAxes.size() == 2){
        toRGB(255, 0, 0);
        glBegin(GL_LINES);
            glVertex3f(rotationAxes.at(0).x, rotationAxes.at(0).y, zPlane);
            glVertex3f(rotationAxes.at(1).x, rotationAxes.at(1).y, zPlane);
        glEnd();
    }
    
    glutSwapBuffers();
}

static void key(unsigned char key, int x, int y)
{
    // std::cout << (int)key;

    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;            
        case 'd':
        case 'D':
            if(curSelectedPoint != -1){
                control_Points.erase(control_Points.begin() + curSelectedPoint);
                point_Hovered.erase(point_Hovered.begin() + curSelectedPoint);

                makeBezier(paraStep);
            }
            break;
        // Make surface of rotation
        case 32:
            if(control_Points.size() > 0 && rotationAxes.size() == 2)
                rotatePointsAboutLine(360.0f, 20.f);
            break;
        case 13: // Space
            makeFacesOfSurfaceOfRotation();
            writeToOBJFile(OBJfilePath);
            break;
    }

    glutPostRedisplay();
}

static void idle(void)
{
    glutPostRedisplay();
}

static void motionCallback(int x, int y){
    mouseX = (float)x / windowWidth * 2.0 - 1.0;
    mouseY = 1.0 - (float)y / windowHeight * 2.0;

    bool flag = false;

    for(int i = 0; i < control_Points.size(); i++){
        float delX = control_Points.at(i).x - mouseX;
        float delY = control_Points.at(i).y - mouseY;

        point_Hovered.at(i) = std::pow(delX, 2) + std::pow(delY, 2) <= std::pow(rPoint, 2);

        if(point_Hovered.at(i)){
            curSelectedPoint = i;
            flag = true;
        }
    }

    if(!flag){
        curSelectedPoint = -1;
    }

    // std::cout << mouseX << ", " << mouseY << "\n";
    glutPostRedisplay();  
}

void mouseCallback(int button, int state, int x, int y){
    mouseX = (float)x / windowWidth * 2.0 - 1.0;
    mouseY = 1.0 - (float)y / windowHeight * 2.0;

    if (button == GLUT_LEFT_BUTTON ) {
        // No point selected must be new point
        if(curSelectedPoint == -1 && state == GLUT_DOWN){
            control_Points.push_back(glm::vec2(mouseX, mouseY));
            point_Hovered.push_back(false);
            makeBezier(paraStep);
        }

        if(curSelectedPoint == -1)
            return;

        if (state == GLUT_UP) {
            control_Points.at(curSelectedPoint).x = mouseX;
            control_Points.at(curSelectedPoint).y = mouseY;

            makeBezier(paraStep);
        }
    }else if(button == GLUT_RIGHT_BUTTON) {
        if(state == GLUT_DOWN)
            return;
        
        if(rotationAxes.size() == 2){
            // Clear Axis
            // std::cout << rotationAxes.at(0) << " " << rotationAxes.at(1) << "---------->" << rotationAxes.at(2) << " " << rotationAxes.at(3);
            rotationAxes.clear();
            return;
        }

        rotationAxes.push_back(glm::vec2(mouseX, mouseY));

    }
}

/* Program entry point */
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(windowWidth,windowHeight);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Bezier Curves");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);

    glutPassiveMotionFunc(motionCallback);
    glutMouseFunc(mouseCallback);  

    glutIdleFunc(idle);
    glClearColor(0.709f,0.917f,1,1);

    // control_Points.push_back(glm::vec2(0.5, 0.5));
    // point_Hovered.push_back(false);
    makeBezier(paraStep);

    // rotatePointsAboutLine(0.0f);

    glutMainLoop();

    return EXIT_SUCCESS;
}
