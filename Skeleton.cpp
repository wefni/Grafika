//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!!
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Halász Dávid Péter
// Neptun : XTBJD6
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"


const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	//uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec3 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x/(vp.z+1), vp.y/(vp.z+1), 1, 1);		// transform vp from modeling space to normalized device space
	}
)";


const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers

	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram;
unsigned int vao;
unsigned int vbo[20];


float newDot(const vec3& v1, const vec3& v2) {
    return (v1.x * v2.x + v1.y * v2.y - v1.z * v2.z);
}

vec3 norm(vec3 v) {
    return v * (1 / sqrtf(newDot(v,v)));
}

float p_w(vec3 p){
    return sqrtf((p.x*p.x+p.y*p.y+1));
}

vec3 hip_w(vec3 v,vec3 p){
    vec3 vek(v.x,v.y,((v.x*p.x+v.y*p.y)/p_w(p)));
    return vek;
}

vec3 mer(vec3 v,vec3 p){
    v = norm(v);
    p.z = p_w(p);
    v = hip_w(v,p);
    return cross(vec3(v.x,v.y,-v.z),vec3(p.x,p.y,-p.z));
}

vec3 vel_vector(vec3 v,vec3 p,float t){
    v = norm(v);
    p.z = p_w(p);
    v = hip_w(v,p);
    return p*coshf(t)+v*sinhf(t);
}

vec3 der(vec3 v,vec3 p,float t){
    v = norm(v);
    p.z = p_w(p);
    v = hip_w(v,p);
    return p*sinhf(t)+v*coshf(t);
}

vec3 dir(vec3 d,vec3 q,float t){
    d.z = p_w(d);
    q.z = p_w(q);
    return (q-d*coshf(t))/sinhf(t);
}

float distance(vec3 d,vec3 q){
    d.z = p_w(d);
    q.z = p_w(q);
    return acoshf(-1*newDot(d,q));
}

vec3 rotate(vec3 v,vec3 p,float phi){
    v = norm(v);
    p.z = p_w(p);
    v = hip_w(v,p);
    return v*cosf(phi) + mer(v,p)*sinf(phi);
}

const int nv = 1000;
bool pressed[256] = {false,};
std::vector<vec3> pontok;
std::vector<vec3> pontok2;
vec3 p1;

float radius = 0.01f;

class Hami{
public:
    vec3 origo;
    vec3 vektor;
    vec3 pupVec;
    vec3 eP;
    vec3 eP2;
    vec3 p3;
    unsigned int id;
    unsigned int id2;

    Hami(vec3 o,vec3 v,unsigned int i,unsigned int j){
        origo = o;
        vektor = v;
        id = i;
        id2 = j;
        pupVec = vec3(1,1,0);
    }

    void drawHami(){

        glBindVertexArray(vao);

        glGenBuffers(1, &vbo[id]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[id]);

        vektor = norm(vektor);
        origo.z = p_w(origo);
        vektor = hip_w(vektor,origo);

        vec3 vertices[nv];
        vec3 eP3;

        for(int i = 0; i < nv; ++i) {
            p1 = vel_vector(vektor,origo,0.30);
            p1.z = p_w(p1);
            vertices[i] = p1;

            if(i==0) eP3 = p1;
            if(i==100) eP = p1;
            if(i==900) eP2 = p1;

            vektor = rotate(vektor,origo,0.00628318531);
        }

        glBufferData(GL_ARRAY_BUFFER,sizeof(vec3)*nv,vertices,GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);

        drawSzem(eP,id2,1);
        drawSzem(eP2,id2+1,2);
        drawSzaj(eP3,id2+2);
    }

    void drawSzem(vec3 ep,int iD,int sz) {

        glBindVertexArray(vao);

        glGenBuffers(1, &vbo[iD]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[iD]);


        vec3 t(ep.x, ep.y, ep.z);
        vec3 vek(1, 1, 0);
        vek = norm(vek);
        t.z = p_w(t);
        vek = hip_w(vek, t);

        vec3 vertices[nv];

        p3 = vel_vector(pupVec, t, 0.05f);

        for (int i = 0; i < nv; ++i) {
            vec3 p2 = vel_vector(vek, t, 0.1f);
            p2.z = p_w(p2);
            vertices[i] = p2;
            vek = rotate(vek, t, 0.00628318531);
        }

        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vec3) * nv,
                     vertices,
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
                              3, GL_FLOAT, GL_FALSE,
                              0, NULL);
        if(sz == 1)  drawPup(p3,id2+3);
        if(sz == 2)  drawPup(p3,id2+4);
    }

    void drawPup(vec3 ep,int iD){

        glBindVertexArray(vao);

        glGenBuffers(1, &vbo[iD]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[iD]);

        vec3 t(ep.x, ep.y, ep.z);
        vec3 vek(1,1,0);
        vek = norm(vek);
        t.z = p_w(t);
        vek = hip_w(vek, t);

        vec3 vertices[nv];

        for (int i = 0; i < nv; ++i) {
            vec3 p2 = vel_vector(vek, t, 0.05f);
            p2.z = p_w(p2);
            vertices[i] = p2;
            vek = rotate(vek, t, 0.00628318531);
        }

        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vec3) * nv,
                     vertices,
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
                              3, GL_FLOAT, GL_FALSE,
                              0, NULL);

    }

    void drawSzaj(vec3 ep,int iD) {

        glBindVertexArray(vao);

        glGenBuffers(1, &vbo[iD]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[iD]);

        vec3 t(ep.x, ep.y, ep.z);
        vec3 vek(1, 1, 0);
        vek = norm(vek);
        t.z = p_w(t);
        vek = hip_w(vek, t);

        vec3 vertices[nv];

            for (int i = 0; i < nv; ++i) {
                vec3 p2 = vel_vector(vek, t, radius);
                p2.z = p_w(p2);
                vertices[i] = p2;
                vek = rotate(vek, t, 0.00628318531);
            }

        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vec3) * nv,
                     vertices,
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
                              3, GL_FLOAT, GL_FALSE,
                              0, NULL);
    }

    void go(){

        glBindVertexArray(vao);

        glGenBuffers(1, &vbo[8]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);

        vec3 pet = vel_vector(vektor,origo,0.01f);
        origo = pet;
        vektor = der(vektor,origo,0.01f);

        pontok.push_back(pet);

        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vec3) * pontok.size(),
                     &pontok[0],
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
                              3, GL_FLOAT, GL_FALSE,
                              0, NULL);

    }

    void korbe(){
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo[12]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);

        vektor = rotate(vektor,origo,-M_PI/150);
        vec3 pet = vel_vector(vektor,origo,0.01f);
        origo = pet;
        vektor = der(vektor,origo,0.01f);


        pontok2.push_back(pet);

        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vec3) * pontok2.size(),
                     &pontok2[0],
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
                              3, GL_FLOAT, GL_FALSE,
                              0, NULL);
    }
};



void circ(){


    glBindVertexArray(vao);


    glGenBuffers(1, &vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

    vec3 vert[nv];

    for (int i = 0; i < nv; ++i) {
       float fi = i*2*M_PI/nv;
       vert[i] = vec3(cosf(fi),sinf(fi),0);
    }


    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vec3)*nv,
                 vert,
                 GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3, GL_FLOAT, GL_FALSE,
                          0, NULL);


}

void onInitialization() {
    glGenVertexArrays(1, &vao);
    glViewport(0, 0, windowWidth, windowHeight);
    circ();
    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

Hami h(vec3(0,0,1.0),vec3(1,1,0.0),1,2);
Hami h2(vec3(2.0,0.9,0),vec3(1,1,0),13,14);

void onDisplay() {
    glClearColor(0.25, 0.25, 0.25, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    int nagyKor = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(nagyKor, 0.0f, 0.0f, 0.0f);


        h2.drawHami();
        h.drawHami();


    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);


    int csikPiros = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(csikPiros, 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_LINE_STRIP, 0, pontok.size());

    int csikZold = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(csikZold, 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_LINE_STRIP, 0, pontok2.size());

    int hamiZold = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(hamiZold, 0.0f, 1.0f, 0.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int szemZold_egy = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(szemZold_egy, 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int szemZold_ketto = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(szemZold_ketto, 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[15]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int szajZold = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(szajZold, 0.0f, 0.0f, 0.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[16]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int pupZold1 = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(pupZold1, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[17]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int pupZold2 = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(pupZold2, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[18]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int hamiPiros = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(hamiPiros, 1.0f, 0.0f, 0.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int szemPiros_egy = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(szemPiros_egy, 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int szemPiros_ketto = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(szemPiros_ketto, 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int szajPiroos = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(szajPiroos, 0.0f, 0.0f, 0.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int pupPiroos1 = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(pupPiroos1, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int pupPiroos2 = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(pupPiroos2, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    glutSwapBuffers();
}


void onKeyboard(unsigned char key, int pX, int pY) {
   pressed[key] = true;
}


void onKeyboardUp(unsigned char key, int pX, int pY) {
    pressed[key] = false;
}


void onMouseMotion(int pX, int pY) {
}


void onMouse(int button, int state, int pX, int pY) {
}

int lt;

void onIdle() {

    float time = glutGet(GLUT_ELAPSED_TIME);
    int dt = (time-lt);

    if(dt>40){
        for (int i = 0; i < 40; ++i) {
            h2.korbe();
            radius = 0.1f*sinf((time/1000)/1.5f);

            h.pupVec = dir(h.eP,h2.origo, distance(h.eP,h2.origo));
            h2.pupVec = dir(h2.eP,h.origo, distance(h2.eP,h.origo));

            if(pressed['e']) {
                h.go();
            }

            if(pressed['f']) {
                h.vektor = rotate(h.vektor,h.origo,M_PI/150);
            }

            if(pressed['s']) {
                h.vektor = rotate(h.vektor,h.origo,-M_PI/150);
            }
        }
        dt-=40;
    }

    h2.korbe();
    radius = 0.1f*sinf((time/1000)/1.5f);

    h.pupVec = dir(h.eP,h2.origo, distance(h.eP,h2.origo));
    h2.pupVec = dir(h2.eP,h.origo, distance(h2.eP,h.origo));

    if(pressed['e']) {
        h.go();
    }

    if(pressed['f']) {
        h.vektor = rotate(h.vektor,h.origo,M_PI/150);
    }

    if(pressed['s']) {
        h.vektor = rotate(h.vektor,h.origo,-M_PI/150);
    }

    lt = time;

    glutPostRedisplay();
}
