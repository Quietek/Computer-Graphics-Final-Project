#include "CSCIx229.h"


//  Basic Variable initialization
int mode=1;              //  Projection mode
int move=1;              //  Move light
int fov=55;              //  Field of view (for perspective)
int light=1;             //  Lighting
double asp=1;            //  Aspect ratio
double dim=3.0;          //  Size of world
int maxparticles = 1000;

// Lighting & Texture Variable initialization
int one       =   1;     // Unit value
int distance  =  20;     // Light distance
int inc       =  10;     // Ball increment
int smooth    =   1;     // Smooth/Flat shading
int local     =   0;     // Local Viewer Model
int emission  =   0;     // Emission intensity (%)
int ambient   =  30;     // Ambient intensity (%)
int diffuse   = 100;     // Diffuse intensity (%)
int specular  =   0;     // Specular intensity (%)
int shininess =   0;     // Shininess (power of two)
float shiny   =   1;     // Shininess (value)
int zh        =  90;     // Light azimuth
float ylight  =   0;     // Elevation of light
unsigned int texture[100]; // Texture names

//First Person Camera Variable initalization

double PlayerX,PlayerY,Dx,Dz = 0.0;
double PlayerZ = 5;
double OldX, OldY;
int XRotation,YRotation = 0;

//Rain Variable Initialization
int loop;
int i,j;



typedef struct {
    // Life
    int alive;    // is the particle alive?
    float life;    // particle lifespan
    float fade; // decay
    // color
    float red;
    float green;
    float blue;
    // Position/direction
    float x_position;
    float y_position;
    float z_position;
    // Velocity/Direction, only goes down in y dir
    float vel;
    // Gravity
    float gravity;
}particles;

particles partic[1500];

// Initialzie Particles
void initParticles(int i) {
    partic[i].alive = 1;
    partic[i].life = 1.0;
    partic[i].fade = (rand()%100)/1000.0f+0.003f;
    
    partic[i].x_position = (float) (rand() % 21) - 5;
    partic[i].y_position = 10.0;
    partic[i].z_position = (float) (rand() % 21) + 36;
    
    partic[i].red = 0;
    partic[i].green = 0;
    partic[i].blue = 1.0;
    
    partic[i].vel = 0;
    partic[i].gravity = -0.8;
    
}

void init( ) {
    for (loop = 0; loop < 1500; loop++) {
        initParticles(loop);
    }
}

void Rain() {
    float x, y, z;
    for (loop = 0; loop < 1500; loop=loop+2) {
        if (partic[loop].alive) {
            x = partic[loop].x_position;
            y = partic[loop].y_position;
            z = partic[loop].z_position - 40;
            
            // Draw particles
            glColor3f(0.2, 0.2, 1.0);
            glBegin(GL_LINES);
            glVertex3f(x, y, z);
            glVertex3f(x, y+0.5, z);
            glEnd();
            

            // Move
            partic[loop].y_position += partic[loop].vel / (2*1000);
            partic[loop].vel += partic[loop].gravity;
            // Decay
            partic[loop].life -= partic[loop].fade;
            
            if (partic[loop].y_position <= -10) {
                partic[loop].life = -1.0;
            }
            // Revive
            if (partic[loop].life < 0.0) {
                initParticles(loop);
            }
        }
    }
}



/*
 *  Draw vertex in polar coordinates with normal
 */
static void Vertex(double th,double ph)
{
    double x = Sin(th)*Cos(ph);
    double y = Cos(th)*Cos(ph);
    double z =         Sin(ph);
    //  For a sphere at the origin, the position and normal vectors are the same
    glNormal3d(x,y,z);
    glVertex3d(x,y,z);
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
static void ball(double x,double y,double z,double r)
{
    int th,ph;
    float yellow[] = {1.0,1.0,0.0,1.0};
    float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    //  Save transformation
    glPushMatrix();
    //  Offset, scale and rotate
    glTranslated(x,y,z);
    glScaled(r,r,r);
    //  White ball
    glColor3f(1,1,1);
    glMaterialf(GL_FRONT,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
    glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
    //  Bands of latitude
    for (ph=-90;ph<90;ph+=inc)
    {
        glBegin(GL_QUAD_STRIP);
        for (th=0;th<=360;th+=2*inc)
        {
            Vertex(th,ph);
            Vertex(th,ph+inc);
        }
        glEnd();
    }
    //  Undo transformations
    glPopMatrix();
}

static void bed(double x,double y,double z,
                     double dx,double dy,double dz,
                     double th,
                     int color1, int color2, int color3)
{
    int wallrep = 2;
    float white[] = {1,1,1,1};
    float black[] = {0,0,0,1};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    // Save Transformation
    glPushMatrix();
    // Offset
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);
    // Enable Textures
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[9]);
    glBegin(GL_QUADS);
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-.8,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.3, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-.8,+0.3, 1);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(+1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-.8,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.8,+.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(+1,+.3,0);
    
    
    
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(+1,0,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(+1,+0.3,+1);
    
    

    
    
    //  Top
    glColor3f(color1,color2,color3);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);              glVertex3f(-.8,+0.3,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,+0.3,+1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-.8,+0.3,0);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-.8,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,0,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-.8,0,+1);
    glEnd();
    
    
    
    glBindTexture(GL_TEXTURE_2D,texture[8]);
    glBegin(GL_QUADS);
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.8,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.8,+0.3, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.3, 1);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(-.8,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,+.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-.8,+.3,0);
    
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,+1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,+0.3,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.3,0);
    
    //  Top
    glColor3f(color1,color2,color3);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,+0.3,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.8,+0.3,+1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.8,+0.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.3,0);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-.8,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.8,0,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,0,+1);
    glEnd();
    
    
    glEnd();
    glBindTexture(GL_TEXTURE_2D,texture[3]);
    glBegin(GL_QUADS);
    
    //Feet
    //Foot One
    
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2, 1);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0.9);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-0.9,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-0.9,-.2,1);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,+0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,1);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-.2,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-.2,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,+1);
    
    
    
    //Foot Two
    
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(1,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2, 1);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(1,0,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,0.9);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(0.9,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(0.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(0.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(0.9,-.2,1);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(1,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(1,0,+0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(1,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,1);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(1,-.2,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-.2,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,+1);
    
    
    //Foot Three
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(-1,0, 0);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0, 0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2, 0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2, 0);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0.1);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-0.9,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.9,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.9,-.2,0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(-0.9,-.2,0);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,-.2,0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-.2,0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-.2,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0);
    
    
    
    //Foot Four
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(1,0, 0);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0, 0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2, 0);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2, 0);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(1,0,0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,+0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,+0.1);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(0.9,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(0.9,0,+0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(0.9,-.2,+0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(0.9,-.2,0);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(1,0,+0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(1,-.2,+0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,0);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(1,-.2,+0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-.2,+0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,0);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,0);
    
    glEnd();
    
    glPopMatrix();
    
    
}
static void lamp(double x,double y,double z,
                  double dx,double dy,double dz,
                  double th,
                  int color1, int color2, int color3)

{
    float white[] = {1,1,1,1};
    float black[] = {0,0,0,1};
    int wallrep = 2;
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    // Save Transformation
    glPushMatrix();
    // Offset
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);
    // Enable Textures
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[3]);
    glBegin(GL_QUADS);
    //Lamp Base
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-.3,0, .3);
    glTexCoord2f(wallrep,0);        glVertex3f(+.3,0, .3);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+.3,+0.1, .3);
    glTexCoord2f(0,wallrep);        glVertex3f(-.3,+0.1, .3);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(+.3,0,-.3);
    glTexCoord2f(wallrep,0);        glVertex3f(-.3,0,-.3);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.3,+.1,-.3);
    glTexCoord2f(0,wallrep);        glVertex3f(+.3,+.1,-0.3);
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(+.3,0,+.3);
    glTexCoord2f(wallrep,0);        glVertex3f(+.3,0,-.3);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+.3,+0.1,-.3);
    glTexCoord2f(0,wallrep);        glVertex3f(+.3,+0.1,+.3);
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-.3,0,+.3);
    glTexCoord2f(wallrep,0);        glVertex3f(-.3,0,-.3);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.3,+0.1,-.3);
    glTexCoord2f(0,wallrep);        glVertex3f(-.3,+0.1,+.3);
    //  Top
    glColor3f(color1,color2,color3);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);              glVertex3f(-.3,+0.1,+.3);
    glTexCoord2f(wallrep,0);        glVertex3f(+.3,+0.1,+.3);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+.3,+0.1,-0.3);
    glTexCoord2f(0,wallrep);        glVertex3f(-.3,+0.1,-.3);
    
    //Lamp Stand
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-.05,0.1, .05);
    glTexCoord2f(wallrep,0);        glVertex3f(+.05,0.1, .05);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+.05,+1, .05);
    glTexCoord2f(0,wallrep);        glVertex3f(-.05,+1, .05);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(+.05,0.1,-.05);
    glTexCoord2f(wallrep,0);        glVertex3f(-.05,0.1,-.05);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.05,+1,-.05);
    glTexCoord2f(0,wallrep);        glVertex3f(+.05,+1,-.05);
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(+.05,.1,+.05);
    glTexCoord2f(wallrep,0);        glVertex3f(+.05,.1,-.05);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+.05,+1,-.05);
    glTexCoord2f(0,wallrep);        glVertex3f(+.05,+1,+.05);
    
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-.05,0.1,+.05);
    glTexCoord2f(wallrep,0);        glVertex3f(-.05,0.1,-.05);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.05,+1,-.05);
    glTexCoord2f(0,wallrep);        glVertex3f(-.05,+1,+.05);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D,texture[11]);
    glBegin(GL_QUADS);
    //Lamp Shade
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,.5547,.8320);
    glTexCoord2f(0,0);              glVertex3f(-.2,.8, .2);
    glTexCoord2f(wallrep,0);        glVertex3f(+.2,.8, .2);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+.1,+1.2, .05);
    glTexCoord2f(0,wallrep);        glVertex3f(-.1,+1.2, .05);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,.5547,-.8320);
    glTexCoord2f(0,0);              glVertex3f(+.2,0.8,-.2);
    glTexCoord2f(wallrep,0);        glVertex3f(-.2,0.8,-.2);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.1,+1.2,-.05);
    glTexCoord2f(0,wallrep);        glVertex3f(+.1,+1.2,-.05);
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(.8320,.5547,0);
    glTexCoord2f(0,0);              glVertex3f(+.2,.8,+.2);
    glTexCoord2f(wallrep,0);        glVertex3f(+.2,.8,-.2);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+.1,+1.2,-.05);
    glTexCoord2f(0,wallrep);        glVertex3f(+.1,+1.2,+.05);
    
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(-.8320,.5547,0);
    glTexCoord2f(0,0);              glVertex3f(-.2,0.8,+.2);
    glTexCoord2f(wallrep,0);        glVertex3f(-.2,0.8,-.2);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.1,+1.2,-.05);
    glTexCoord2f(0,wallrep);        glVertex3f(-.1,+1.2,+.05);
    glEnd();
    glPopMatrix();
}

static void table(double x,double y,double z,
                double dx,double dy,double dz,
                double th,
                int color1, int color2, int color3)
{
    int wallrep = 2;
    float white[] = {1,1,1,1};
    float black[] = {0,0,0,1};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    // Save Transformation
    glPushMatrix();
    // Offset
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);
    // Enable Textures
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[3]);
    glBegin(GL_QUADS);
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.3, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.3, 1);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(+1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,+.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(+1,+.3,0);
    
    
    
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(+1,0,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(+1,+0.3,+1);
    
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,+0.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.3,+1);
    
    
    
    
    //  Top
    glColor3f(color1,color2,color3);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,+0.3,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,+0.3,+1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.3,0);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,0,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,0,+1);
    
    //Feet
    //Foot One
    
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2, 1);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0.9);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-0.9,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-0.9,-.2,1);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,+0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,1);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-.2,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-.2,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,+1);
    
    
    
    //Foot Two
    
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(1,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2, 1);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(1,0,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,0.9);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(0.9,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(0.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(0.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(0.9,-.2,1);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(1,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(1,0,+0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(1,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,1);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(1,-.2,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-.2,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,+1);
    
    
    //Foot Three
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(-1,0, 0);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0, 0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2, 0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2, 0);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0.1);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-0.9,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.9,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.9,-.2,0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(-0.9,-.2,0);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,-.2,0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-.2,0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-.2,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0);
    
    
    
    //Foot Four
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(1,0, 0);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0, 0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2, 0);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2, 0);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(1,0,0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,+0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,+0.1);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(0.9,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(0.9,0,+0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(0.9,-.2,+0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(0.9,-.2,0);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(1,0,+0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(1,-.2,+0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,0);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(1,-.2,+0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-.2,+0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,0);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,0);
    
    glEnd();
    
    glPopMatrix();
    
    
}

static void sofa(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th,
                 int color1, int color2, int color3)
{
    float white[] = {1,1,1,1};
    float black[] = {0,0,0,1};
    int wallrep = 2;
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    // Save Transformation
    glPushMatrix();
    // Offset
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);
    // Enable Textures
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[7]);
    glBegin(GL_QUADS);
    //Body of Sofa
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.3, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.3, 1);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(+1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,+1,0);
    glTexCoord2f(0,wallrep);        glVertex3f(+1,+1,0);
    //Right raised side
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(+1,0.3,+0.3);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0.3,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+1,0);
    glTexCoord2f(0,wallrep);        glVertex3f(+1,+1,+0.3);
    //Left raised side
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0.3,+0.3);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0.3,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,+1,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+1,+0.3);
    //Seat back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0.3, 0.3);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0.3, 0.3);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+1, 0.3);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+1, 0.3);
    //Seat Top
    glColor3f(color1,color2,color3);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,+1,+0.3);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,+1,+0.3);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+1,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+1,0);
    
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(+1,0,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(+1,+0.3,+1);
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,+1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,+0.3,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.3,0);
    //  Top
    glColor3f(color1,color2,color3);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,+0.3,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,+0.3,+1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.3,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.3,0);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,0,0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,0,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,0,+1);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D,texture[3]);
    glBegin(GL_QUADS);
    
    //Feet
    //Foot One
    
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2, 1);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0.9);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-0.9,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-0.9,-.2,1);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,+0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,1);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-.2,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-.2,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,+1);
    
    
    
    //Foot Two
    
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(1,0, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2, 1);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(1,0,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,0.9);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(0.9,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(0.9,0,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(0.9,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(0.9,-.2,1);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(1,0,1);
    glTexCoord2f(wallrep,0);        glVertex3f(1,0,+0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(1,-.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,1);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(1,-.2,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-.2,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,+1);
    
    
    //Foot Three
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(-1,0, 0);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0, 0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2, 0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2, 0);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0.1);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-0.9,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.9,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.9,-.2,0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(-0.9,-.2,0);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,-.2,0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-.2,0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-.2,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-.2,0);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-.2,0);
    
    
    
    //Foot Four
    //  Front
    glColor3f(1,1,1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(1,0, 0);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0, 0);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2, 0);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2, 0);
    //  Back
    glColor3f(1,1,1);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(1,0,0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,0,0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,+0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,+0.1);
    //  Right
    glColor3f(1,1,1);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(0.9,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(0.9,0,+0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(0.9,-.2,+0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(0.9,-.2,0);
    //  Left
    glColor3f(1,1,1);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(1,0,0);
    glTexCoord2f(wallrep,0);        glVertex3f(1,0,+0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(1,-.2,+0.1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,0);
    //  Bottom
    glColor3f(1,1,1);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(1,-.2,+0.1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-.2,+0.1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-.2,0);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-.2,0);
    
    glEnd();
    
    glPopMatrix();
}


static void ground(double x,double y,double z,
                     double dx,double dy,double dz,
                     double th,
                     int color1, int color2, int color3)
{
    int rep = 10;
    float white[] = {1,1,1,1};
    float black[] = {0,0,0,1};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    // Save Transformation
    glPushMatrix();
    // Offset
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);
    // Enable Textures
    
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    
    glBindTexture(GL_TEXTURE_2D,texture[5]);
    glBegin(GL_QUADS);
    glColor3f(color1,color2,color3);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);glVertex3f(-1,0,+1);
    glTexCoord2f(rep,0);glVertex3f(+1,0,+1);
    glTexCoord2f(rep,rep);glVertex3f(+1,0,-1);
    glTexCoord2f(0,rep);glVertex3f(-1,0,-1);
    glEnd();
    
    glPopMatrix();
    
}

static void house(double x,double y,double z,
                  double dx,double dy,double dz,
                  double th,
                  int color1, int color2, int color3)
{
    //  Set specular color to white
    float white[] = {1,1,1,1};
    float black[] = {0,0,0,1};
    double wallrep = 3;
    double roofrep = 2;
    double doorrep = 2;
    double chimrep = 2;
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    //  Save transformation
    glPushMatrix();
    //  Offset
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);
    
    // Enable Textures
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    //House Base (Based on cube)
    glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBegin(GL_QUADS);

    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
    glTexCoord2f(wallrep,0);glVertex3f(+1,-1, 1);
    glTexCoord2f(wallrep,wallrep);glVertex3f(+1,+1, 1);
    glTexCoord2f(0,wallrep);glVertex3f(-1,+1, 1);
    glColor3f(color1,color2,color3);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);glVertex3f(+1,-1,-1);
    glTexCoord2f(wallrep,0);glVertex3f(-1,-1,-1);
    glTexCoord2f(wallrep,wallrep);glVertex3f(-1,+1,-1);
    glTexCoord2f(0,wallrep);glVertex3f(+1,+1,-1);
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);glVertex3f(+1,-1,+1);
    glTexCoord2f(wallrep,0);glVertex3f(+1,-1,-1);
    glTexCoord2f(wallrep,wallrep); glVertex3f(+1,+1,-1);
    glTexCoord2f(0,wallrep);glVertex3f(+1,+1,+1);
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);glVertex3f(-1,-1,-1);
    glTexCoord2f(wallrep,0);glVertex3f(-1,-1,+1);
    glTexCoord2f(wallrep,wallrep);glVertex3f(-1,+1,+1);
    glTexCoord2f(0,wallrep);glVertex3f(-1,+1,-1);
    //  Top
    glColor3f(color1,color2,color3);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);glVertex3f(-1,+0.99,+1);
    glTexCoord2f(wallrep,0);glVertex3f(+1,+0.99,+1);
    glTexCoord2f(wallrep,wallrep);glVertex3f(+1,+0.99,-1);
    glTexCoord2f(0,wallrep);glVertex3f(-1,+0.99,-1);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);glVertex3f(-1,-1,-1);
    glTexCoord2f(wallrep,0);glVertex3f(+1,-1,-1);
    glTexCoord2f(wallrep,wallrep);glVertex3f(+1,-1,+1);
    glTexCoord2f(0,wallrep);glVertex3f(-1,-1,+1);
    glColor3f(color1,color2,color3);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D,texture[10]);
    glBegin(GL_QUADS);
    //Interior
    //Front
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0); glVertex3f(-1,-1, 0.95);
    glTexCoord2f(wallrep,0);glVertex3f(+1,-1, 0.95);
    glTexCoord2f(wallrep,wallrep);glVertex3f(+1,+1, 0.95);
    glTexCoord2f(0,wallrep);glVertex3f(-1,+1, 0.95);
    //Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);glVertex3f(+1,-1,-0.95);
    glTexCoord2f(wallrep,0);glVertex3f(-1,-1,-0.95);
    glTexCoord2f(wallrep,wallrep);glVertex3f(-1,+1,-0.95);
    glTexCoord2f(0,wallrep);glVertex3f(+1,+1,-0.95);
    //Right
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);glVertex3f(+0.95,-1,+1);
    glTexCoord2f(wallrep,0);glVertex3f(+0.95,-1,-1);
    glTexCoord2f(wallrep,wallrep); glVertex3f(+0.95,+1,-1);
    glTexCoord2f(0,wallrep);glVertex3f(+0.95,+1,+1);
    //Left
    glColor3f(color1,color2,color3);
    glNormal3f(1,0,0);
    glTexCoord2f(0,0);glVertex3f(-0.95,-1,-1);
    glTexCoord2f(wallrep,0);glVertex3f(-0.95,-1,+1);
    glTexCoord2f(wallrep,wallrep);glVertex3f(-0.95,+1,+1);
    glTexCoord2f(0,wallrep);glVertex3f(-0.95,+1,-1);
    //Top
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);glVertex3f(-1,+0.95,+1);
    glTexCoord2f(wallrep,0);glVertex3f(+1,+0.95,+1);
    glTexCoord2f(wallrep,wallrep);glVertex3f(+1,+0.95,-1);
    glTexCoord2f(0,wallrep);glVertex3f(-1,+0.95,-1);
    glEnd();
    glBindTexture(GL_TEXTURE_2D,texture[12]);
    glBegin(GL_QUADS);
    //Bottom
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);glVertex3f(-1,-0.95,-1);
    glTexCoord2f(wallrep,0);glVertex3f(+1,-0.95,-1);
    glTexCoord2f(wallrep,wallrep);glVertex3f(+1,-0.95,+1);
    glTexCoord2f(0,wallrep);glVertex3f(-1,-0.95,+1);
    glEnd();
    
    
    // Roof
    glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBegin(GL_TRIANGLES);
    // Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);glVertex3f(+1.25,+1.0,+1.1);
    glTexCoord2f(1,0);glVertex3f(-1.25,+1.0,+1.1);
    glTexCoord2f(1/2,1);glVertex3f(0,+1.2,+1.1);
    // Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);glVertex3f(+1.25,+1.0,-1.1);
    glTexCoord2f(1,0);glVertex3f(-1.25,+1.0,-1.1);
    glTexCoord2f(1/2,1);glVertex3f(0,+1.2,-1.1);
    glEnd();
    // Sides of Roof
    glBindTexture(GL_TEXTURE_2D,texture[4]);
    glBegin(GL_QUADS);
    // Right side
    glColor3f(color1,color2,color3);
    glNormal3f(+0.16,+0.99,0);
    glTexCoord2f(0,0);glVertex3f(+1.25,+1.0,-1.1);
    glTexCoord2f(roofrep,0);glVertex3f(+1.25,+1.0,+1.1);
    glTexCoord2f(roofrep,roofrep);glVertex3f(0,+1.2,+1.1);
    glTexCoord2f(0,roofrep);glVertex3f(0,+1.2,-1.1);
    // Left Side
    glColor3f(color1,color2,color3);
    glNormal3f(-0.16,+0.99,0);
    glTexCoord2f(0,0);glVertex3f(-1.25,+1.0,-1.1);
    glTexCoord2f(roofrep,0);glVertex3f(-1.25,+1.0,+1.1);
    glTexCoord2f(roofrep,roofrep);glVertex3f(0,+1.2,+1.1);
    glTexCoord2f(0,roofrep);glVertex3f(0,+1.2,-1.1);
    // Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);glVertex3f(+1.25,+1.0,-1.1);
    glTexCoord2f(roofrep,0);glVertex3f(+1.25,+1.0,+1.1);
    glTexCoord2f(roofrep,roofrep);glVertex3f(-1.25,+1.0,+1.1);
    glTexCoord2f(0,roofrep);glVertex3f(-1.25,+1.0,-1.1);
    glEnd();
    
    // Draw the chimney
    glBindTexture(GL_TEXTURE_2D,texture[3]);
    glBegin(GL_QUADS);
    // Left
    glColor3f(color1,color2,color3);
    glNormal3f( -1, 0,   0);
    glTexCoord2f(0,0);glVertex3f(+.5,+1,+.25);
    glTexCoord2f(chimrep,0);glVertex3f(+.5,+1,-.25);
    glTexCoord2f(chimrep,chimrep);glVertex3f(+.5,+2,-.25);
    glTexCoord2f(0,chimrep);glVertex3f(+.5,+2,+.25);
    // Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1, 0,   0);
    glTexCoord2f(0,0);glVertex3f(+1,+1,+.25);
    glTexCoord2f(chimrep,0);glVertex3f(+1,+1,-.25);
    glTexCoord2f(chimrep,chimrep);glVertex3f(+1,+2,-.25);
    glTexCoord2f(0,chimrep);glVertex3f(+1,+2,+.25);
    // Back
    glColor3f(color1,color2,color3);
    glNormal3f(  0, 0,  -1);
    glTexCoord2f(0,0);glVertex3f(+.5,+1,-.25);
    glTexCoord2f(chimrep,0);glVertex3f(+.5,+2,-.25);
    glTexCoord2f(chimrep,chimrep);glVertex3f(+1,+2,-.25);
    glTexCoord2f(0,chimrep);glVertex3f(+1,+1,-.25);
    // Front
    glColor3f(color1,color2,color3);
    glNormal3f(  0, 0,  +1);
    glTexCoord2f(0,0);glVertex3f(+.5,+1,+.25);
    glTexCoord2f(chimrep,0);glVertex3f(+.5,+2,+.25);
    glTexCoord2f(chimrep,chimrep);glVertex3f(+1,+2,+.25);
    glTexCoord2f(chimrep,chimrep);glVertex3f(+1,+1,+.25);
    // Top
    glColor3f(0,0,0);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);glVertex3f(+.5,+2,+.25);
    glTexCoord2f(chimrep,0);glVertex3f(+.5,+2,-.25);
    glTexCoord2f(chimrep,chimrep);glVertex3f(+1,+2,-.25);
    glTexCoord2f(0,chimrep);glVertex3f(+1,+2,+.25);
    glEnd();
    
    // Door
    glBindTexture(GL_TEXTURE_2D,texture[1]);
    glBegin(GL_QUADS);
    glColor3f(color1,color2,color3);
    // Front
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);      glVertex3f(+.3,-1,+1.1);
    glTexCoord2f(1,0);      glVertex3f(-.3,-1,+1.1);
    glTexCoord2f(1,1);      glVertex3f(-.3,-0.2,+1.1);
    glTexCoord2f(0,1);      glVertex3f(+.3,-0.2,+1.1);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);      glVertex3f(+.3,-1,+.9);
    glTexCoord2f(1,0);      glVertex3f(-.3,-1,+.9);
    glTexCoord2f(1,1);      glVertex3f(-.3,-0.2,+.9);
    glTexCoord2f(0,1);      glVertex3f(+.3,-0.2,+.9);
    glEnd();
    // Top
    glBindTexture(GL_TEXTURE_2D,texture[3]);
    glBegin(GL_QUADS);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);              glVertex3f(+.3,-0.2,+1.1);
    glTexCoord2f(doorrep,0);        glVertex3f(+.3,-0.2,+0.9);
    glTexCoord2f(doorrep,doorrep);  glVertex3f(-.3,-0.2,+0.9);
    glTexCoord2f(0,doorrep);        glVertex3f(-.3,-0.2,+1.1);
    // Bottom
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(+.3,-1,+1.1);
    glTexCoord2f(doorrep,0);        glVertex3f(+.3,-1,+0.9);
    glTexCoord2f(doorrep,doorrep);  glVertex3f(-.3,-1,+0.9);
    glTexCoord2f(0,doorrep);        glVertex3f(-.3,-1,+1.1);
    // Right
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(+.3,-0.2,+1.1);
    glTexCoord2f(doorrep,0);        glVertex3f(+.3,-1,+1.1);
    glTexCoord2f(doorrep,doorrep);  glVertex3f(+.3,-1,+0.9);
    glTexCoord2f(0,doorrep);        glVertex3f(+.3,-0.2,+0.9);
    // Left
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-.3,-0.2,+1.1);
    glTexCoord2f(doorrep,0);        glVertex3f(-.3,-1,+1.1);
    glTexCoord2f(0,doorrep);        glVertex3f(-.3,-1,+0.9);
    glTexCoord2f(doorrep,doorrep);  glVertex3f(-.3,-0.2,+0.9);
    glEnd();
    
    glPopMatrix();
    
    
    
}

static void tvset(double x, double y, double z,
                  double dx, double dy, double dz,
                  double th,
                  int color1, int color2, int color3)
{
    double wallrep = 2;
    float white[] = {1,1,1,1};
    float black[] = {0,0,0,1};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
    // Save Transformation
    glPushMatrix();
    // Offset
    glTranslated(x,y,z);
    glRotated(th,0,1,0);
    glScaled(dx,dy,dz);
    // Enable Textures
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    
    //  TV Model
    glBindTexture(GL_TEXTURE_2D,texture[6]);
    glBegin(GL_QUADS);
    
    // Screen
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,-1, 0.8);
    glTexCoord2f(1,0);        glVertex3f(+1,-1, 0.8);
    glTexCoord2f(1,1);  glVertex3f(+1,+1, 0.8);
    glTexCoord2f(0,1);        glVertex3f(-1,+1, 0.8);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D,texture[3]);
    glBegin(GL_QUADS);
    
    //  Screen Bezel
    
    //Bottom Bezel
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,-1, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,-1, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,-0.8, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-0.8, 1);
    //Bottom Bezel Indent
    glColor3f(color1,color2,color3);
    glNormal3f(0,1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-0.8, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,-0.8, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,-0.8, 0.8);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-0.8, 0.8);
    //Top Bezel
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,+.8, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,+.8, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+1, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+1, 1);
    //Top Bezel Indent
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,+0.8, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,+0.8, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+0.8, 0.8);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+0.8, 0.8);
    //Left Bezel
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,-1, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.8,-1, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.8,+1, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+1, 1);
    //Left Bezel Indent
    glColor3f(color1,color2,color3);
    glNormal3f(1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-0.8,-1, 0.8);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.8,-1, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.8,+1, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-0.8,+1, 0.8);
    //Right Bezel
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(+0.8,-1, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,-1, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+1, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(+0.8,+1, 1);
    //Right Bezel Indent
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(0.8,-1, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(0.8,-1, 0.8);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(0.8,+1, 0.8);
    glTexCoord2f(0,wallrep);        glVertex3f(0.8,+1, 1);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(+1,-1,-1);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,-1,-1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,+1,-1);
    glTexCoord2f(0,wallrep);        glVertex3f(+1,+1,-1);
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(+1,-1,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,-1,-1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+1,-1);
    glTexCoord2f(0,wallrep);        glVertex3f(+1,+1,+1);
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-1,-1);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,-1,+1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,+1,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+1,-1);
    //  Top
    glColor3f(color1,color2,color3);
    glNormal3f(0,+1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,+1,+1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,+1,+1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,+1,-1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,+1,-1);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-1,-1);
    glTexCoord2f(wallrep,0);        glVertex3f(+1,-1,-1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(+1,-1,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-1,+1);
    
    //  Feet
    
    //Foot One
    
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,-1, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-1, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-1.2, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-1.2, 1);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(-1,-1,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-1,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-1.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-1.2,0.9);
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-0.9,-1,1);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.9,-1,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.9,-1.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-0.9,-1.2,1);
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-1,1);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,-1,+0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,-1.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-1.2,1);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-1.2,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-1.2,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-1.2,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-1.2,+1);
    
    
    
    //Foot Two
    
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(1,-1, 1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-1, 1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-1.2, 1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-1.2, 1);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(1,-1,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-1,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-1.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-1.2,0.9);
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(0.9,-1,1);
    glTexCoord2f(wallrep,0);        glVertex3f(0.9,-1,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(0.9,-1.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(0.9,-1.2,1);
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(1,-1,1);
    glTexCoord2f(wallrep,0);        glVertex3f(1,-1,+0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(1,-1.2,0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-1.2,1);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(1,-1.2,0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-1.2,0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-1.2,+1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-1.2,+1);
    
    
    //Foot Three
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(-1,-1, -1);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-1, -1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-1.2, -1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-1.2, -1);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(-1,-1,-0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-1,-0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-1.2,-0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-1.2,-0.9);
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-0.9,-1,-1);
    glTexCoord2f(wallrep,0);        glVertex3f(-0.9,-1,-0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-0.9,-1.2,-0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-0.9,-1.2,-1);
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-1,-1);
    glTexCoord2f(wallrep,0);        glVertex3f(-1,-1,-0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-1,-1.2,-0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-1.2,-1);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(-1,-1.2,-0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(-.9,-1.2,-0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(-.9,-1.2,-1);
    glTexCoord2f(0,wallrep);        glVertex3f(-1,-1.2,-1);
    
    
    
    //Foot Four
    //  Front
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,+1);
    glTexCoord2f(0,0);              glVertex3f(1,-1, -1);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-1, -1);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-1.2, -1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-1.2, -1);
    //  Back
    glColor3f(color1,color2,color3);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0);              glVertex3f(1,-1,-0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-1,-0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-1.2,-0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-1.2,-0.9);
    //  Right
    glColor3f(color1,color2,color3);
    glNormal3f(+1,0,0);
    glTexCoord2f(0,0);              glVertex3f(0.9,-1,-1);
    glTexCoord2f(wallrep,0);        glVertex3f(0.9,-1,-0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(0.9,-1.2,-0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(0.9,-1.2,-1);
    //  Left
    glColor3f(color1,color2,color3);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0);              glVertex3f(1,-1,-1);
    glTexCoord2f(wallrep,0);        glVertex3f(1,-1,-0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(1,-1.2,-0.9);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-1.2,-1);
    //  Bottom
    glColor3f(color1,color2,color3);
    glNormal3f(0,-1,0);
    glTexCoord2f(0,0);              glVertex3f(1,-1.2,-0.9);
    glTexCoord2f(wallrep,0);        glVertex3f(.9,-1.2,-0.9);
    glTexCoord2f(wallrep,wallrep);  glVertex3f(.9,-1.2,-1);
    glTexCoord2f(0,wallrep);        glVertex3f(1,-1.2,-1);
    glEnd();
    
    glPopMatrix();
    
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
    //  Erase the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //  Enable Z-buffering in OpenGL
    glEnable(GL_DEPTH_TEST);

    //  Undo previous transformations
    glLoadIdentity();
    
    //  Camera Update
    glRotatef(XRotation,1.0,0.0,0.0);  //Rotate on X axis
    glRotatef(YRotation,0.0,1.0,0.0);  //Rotate on Y axis
    glTranslated(-PlayerX,-PlayerY,-PlayerZ); //Move the camera to the updated camera position

    //  Flat or smooth shading
    glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);


    //  Translate intensity to color vectors
    float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
    float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
    float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
    //  Light position
    float Position[]  = {distance*Sin(zh),distance*Cos(zh),ylight,1.0};
    //  Draw light position as ball (still no lighting here)
    glColor3f(1,1,1);
    ball(Position[0],Position[1],Position[2] , 0.1);
    //  OpenGL should normalize normal vectors
    glEnable(GL_NORMALIZE);
    //  Enable lighting
    glEnable(GL_LIGHTING);
    //  Location of viewer for specular calculations
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
    //  glColor sets ambient and diffuse color materials
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    //  Enable light 0
    glEnable(GL_LIGHT0);
    //  Set ambient, diffuse, specular components and position of light 0
    glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
    glLightfv(GL_LIGHT0,GL_POSITION,Position);


    

    Rain();

    house(0,1.1,0 , 2,2,2 , 0 , 1,1,1);

    tvset(-1.3,-.4,-1.3 , 0.3,0.3,0.3 , 45, 1,1,1);
    ground(0,-1,0 , 100,1,100, 0, 1,1,1);
    table(-.7,-.7,-.7 , .45,.45,.45 , 45 , 1,1,1);
    sofa(.3,-.7,.3 , .6,.6,.6 , 225 , 1,1,1);
    bed(1,-.7,1.2 , .6,.6,.6 , 90 , 1,1,1);
    lamp(1,-.7,-1 , .5,.8,.5 , 0 , 1,1,1);
    

    //  Render the scene and make it visible
    ErrCheck("display");
    glFlush();
    glutSwapBuffers();
}



/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
    //  Elapsed time in seconds
    double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
    zh = fmod(90*t*.1,360.0);
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}


/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
    //  Exit on ESC
    if (ch == 27)
    {
        exit(0);
    }
    //  Move forward
    else if (ch == 'w')
    {
        PlayerZ -= .1;
    }
    //  Move Backward
    else if (ch == 's')
    {
        PlayerZ += .1;
        
    }
    //  Strafe Left
    else if (ch == 'a')
    {
        PlayerX -= .1;
    }
    //  Strafe Right
    else if (ch == 'd')
    {
        PlayerX += .1;
    }
        
    //  Reproject
    Project(mode?fov:0,asp,dim);
    //  Animate if requested
    glutIdleFunc(move?idle:NULL);
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the mouse is moved
 */
void mouseMovement(int x, int y) {
    int XDifference=x-OldX; //  Find the difference between the current mouse X position, and the previously recorded mouse position
    int YDifference=y-OldY; //  Find the difference between the current mouse Y position, and the previously recorded mouse position
    OldX=x; //  Record the current mouse X position for the next calculation of XDifference
    OldY=y; //  Record the current mouse Y position for the next calculation of YDifference
    XRotation += YDifference; //    Rotate the camera on the x axis based on the changes in the mouse's Y position
    YRotation += XDifference; //    Rotate the camera on the y axis based on the changes in the mouse's X position
    if (XRotation > 90)
    {
        XRotation = 90;
    }
    else if (XRotation < -90)
    {
        XRotation = -90;
    }

}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
    //  Ratio of the width to the height of the window
    asp = (height>0) ? (double)width/height : 1;
    //  Set the viewport to the entire window
    glViewport(0,0, width,height);
    //  Set projection
    Project(mode?fov:0,asp,dim);
}


/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
    //  Initialize GLUT
    glutInit(&argc,argv);
    //  Request double buffered, true color window with Z buffering at 600x600
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(1000,1000);
    glutCreateWindow("finalproject");
    //  Set callbacks
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouseMovement);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);
    texture[0] = LoadTexBMP("wall.bmp");
    texture[1] = LoadTexBMP("door.bmp");
    texture[2] = LoadTexBMP("chimney.bmp");
    texture[3] = LoadTexBMP("door_side.bmp");
    texture[4] = LoadTexBMP("roof.bmp");
    texture[5] = LoadTexBMP("grass.bmp");
    texture[6] = LoadTexBMP("tvscreen.bmp");
    texture[7] = LoadTexBMP("sofa.bmp");
    texture[8] = LoadTexBMP("bedsheet.bmp");
    texture[9] = LoadTexBMP("sheetcover.bmp");
    texture[10] = LoadTexBMP("interiorwall.bmp");
    texture[11] = LoadTexBMP("lampshade.bmp");
    texture[12] = LoadTexBMP("carpet.bmp");
    //  Pass control to GLUT so it can interact with the user
    ErrCheck("init");
    glutMainLoop();
    return 0;
}
