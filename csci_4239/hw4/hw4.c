/*
 *  3D Objects
 *  Alexander Curtiss
 *  
 *  This program is based off the ex9.c file, located at (http://www.prinmath.com/csci5229/F15/programs/ex9.zip)
 *  Any sections that have remained unchanged from the original have been marked as such.
 *
 *  Key bindings:
 *  arrow keys Change view angle - also change camera position in overview mode
 *  w/a/s/d    Move camera forward, backward, left, and right while in third person view
 *  q/e        Move camera up and down while in third person view
 *  m          Change mode - cycles through:
 *                (0) orthogonal overview
 *                (1) perspective overview
 *                (2) perspective first-person
 *  0          Reset view angle
 *  ESC        Exit
 */
 
 //  Number of particles
#define TERRAIN_SIZE 256
#define TERRAIN_SCALE 2

//  Modes
#define MODE_COUNT 3
#define PARTICLES 0
#define BUMP_MAP 1
#define WATER 2
#define SHADER_COUNT 5


//  Texture handles
#define DEFAULT 0
#define VELOCITY 1
#define POSITION 2
#define WIND 3
#define TERRAIN 4
#define CHOPPY_WATER 5
#define SMOOTH_WATER 6
#define ROCK 7

//  Attribute array handle
#define PARTICLE_ARRAY 4
 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES

#include <GL/glut.h>
#include "CSCIx239.h"

int axes=1;       //  Display axes
int mode=0;       //  Projection mode
int th=220;       //  Azimuth of view angle
int ph=-20;       //  Elevation of view angle
int fov=55;       //  Field of view
double asp=1;     //  Aspect ratio
double dim=5;     //  Size of world
double camX = 7;  //  Camera x
double camY = 5;  //  Camera y
double camZ = 7;  //  Camera z
double step = 0.2;//  Size of step in third person view

/*
 *  Set projection
 *
 *  Unchanged from original method in ex9.
 */
static void P()
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   
   gluPerspective(fov,asp,dim/4,4*dim);
   
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

/*
 *  Legs are used for both table and chair objects
 */
static void leg(double x,double y,double z) {
   glBegin(GL_QUADS);
   // Cap on the bottom
   glColor3f(0.3,0.5,0.9);
   glVertex3d(x - 0.1, 0, z + 0.1);
   glVertex3d(x + 0.1, 0, z + 0.1);
   glVertex3d(x + 0.1, 0, z - 0.1);
   glVertex3d(x - 0.1, 0, z - 0.1);
   
   // Sides
   glColor3f(0.3,0.5,0.9);
   glVertex3d(x - 0.1, 0, z + 0.1);
   glVertex3d(x - 0.1, 2, z + 0.1);
   glVertex3d(x + 0.1, 2, z + 0.1);
   glVertex3d(x + 0.1, 0, z + 0.1);
   
   glVertex3d(x + 0.1, 0, z - 0.1);
   glVertex3d(x + 0.1, 2, z - 0.1);
   glVertex3d(x - 0.1, 2, z - 0.1);
   glVertex3d(x - 0.1, 0, z - 0.1);
   
   glColor3f(0.6,0.6,0.9);
   glVertex3d(x + 0.1, 0, z - 0.1);
   glVertex3d(x + 0.1, 2, z - 0.1);
   glVertex3d(x + 0.1, 2, z + 0.1);
   glVertex3d(x + 0.1, 0, z + 0.1);
   
   glVertex3d(x - 0.1, 0, z - 0.1);
   glVertex3d(x - 0.1, 2, z - 0.1);
   glVertex3d(x - 0.1, 2, z + 0.1);
   glVertex3d(x - 0.1, 0, z + 0.1);
   glEnd();
}

/*
 *  Make a chair at given coordinates with given rotation
 */
static void chair(double x,double y,double z,
                 double dx,double dy,double dz, double zh,
                 double th, double ph)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(zh,0,0,1);
   glRotated(th,0,1,0);
   glRotated(ph,1,0,0);
   glScaled(dx,dy,dz);
   
   // Chair legs
   leg(1,0,1);
   leg(1,0,-1);
   leg(-1,0,-1);
   leg(-1,0,1);
   
   glBegin(GL_QUADS);
   // Top of chair
   glColor3f(0.3,0.5,0.9);
   glVertex3d(1.1, 2, -1.1);
   glVertex3d(1.1, 2, 1.1);
   glVertex3d(-1.1, 2, 1.1);
   glVertex3d(-1.1, 2, -1.1);
   
   // Bottom of chair
   glColor3f(0.6,0.6,0.9);
   glVertex3d(1.1, 2.2, -1.1);
   glVertex3d(1.1, 2.2, 1.1);
   glVertex3d(-1.1, 2.2, 1.1);
   glVertex3d(-1.1, 2.2, -1.1);
   
   // Sides of chair   
   glColor3f(0.3,0.5,0.9);
   glVertex3d(1.1, 2, -1.1);
   glVertex3d(-1.1, 2, -1.1);
   glVertex3d(-1.1, 2.2, -1.1);
   glVertex3d(1.1, 2.2, -1.1);
   
   glVertex3d(-1.1, 2.2, 1.1);
   glVertex3d(-1.1, 2, 1.1);
   glVertex3d(1.1, 2, 1.1);
   glVertex3d(1.1, 2.2, 1.1);
   
   glColor3f(0.6,0.6,0.9);
   glVertex3d(-1.1, 2, -1.1);
   glVertex3d(-1.1, 2, 1.1);
   glVertex3d(-1.1, 2.2, 1.1);
   glVertex3d(-1.1, 2.2, -1.1);

   glVertex3d(1.1, 2, -1.1);
   glVertex3d(1.1, 2, 1.1);
   glVertex3d(1.1, 2.2, 1.1);
   glVertex3d(1.1, 2.2, -1.1);
   
   // Back of chair
   glColor3f(0.6,0.6,0.9);
   glVertex3d(-1.1, 2.2, -1.1);
   glVertex3d(-1.1, 4.0, -1.1);
   glVertex3d(-1.1, 4.0, 1.1);
   glVertex3d(-1.1, 2.2, 1.1);
   
   glVertex3d(-0.9, 2.2, -1.1);
   glVertex3d(-0.9, 4.0, -1.1);
   glVertex3d(-0.9, 4.0, 1.1);
   glVertex3d(-0.9, 2.2, 1.1);
   
   glVertex3d(-1.1, 2.2, -1.1);
   glVertex3d(-1.1, 4.0, -1.1);
   glVertex3d(-0.9, 4.0, -1.1);
   glVertex3d(-0.9, 2.2, -1.1);
   
   glVertex3d(-1.1, 2.2, 1.1);
   glVertex3d(-1.1, 4.0, 1.1);
   glVertex3d(-0.9, 4.0, 1.1);
   glVertex3d(-0.9, 2.2, 1.1);
   
   glVertex3d(-1.1, 4.0, -1.1);
   glVertex3d(-0.9, 4.0, -1.1);
   glVertex3d(-0.9, 4.0, 1.1);
   glVertex3d(-1.1, 4.0, 1.1);
   
   glEnd();   
   //  Undo transformations
   glPopMatrix();
}

/*
 *  Make a table at the specified coordinates with the specified rotation
 */
static void table (double x,double y,double z,
                 double dx,double dy,double dz, double zh,
                 double th, double ph)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(zh,0,0,1);
   glRotated(th,0,1,0);
   glRotated(ph,1,0,0);
   glScaled(dx,dy,dz);
   
   // Table legs
   leg(1,0,1);
   leg(1,0,-1);
   leg(-1,0,-1);
   leg(-1,0,1);
   
   // Table edge
   double k;
   glBegin(GL_QUAD_STRIP);
   for (k=0;k<=360;k+=1)
   {
	  glColor3f(Cos(k)*Cos(k) / 1.5 , 0.4 , Sin(k)*Sin(k) / 1.5);
      glVertex3d(2 * Sin(k), 2 , 2 * Cos(k));
	  glVertex3d(2 * Sin(k), 2.2 , 2 * Cos(k));
   }
   glEnd();
   
   // Table bottom
   glBegin(GL_TRIANGLE_FAN);
   glVertex3d(0,2,0);
   for (k=0;k<=360;k+=1)
   {
	  glColor3f(Cos(k)*Cos(k) / 2 , 0.25 , Sin(k)*Sin(k) / 2);
      glVertex3d(2 * Sin(k), 2 , 2 * Cos(k));
   }
   glEnd();
   
   // Table top
   glBegin(GL_TRIANGLE_FAN);
   glVertex3d(0,2.2,0);
   for (k=0;k<=360;k+=1)
   {
	   glColor3f(Cos(k)*Cos(k) , 0.5 , Sin(k)*Sin(k));
      glVertex3d(2 * Sin(k), 2.2 , 2 * Cos(k));
   }
   glEnd();
   
   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len=4.0;  //  Length of axes
   //  Erase the window and the DEPTH buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   //glLoadIdentity();
   //  Perspective - set eye position

  
      gluLookAt(camX,camY,camZ, camX + (Sin(th) * Cos(ph)),camY + Sin(ph),camZ + (Cos(th) * Cos(ph)), 0,Cos(ph),0);
     
   
   chair(1.5,0,-2.3, .5,.5,.5, 0,-70,0);
   chair(-0.6,0,1.8, .7,.7,.7, 0,45,0);
   table(1,0,0, 1,1,1, 0,0,0);
   
   //  Draw axes
   glColor3f(1,1,1);
   if (axes)
   {
      glBegin(GL_LINES);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(len,0.0,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,len,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,0.0,len);
      glEnd();
      //  Label axes
      /*glRasterPos3d(len,0.0,0.0);
      Print("X");
      glRasterPos3d(0.0,len,0.0);
      Print("Y");
      glRasterPos3d(0.0,0.0,len);
      Print("Z");
      */
   }
   
   //  Render the scene and make it visible
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
   {
      th -= 5;
   }
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
   {
      th += 5;
   }
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
   {
      ph += 5;
   }
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
   {
      ph -= 5;
   }
   
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   
   //  Update projection
   P();
   
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Switches controls when upside-down
   int direction=Cos(ph)>0?1:-1;
   
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   
   //  Reset view angle
   else if (ch == '0')
   {
      camX = 7;
      camY = 5;
      camZ = 7;
      th=220;
      ph=-20;
   }
   
   //  Move first person camera - switched when upside-down
   //  W - forward
   else if (ch == 'w' || ch == 'W') 
   {
      camX += Sin(th)*step*direction;
      camZ += Cos(th)*step*direction;
   }
   //  S - back
   else if (ch == 's' || ch == 'S') 
   {
		camX -= Sin(th)*step*direction;
      camZ -= Cos(th)*step*direction;
   }
   //  A - left
   else if (ch == 'a' || ch == 'A') 
   {
		camX += Cos(th)*step*direction;
      camZ -= Sin(th)*step*direction;
   }
   //  D - right
   else if (ch == 'd' || ch == 'D') 
   {
      camX -= Cos(th)*step*direction;
      camZ += Sin(th)*step*direction;
   }
   //  Q - down
   else if (ch == 'q' || ch == 'Q')
   {
      camY -= 4 * step*direction;
   }
   //  E - up
   else if (ch == 'e' || ch == 'E') 
   {
      camY += 4 * step*direction;
   }
   
   //  Switch display mode
   else if (ch == 'm' || ch == 'M') {
      mode = (mode + 1) % 3;
      if(mode == 2){
         camX = -Sin(th) * 10 * Cos(ph);
         camZ = -Cos(th) * 10 * Cos(ph);
         camY = -Sin(ph) * 10;
      }
   }
   
   //  Reproject
   P();
   
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
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
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   
   gluPerspective(fov,asp,dim/4,4*dim);
   
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
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
   glutInitWindowSize(600,600);
   glutCreateWindow("Alexander Curtiss: Projections");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
