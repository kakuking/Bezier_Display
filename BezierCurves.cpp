#include <GL/glut.h>
#include <GL/gl.h>

#include <vector>
#include <iostream>
#include <math.h>
#include <stdlib.h>

const float zPlane = -3.0f;
const float rPoint = 0.025f;
const float borderThickness = rPoint/3.0f;

int windowWidth = 800;
int windowHeight = 800;

float mouseX = 0.0;
float mouseY = 0.0;

std::vector<float> control_Points;
std::vector<bool> point_Hovered;

std::vector<float> bezierPoints;

int curSelectedPoint;

float paraStep = 0.05f;

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

// to < from
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
    if(control_Points.size() < 4)
        return;
    
    float t = 0;
    bezierPoints.clear();

    int n = control_Points.size()/2 - 1;

    for( ; t <= 1 + tStep; t+=tStep){
        float curX = 0;
        float curY = 0;
        for(int i = 0; i < control_Points.size(); i+=2){
            float nCi = nChooseI(n, i/2);
            float coeff = nCi * (float)std::pow(1-t, n-i/2) * (float)std::pow(t, i/2);
            
            // std::cout << n << "C" << i/2 << " = " << nCi << "\n";
            // std::cout << "Point: " << i/2 << " Coeff: " << coeff << "\n";

            curX += coeff * control_Points.at(i);
            curY += coeff * control_Points.at(i+1);
        }

        bezierPoints.push_back(curX);
        bezierPoints.push_back(curY);

        // std::cout << t << ": " << curX << ", " << curY << "\n==========================\n";
    }

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
    glOrtho(-1, 1, -1, 1, 2, 100);

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
        for(int i = 0; i < control_Points.size(); i+=2){
            toRGB(255, 115, 0);
            drawCircle(control_Points.at(i), control_Points.at(i+1), point_Hovered.at(i/2));
        }
    glEnd();

    toRGB(0, 87, 16);
    glBegin(GL_LINE_STRIP);
        for(int i = 0; i < control_Points.size(); i+= 2){
            glVertex3f(control_Points.at(i), control_Points.at(i+1), zPlane);
        }
    glEnd();

    toRGB(58, 54, 168);
    glBegin(GL_LINE_STRIP);
        for(int i = 0; i < bezierPoints.size(); i+=2){
            glVertex3f(bezierPoints.at(i), bezierPoints.at(i+1), zPlane);
        }
    glEnd();
    
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
                control_Points.erase(control_Points.begin() + curSelectedPoint);
                point_Hovered.erase(point_Hovered.begin() + curSelectedPoint/2);

                makeBezier(paraStep);
            }
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

    for(int i = 0; i < control_Points.size(); i+=2){
        float curX = control_Points.at(i);
        float curY = control_Points.at(i+1);

        float delX = curX - mouseX;
        float delY = curY - mouseY;

        point_Hovered.at(i/2) = std::pow(delX, 2) + std::pow(delY, 2) <= std::pow(rPoint, 2);

        if(point_Hovered.at(i/2)){
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
            control_Points.push_back(mouseX);
            control_Points.push_back(mouseY);
            point_Hovered.push_back(false);
            makeBezier(paraStep);
        }

        if(curSelectedPoint == -1)
            return;

        if (state == GLUT_UP) {
            control_Points.at(curSelectedPoint) = mouseX;
            control_Points.at(curSelectedPoint + 1) = mouseY;

            makeBezier(paraStep);
        }

        
        
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

    glClearColor(1,1,1,1);
    
    control_Points.push_back(0.0);
    control_Points.push_back(0.0);
    point_Hovered.push_back(false);

    
    control_Points.push_back(0.5);
    control_Points.push_back(0.5);
    point_Hovered.push_back(false);

    control_Points.push_back(0.9);
    control_Points.push_back(0.0);
    point_Hovered.push_back(false);

    makeBezier(paraStep);

    glutMainLoop();

    return EXIT_SUCCESS;
}
