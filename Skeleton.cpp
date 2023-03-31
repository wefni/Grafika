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
unsigned int vbo[12];



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

vec3 dir(vec3 p,vec3 q,float t){
    return (q-p*coshf(t))/sinhf(t);
}

float distance(vec3 p,vec3 q){
    return acoshf(newDot(p,q));
}


vec3 rotate(vec3 v,vec3 p,float phi){
    v = norm(v);
    p.z = p_w(p);
    v = hip_w(v,p);
    return v*cosf(phi) + mer(v,p)*sinf(phi);
}

vec3 pont(vec3 v,vec3 p){
    v = norm(v);
    p.z = p_w(p);
    v = hip_w(v,p);
    float t = sqrtf(newDot(v,v));
    return p*coshf(t)+v*sinhf(t);
}

const int nv = 1000;


bool pressed[256] = {false,};

std::vector<vec3> pontok;

vec3 p1;

class Hami{
public:
    vec3 origo;
    vec3 vektor;
    unsigned int id;
    unsigned int id2;


    Hami(vec3 o,vec3 v,unsigned int i,unsigned int j){
        origo = o;
        vektor = v;
        id = i;
        id2 = j;
    }

    void drawHami(){
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo[id]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[id]);

        vektor = norm(vektor);
        origo.z = p_w(origo);
        vektor = hip_w(vektor,origo);

        vec3 vertices[nv];
        vec3 eP;
        vec3 eP2;


        for(int i = 0; i < nv; ++i) {
            p1 = vel_vector(vektor,origo,0.35);
            p1.z = p_w(p1);
            vertices[i] = p1;

           if(i==100) eP = p1;
           if(i==900) eP2 = p1;

            vektor = rotate(vektor,origo,0.00628318531);
        }

        glBufferData(GL_ARRAY_BUFFER,sizeof(vec3)*nv,vertices,GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);

        drawSzem(eP,id2);
        drawSzem(eP2,id2+1);
    }

    void drawSzem(vec3 ep,int iD) {
        glGenVertexArrays(1, &vao);
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
    }

    void go(){

        vec3 pet = vel_vector(vektor,origo,0.01f);
        origo = pet;
        vektor = der(vektor,origo,0.01f);

        pontok.push_back(origo);


    }
};



void circ(){

    glGenVertexArrays(1, &vao);
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
    glViewport(0, 0, windowWidth, windowHeight);
    circ();

    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

Hami h(vec3(0,0,1.0),vec3(1,1,0.0),1,2);

void onDisplay() {
    glClearColor(0.25, 0.25, 0.25, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    int location = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(location, 0.0f, 0.0f, 0.0f);

    h.drawHami();

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);


    int location2 = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(location2, 1.0f, 0.0f, 0.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int location3 = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(location3, 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int location4 = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(location4, 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nv);

    int location5 = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(location5, 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_LINE_STRIP, 0, pontok.size());



    glutSwapBuffers();
}


void onKeyboard(unsigned char key, int pX, int pY) {
   pressed[key] = true;
}


void onKeyboardUp(unsigned char key, int pX, int pY) {
    pressed[key] = false;
}


void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;
    printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}


void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;

    char * buttonStat;
    switch (state) {
        case GLUT_DOWN: buttonStat = "pressed"; break;
        case GLUT_UP:   buttonStat = "released"; break;
    }

    switch (button) {
        case GLUT_LEFT_BUTTON:   printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);   break;
        case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
        case GLUT_RIGHT_BUTTON:  printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);  break;
    }
}


void onIdle() {
    float time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;

    if(pressed['e']) {
        h.go();
        /*glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo[11]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);


        //vec3 pet = vel_vector(vektor,origo,0.01f);
        //origo = pet;
       // vektor = der(vektor,origo,0.01f);

        printf("{%f %f}\n",h.origo.x,h.origo.y);


        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vec3) * pontok.size(),
                     &pontok,
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
                              3, GL_FLOAT, GL_FALSE,
                              0, NULL);*/

    }
    else if(pressed['f']) {
        h.vektor = rotate(h.vektor,h.origo,M_PI/150);
    }
    else if(pressed['s']) {
       h.vektor = rotate(h.vektor,h.origo,-M_PI/150);
    }


    glutPostRedisplay();
}
