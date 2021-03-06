#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

const int WIDTH = 800;
const int HEIGHT = 600;

int win_handler;

class Map {
    public:
        int sqrSize;
        bool display_map;
        std::vector< std::vector<int> > map;
        void loadData(char*);
        void print();
        void draw();
        void toggleMap();
        int getXY(float,float);
        Map();
};

Map::Map(){
    sqrSize = 40;
    display_map = false;
}

void Map::loadData(char* path) {
    std::ifstream infile(path);
    std::string line;
    while (std::getline(infile, line)) {
        std::vector<int> l;
        for(auto c: line)
            l.push_back(atoi(&c));
        map.push_back(l);
    }
}

void Map::print() {
    for(auto l: map) {
        for(auto c: l)
            std::cout << c;
        std::cout << std::endl;
    }
}

void Map::draw() {
    if (!display_map) return;
    for(unsigned y=0; y<map.size(); y++) {
        for(unsigned x=0; x<map[y].size(); x++) {
            int x0=x*sqrSize,y0=y*sqrSize;
            int x1=(x+1)*sqrSize,y1=(y+1)*sqrSize;
            if(map[y][x] == 1) glColor3f(1,1,1); else glColor3f(0,0,0);
            glBegin(GL_QUADS);
            glVertex2i(x0,y0);
            glVertex2i(x0,y1-1);
            glVertex2i(x1-1,y1-1);
            glVertex2i(x1-1,y0);
            glEnd();
        }
    }
}

void Map::toggleMap() {
    display_map = !display_map;
}

int Map::getXY(float x, float y) {
    x = fabs(x); y = fabs(y);
    int ix = fmod((x/sqrSize) , map[0].size());
    int iy = fmod((y/sqrSize) , map.size());
    return map[iy][ix];
}

class Player {
    public:
        float x,y,dir,turnRate,speed,maxDist,fov,color_fadeoff;
        int nb_rays;
        bool display_map;
        std::vector<float> rays;
        std::vector<int> faces;
        Map*map;
        void draw();
        void toggleMap();
        void move(int);
        void strafe(int);
        void rotate(int);
        void castRay(float,float&,int&);
        void raycast();
        void draw3D();
        Player(Map*);
};

Player::Player(Map*m) {
    x = WIDTH / 2;
    y = HEIGHT / 2;
    dir = 0;
    turnRate = 0.1;
    speed = 10;
    map = m;
    maxDist=6000;
    color_fadeoff=300;
    nb_rays = 600;
    fov = 60;
    display_map = false;
}

void Player::draw() {
    if (display_map) {
        glColor3f(0,0,1);
        glPointSize(8);
        glBegin(GL_POINTS);
        glVertex2i(x,y);
        glEnd();

        glLineWidth(3.0);
        glBegin(GL_LINES);
        glVertex2i(x,y);
        const float l = 20;
        glVertex2i(x+l*cos(dir),y+l*sin(dir));
        glEnd();
    }

    raycast();
    if(!display_map) draw3D();
}

void Player::toggleMap() {
    display_map = !display_map;
}

void Player::move(const int dx) {
    x += speed*dx*cos(dir);
    y += speed*dx*sin(dir);
}

void Player::strafe(const int dy) {
    x += speed*dy*cos(dir+M_PI/2);
    y += speed*dy*sin(dir+M_PI/2);
}


void Player::rotate(const int da) {
    if (da > 0) {
        dir += turnRate;
        if (dir > 2*M_PI)
            dir = dir - (2*M_PI);
    }
    else {
        dir -= turnRate;
        if (dir < 0)
            dir = (float)(2*M_PI) + dir;
    }
}

void Player::castRay(float angle, float& dist, int& face) {
    float casted_rayx, casted_rayy;
    face = 0;
    // Cast y
    float rx,ry,xo,yo,currDist=0;
    int sqrSize = map->sqrSize;
    // rx ry
    if(angle > M_PI) ry = floor(y/sqrSize)*sqrSize;
    else if(angle < M_PI) ry = ceil(y/sqrSize)*sqrSize;
    else {
        currDist = maxDist;
        ry = 0;
    }
    rx = -(tan(-angle - M_PI/2) * (y-ry) - x);
    float dy = (angle > M_PI)? -sqrSize/2: sqrSize/2;
    // xo yo
    yo = (angle > M_PI)? -sqrSize: sqrSize;
    xo = -tan(angle - M_PI/2) * yo;

    float cx = rx, cy = ry;
    float distO = sqrt(xo*xo+yo*yo);
    if (currDist == 0) currDist = sqrt(rx*rx+ry*ry);
    while(currDist < maxDist) {
       if (map->getXY(cx,cy+dy))
           break;
       cx += xo;
       cy += yo;
       currDist += distO;
    }
    casted_rayx = cx;
    casted_rayy = cy;

    if (display_map) {
        glLineWidth(3.0);
        glColor3f(1,0,0);
        glBegin(GL_LINES);
        glVertex2i(x,y);
        glVertex2i(cx,cy);
        glEnd();
    }

    // Cast x
    currDist = 0;
    if(angle > 3*M_PI/2 || angle < M_PI/2)
        rx = ceil(x/sqrSize)*sqrSize;
    else if(angle < 3*M_PI/2 && angle > M_PI/2)
        rx = floor(x/sqrSize)*sqrSize;
    else {
        currDist = maxDist;
        rx = 0;
    }
    ry = -(tan(angle) * (x-rx) - y);
    float dx = (angle > 3*M_PI/2 || angle < M_PI/2)? sqrSize/2: -sqrSize/2;
    if (map->getXY(rx+dx,ry) == 1) glColor3f(1,0,0); else glColor3f(0,1,0);
    // xo yo
    xo = (angle > 3*M_PI/2 || angle < M_PI/2)? sqrSize: -sqrSize;
    yo = tan(angle) * xo;

    cx = rx, cy = ry;
    distO = sqrt(xo*xo+yo*yo);
    if (currDist == 0) currDist = sqrt(rx*rx+ry*ry);
    while(currDist < maxDist) {
       if (map->getXY(cx+dx,cy))
           break;
       cx += xo;
       cy += yo;
       currDist += distO;
    }
    if (
            sqrt(pow(x-cx,2)+pow(y-cy,2)) < 
            sqrt(pow(x-casted_rayx,2)+pow(y-casted_rayy,2))) {
        casted_rayx = cx;
        casted_rayy = cy;
        face = 1;
    }

    // Remove fish eye
    float ca = angle - dir; if (ca < 0) ca += 2*M_PI; if (ca > 2*M_PI) ca -= 2*M_PI;
    dist = cos(ca)*sqrt(pow(x-casted_rayx,2)+pow(y-casted_rayy,2));


    if (display_map) {
        glLineWidth(1.0);
        glColor3f(0,1,0);
        glBegin(GL_LINES);
        glVertex2i(x,y);
        glVertex2i(cx,cy);
        glEnd();
        glBegin(GL_POINTS);
        glVertex2i(casted_rayx,casted_rayy);
        glEnd();
    }
}


void Player::raycast() {
    rays.erase(rays.begin(),rays.begin()+rays.size());
    faces.erase(faces.begin(),faces.begin()+faces.size());
    // compute fov rad increment
    const float fov_rad = fov * M_PI / 180;
    const float d_rad = fov_rad / nb_rays;

    for(int i=-nb_rays/2; i<nb_rays/2; i++) {
        float angle = dir + i*d_rad;
        if (angle > 2*M_PI)
            angle = angle - (2*M_PI);
        else if (angle < 0)
            angle = (2*M_PI) + angle;
        float ray; int face;
        castRay(angle, ray, face);
        rays.push_back(ray);
        faces.push_back(face);
    }
}

void Player::draw3D() {
    // Draw sky
    glColor3f(0.8,0.8,0.8);
    //glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glVertex2i(0,0);
    glVertex2i(0,HEIGHT/2);
    glVertex2i(WIDTH,HEIGHT/2);
    glVertex2i(WIDTH,0);
    glEnd();

    float wall_w = (float)WIDTH /(float) nb_rays;
    for(unsigned i=0; i<rays.size(); i++) {
        float wall_h = HEIGHT/(rays[i]/10); if (wall_h > HEIGHT) wall_h = HEIGHT;
        float x0 = i * wall_w;
        float y0 = (HEIGHT - wall_h)/2;
        float x1 = (i+1) * wall_w;
        float y1 = HEIGHT - y0;

        float color_fade = rays[i] * (-1.0/color_fadeoff)+1.0;
        color_fade = 1.0;
        float face_fade = (faces[i])? 0.5 : 1.0;

        glColor3f(
                0*color_fade*face_fade,
                0*color_fade*face_fade,
                1*color_fade*face_fade
                );
        glBegin(GL_QUADS);
        glVertex2i(x0,y0);
        glVertex2i(x0,y1);
        glVertex2i(x1,y1);
        glVertex2i(x1,y0);
        glEnd();

    }

}

void init(void) {
    glClearColor(0.3, 0.3, 0.3, 0.0);
    gluOrtho2D(0,WIDTH,HEIGHT,0);
}

Map map;
Player player(&map);

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    map.draw();
    player.draw();
    glFlush();
    glutSwapBuffers();
}

void keyboard_cb(unsigned char key, int x, int y) {
    switch((char)key) {
        case 'w':
            player.move(1);
            break;
        case 's':
            player.move(-1);
            break;
        case 'd':
            player.strafe(1);
            break;
        case 'a':
            player.strafe(-1);
            break;
        case 'q':
            player.rotate(-1);
            break;
        case 'e':
            player.rotate(1);
            break;
        case 'm':
            player.toggleMap();
            map.toggleMap();
            break;
        case 'p':
            std::cout << "x:" << x << ", y:" << y << std::endl;
            break;
        case 27:    /* ESC */
            glutDestroyWindow(win_handler);
            exit(0);
        default:
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char *argv[]) {
    map.loadData((char*)"map.txt");
    map.print();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    win_handler = glutCreateWindow("Raycaster" );


    init();

    glutKeyboardFunc(keyboard_cb);
    glutDisplayFunc(display);

    glutMainLoop();

    return 0;
}


