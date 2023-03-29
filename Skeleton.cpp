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

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	//uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec3 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x/(vp.z+1), vp.y/(vp.z+1), 1, 1);		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers

	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram; // vertex and fragment shaders
unsigned int vao;	   // virtual world on the GPU


////                Írt függvények              /////////////

float newDot(const vec3& v1, const vec3& v2) {
    return (v1.x * v2.x + v1.y * v2.y - v1.z * v2.z);                                             // Skaláris szorzás
}

vec3 mer(vec3 v,vec3 p){
    return cross(vec3(v.x,v.y,-v.z),vec3(p.x,p.y,-p.z));         // Merőleges állítás
}

vec3 vel_vector(vec3 v,vec3 p,float t){                                                         // Sebesség vektor számítás
    return p*coshf(t)+v*sinhf(t);
}

vec3 t_dis(vec3 v,vec3 p,float t){                                                              // Pont helye t idővel később
    return p*sinhf(t)+v*coshf(t);
}

vec3 dir(vec3 p,vec3 q,float t){                                                                // Pont iránya
    return (q-p*coshf(t))/sinhf(t);
}

float distance(vec3 p,vec3 q){                                                                  // Pont távolság
    return acoshf(newDot(p,q));
}

vec3 rotate(vec3 v,vec3 p,float phi){                                                           // Elforgatás fi szöggel
    return v*cosf(phi) + mer(v,p)*sinf(phi);
}

vec3 pont(vec3 v,vec3 p){
    float t = sqrtf(newDot(v,v));                                                       // Egy ponthoz képest adott irányban és távolságra lévő pont előállítása
    return p*coshf(t)+v*sinhf(t);
}

vec3 norm(vec3 v) {
    return v * (1 / sqrtf(newDot(v,v)));
}

float p_w(vec3 p){                                                                                // Pontok
    return sqrtf((p.x*p.x+p.y*p.y+1));
}

vec3 hip_w(vec3 v,vec3 p){
    vec3 vek(v.x,v.y,((v.x*p.x+v.y*p.y)/p_w(p)));
    return vek;                                                              // Merőleges pont visszavetítés
}

const int nv = 1000;
vec3 vert[nv];

unsigned int vbo;
unsigned int vbo2;

bool pressed[256] = {false,};
float yRot = 0.0f;
float xRot = 0.0f;


void circ(){

    glGenVertexArrays(1, &vao);	// get 1 vao id
    glBindVertexArray(vao);


    glGenBuffers(1, &vbo);	// Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    vec3 kor(0,0,1);
    vec3 nagyVek(1.54,0,0);

    for (int i = 0; i < nv; ++i) {
        vec3 p1 = pont(nagyVek,kor);
        vert[i] = p1;
        nagyVek = rotate(nagyVek,kor,0.00628318531);
    }


    glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
                 sizeof(vec3)*nv,  // # bytes
                 vert,	      	// address
                 GL_STATIC_DRAW);	// we do not change later


    glEnableVertexAttribArray(0);  // AttribArray 0
    glVertexAttribPointer(0,       // vbo -> AttribArray 0
                          3, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
                          0, NULL); 		     // stride, offset: tightly packed


}

void hip_circ(float yrot,float xrot){

    glGenVertexArrays(1, &vao);	// get 1 vao id
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo2);	// Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);

    vec3 ori(xrot,yrot,1);
    vec3 vek(1,0,0);
    ori.z = p_w(ori);
    vek = hip_w(vek,ori);
    vek = norm(vek);
    vec3 vertices[nv];


    for(int i = 0; i < nv; ++i) {
      vec3 p1 = pont(vek,ori);
      p1.z = p_w(p1);
      vertices[i] = p1;
      vek = rotate(vek,ori,0.0628318531);
    }


    glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
                 sizeof(vec3)*nv,  // # bytes
                 vertices,	      	// address
                 GL_STATIC_DRAW);	// we do not change later

    glEnableVertexAttribArray(0);  // AttribArray 0
    glVertexAttribPointer(0,       // vbo -> AttribArray 0
                          3, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
                          0, NULL); 		     // stride, offset: tightly packed

}

///////////////////////////////////////////////////////////////




// Initialization, create an OpenGL context
void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);

    circ();



    // create program for the GPU
    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

// Window has become invalid: Redraw
void onDisplay() {
    glClearColor(0.25, 0.25, 0.25, 0);     // background color
    glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer

    int location = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(location, 0.0f, 0.0f, 0.0f); // 3 floats

    //const float r0 = xmax;
    hip_circ(yRot,xRot);

    glBindVertexArray(vao);  // Draw call

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0/*startIdx*/, nv/*# Elements*/);


    int location2 = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(location2, 1.0f, 0.0f, 0.0f); // 3 floats

    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,0, NULL);
    glDrawArrays(GL_TRIANGLE_FAN, 0/*startIdx*/, nv/*# Elements*/);


    glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
   pressed[key] = true;
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
    pressed[key] = false;
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;
    printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

// Mouse click event
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

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
    float time = glutGet(GLUT_ELAPSED_TIME)/1000.0f;

    if(pressed['e']){
        yRot += 0.01f*time;
    }else if(pressed['s']){
        xRot += 0.01f*time;
    }else if(pressed['f']){
        xRot -= 0.01f*time;
    }


    glutPostRedisplay();
}
