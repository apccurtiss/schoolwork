/*
 *  Alexander Curtiss
 */
#include "CSCIx229.h"
#include "environment.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SNOW_COUNT 2500
#define MAX_SPEED 0.001
#define FOG_DENSITY 0.2

//  Width and height
int w = 600;
int h = 600;

int starting = 1; //  Game has just begun
int landed = 1;   //  Helicopter has landed
int mode=1;       //  First person or orbital
int th=0;         //  Azimuth of view angle
int ph=-45;       //  Elevation of view angle
int fov=55;       //  Field of view
double asp=1;     //  Aspect ratio
double dim=3.0;   //  Size of world
int emission =   0;  // Emission intensity (%)
int ambient  =  30;  // Ambient intensity (%)
int diffuse  = 100;  // Diffuse intensity (%)
int specular =   0;  // Specular intensity (%)
float viewy  =  .6;  // Elevation of light
double speed = 0;    //  Speed of helicopter
float acc = 0;       //  Helicopter vertical motion
float heliPos[3]={0,0.2,0}; //  Helicopter position
float tick  = 0;            //  Game tick
float snowflakes[SNOW_COUNT][2];  //  Location of snowflakes  
float y[65][65];                  //  Terrain map
float ymin=+1e8;
float ymax=-1e8;
float ymag=1;
//  Used to process animations and delayed events, like vehicle roll
float animationTick = 0, rollTick  = 0, pitchTick = 0, turnTick = 0, speedTick = 0;
//  Textures
unsigned int leafTex, ground;
//  Models - loaded seperately for animations and coloring
int body, prop, doors;
//  Used to tell when keys are pressed
int up = 0, down = 0, left = 0, right = 0, faster = 0, slower = 0;

typedef struct{
   float x;
   float y;
   float z;
   float w;
   float h;
} tree;

//  Used to generate the map
tree trees[] = {
//  X, Y, Z, Width, Height
{-1,0,1,1,4},
{2,0,1,0.9,3},
{-1,0,-3,1,2},
{-3,0,-3,1.3,2},
{2,0,-1.8,0.8,2.8},
};
int treeSize = sizeof(trees) / sizeof(*trees); //  Used so often, it helps to figure it out now

/*
 *  Draws the ground - taken from ex17 with slight changes
 */
void drawGround() {
   int i,j;
   float y0 = (ymin+ymax)/2;
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,ground);
   glPushMatrix();
   for (i=0;i<64;i++) {
      for (j=0;j<64;j++) {
         float x=16*i-512;
         float z=16*j-512;
         glBegin(GL_QUADS);
         glTexCoord2f((i+0)/64.,(j+1)/64.); glVertex3d(x+ 0,ymag*(y[i+0][j+1]-y0),z+16);
         glTexCoord2f((i+1)/64.,(j+1)/64.); glVertex3d(x+16,ymag*(y[i+1][j+1]-y0),z+16);
         glTexCoord2f((i+1)/64.,(j+0)/64.); glVertex3d(x+16,ymag*(y[i+1][j+0]-y0),z+0);
         glTexCoord2f((i+0)/64.,(j+0)/64.); glVertex3d(x+ 0,ymag*(y[i+0][j+0]-y0),z+0);
         
         glEnd();
      }
   }
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

/*
 *  Draws parts of the display and the snow as an overlay - taken from ex20 with major changes
 */
void overlay()
{
   //  Project as orthoganal
   glPushAttrib(GL_TRANSFORM_BIT|GL_ENABLE_BIT);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(-asp,+asp,-1,1,-1,1);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glDisable(GL_DEPTH_TEST);
   
   //  Speed indicator
   glBegin(GL_QUADS);
   glColor3f(0.1,0.2,0.5);
   glVertex2f(-0.925,-0.5);
   glVertex2f(-0.775,-0.5);
   glColor3f(0.2,0.4,0.6);
   glVertex2f(-0.8,-0.9);
   glVertex2f(-0.9,-0.9);
   
   glColor3f(0.3,0.5,0.7);
   glVertex2f(-0.8 + (0.025 * speed / MAX_SPEED),-0.9 + (0.4 * speed / MAX_SPEED));
   glVertex2f(-0.8,-0.9);
   glVertex2f(-0.9,-0.9);
   glVertex2f(-0.9 - (0.025 * speed / MAX_SPEED),-0.9 + (0.4 * speed / MAX_SPEED));
   glEnd();
   
   //  Compass
   int i;
   glLineWidth(2.5);
   glColor3f(0, 0, 0);
   glBegin(GL_LINES);
   for(i = 0; i <= 360; i+=5){
      glVertex2f(0.8 + Sin(i) / 10, 0.8 + Cos(i) / 10);
      glVertex2f(0.8 + Sin(i + 5) / 10, 0.8 + Cos(i + 5) / 10);
   }
   glVertex2f(0.8 + Sin(th) / 10, 0.8 + Cos(th) / 10);
   glVertex2f(0.8, 0.8);
   glEnd();
   
   //  Snowflakes
   glColor3f(1,1,1);
   glBegin(GL_POINTS);
   for(i = 0; i < SNOW_COUNT; i++) {
      glVertex2f(snowflakes[i][0],snowflakes[i][1]);
   }
   glEnd();
   
   //  Reset everything
   glEnable(GL_DEPTH_TEST);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glPopAttrib();
}

void display()
{
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   glLoadIdentity();
   glShadeModel(GL_SMOOTH);
   
   //  First person perspective
   viewy = 0.6 + heliPos[1] * 0.5;
   if(mode)
      gluLookAt(heliPos[0] - Cos(th) * 2.5,heliPos[1] + 1 + speed * 70,heliPos[2] - Sin(th) * 2.5, heliPos[0],heliPos[1],heliPos[2], 0,1,0);
   //  Orbital view
   else
      gluLookAt(-Sin(th) * Cos(ph) * 4,-Sin(ph) * 4,-Cos(th) * Cos(ph) * 4, 0,0,0, 0,Cos(ph),0);
   
   //  Light
   float Ambient[]  = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
   float Diffuse[]  = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
   float Specular[] = {0.01*specular,0.01*specular,0.01*specular,1.0};
   float Position[] = {10,0,2,1.0};
   float white[]    = {1,1,1,1};
   glColor3f(1,1,1);
   glEnable(GL_LIGHT0);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);
   glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
   glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
   glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
   glLightfv(GL_LIGHT0,GL_POSITION,Position);
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,32);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   
   //  Ground is about 100x larger than the rest of the scene, hence the scaling
   glPushMatrix();
   glScaled(0.01,0.01,0.01);
   drawGround();
   glPopMatrix();
   
   //  Draw all the trees
   int i;
   for(i = 0; i < treeSize; i++) {
      drawTree(trees[i].x, trees[i].y + 0.3, trees[i].z, trees[i].w, trees[i].h, sin(animationTick / i) * 2, 0, leafTex);
   }
   
   //  Draw a sphere that the fog can hide the sky with
   skySphere(heliPos[0],heliPos[1],heliPos[2],5);
   
   //  Draw the player
   int pitch = pitchTick;
   int roll = rollTick;
   glPushMatrix();
   //  Body
   glColor3f(0.3,0.3,0.3);
   glTranslated(heliPos[0],heliPos[1],heliPos[2]);
   glRotated(-th - 90 + turnTick, 0,1,0);
   glRotated(pitch,1,0,0);
   glRotated(-roll,0,0,1);
   glScaled(0.002,0.002,0.002);
   glCallList(body);
   
   //  Rotor blades
   glPushMatrix();
   glRotated(animationTick * speedTick * 1000000, 0, 1, 0);
   glCallList(prop);
   glPopMatrix();
   
   //  Transparent windows
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE);
   glDepthMask(0);
   glColor4f(0.8,0.8,1,0.2);
   glCallList(doors);
   glDepthMask(1);
   glDisable(GL_BLEND);
   glPopMatrix();
   glDisable(GL_LIGHTING);
   glDisable(GL_NORMALIZE);
   glDisable(GL_LIGHT0);
   
   //  Display any text
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);
   glWindowPos2i(15,10);
   glColor3f(1,1,1);
   Print("Speed");
   glEnable(GL_DEPTH_TEST);
   if(starting){
      glWindowPos2i(h/2-180,120);
      Print("Press 'w' to speed up and 'q' to slow down.");
      glWindowPos2i(h/2-125,100);
      Print("Use the arrow keys to navigate!");
   }
   
   //  Draw snow, overlay, and fog
   overlay();
   glFogf(GL_FOG_DENSITY, FOG_DENSITY);
   
   //  Clean up
   glEnable(GL_LIGHTING);
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 * Controls game actions such as collisions and motion
 */
void idle()
{
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   int obstructedx = 0, obstructedz = 0;
   //  Used to cap game speed - only tick if it's been 0.01 second since the last tick.
   if(t > tick + .01) {
      tick = t;
      //  Used to control animations and other events!
      animationTick+= 0.01;
      
      float step = 0.035*Sin(-pitchTick);  //  How far to move per tick
      int i;
      
      //  Checks horizontal clipping
      if (up || down) {
         for(i=0; i < treeSize; i++) {
            if(fabs(trees[i].y - heliPos[1]) < trees[i].h - 0.2) //  Checks y-axis
            {
               //  Checks for x-axis clipping
               if(fabs(trees[i].x - heliPos[0]) > trees[i].w
               && fabs(trees[i].x - heliPos[0] - Cos(th)*step) < trees[i].w
               && fabs(trees[i].z - heliPos[2]) <= trees[i].w - 0.5)
                  obstructedx = 1;
               //  Checks for z-axis clipping
               if(fabs(trees[i].z - heliPos[2]) > trees[i].w
               && fabs(trees[i].z - heliPos[2] - Sin(th)*step) < trees[i].w
               && fabs(trees[i].x - heliPos[0]) <= trees[i].w - 0.5)
                  obstructedz = 1;
               
            }
         }
      }
      
      float pitchAmount = 0.3;
      float rollAmount = 0.3;
      float turnAmount = 0.3;
      //  Only move if not clipping and in first-person mode
      if (right && (!landed || !mode)) {
         rollTick+=rollAmount * 2;
         turnTick+=turnAmount * 2;
         if(!starting && mode) {
            th += 2;
         } else {
            th += 2;
         }
      } if (left && (!landed || !mode)) {
         rollTick-=rollAmount * 2;
         turnTick-=turnAmount * 2;
         if(!starting && mode) {
            th -= 2;
         } else {
            th -= 2;
         }
      } if (up && (!landed || !mode)) {
         pitchTick-=pitchAmount * 2;
         if(!starting && mode) {
            if(!obstructedx)
               heliPos[0] += Cos(th)*step;
            if(!obstructedz)
               heliPos[2] += Sin(th)*step;
         } else {
            ph -= 2;
         }
      }
      if (down && (!landed || !mode)) {
         pitchTick+=pitchAmount * 2;
         if(!starting && mode) {
            if(!obstructedx)
               heliPos[0] += Cos(th)*step;
            if(!obstructedz)
               heliPos[2] += Sin(th)*step;
         } else {
            ph += 2;
         }
      }
      
      //  Slowly resets rotation back to 0
      if (pitchTick > pitchAmount)
         pitchTick-=pitchAmount;
      else if (pitchTick < -pitchAmount)
         pitchTick+=pitchAmount;
      if (rollTick > rollAmount)
         rollTick-=rollAmount;
      else if (rollTick < -rollAmount)
         rollTick+=rollAmount;
      if (turnTick > turnAmount)
         turnTick-=turnAmount;
      else if (turnTick < -turnAmount)
         turnTick+=turnAmount;
      
      //  Caps rotations
      if(pitchTick > 45)
         pitchTick = 45;
      else if (pitchTick < -45)
         pitchTick = -45;
      if(rollTick > 25)
         rollTick = 25;
      else if (rollTick < -25)
         rollTick = -25;
      if(turnTick > 15)
         turnTick = 15;
      else if (turnTick < -15)
         turnTick = -15;
      
      ph %= 360;
      th %= 360;
      
      //  Adjusts rotor speed
      if (slower) speed -= 0.00001;
      if (faster) speed += 0.00001;
      
      //  Caps rotor speed
      if (speed > MAX_SPEED) speed = MAX_SPEED;
      if (speed < 0)        speed = 0;
      
      //  Allows for rotor speed to reduce slowly on it's own, rather than stopping immidiately
      speedTick *= 0.99;
      if (speedTick < speed) speedTick = speed;
      if (speedTick < 0.000001) speedTick = 0.000001;
      
      //  Calculate helicopter acceletation
      acc += (speed * Cos(rollTick) * Cos(pitchTick)) - 0.0007;
      if (acc < -0.1) acc = -0.1;
      if (acc > 0.01) acc = 0.01;
      if(acc > 0) landed = 0;
      
      //  Checks clipping on top of trees, albeit awkwardly
      for(i=0; i < treeSize; i++) {
         if(fabs(heliPos[0] - trees[i].x) <= trees[i].w
         && fabs(heliPos[2] - trees[i].z) <= trees[i].w)
         {
            if(heliPos[1] >= trees[i].y + trees[i].h
            && heliPos[1] + acc <= trees[i].y + trees[i].h)
            {
               landed = 1;
               acc = 0;
               break;
            }
         }
      }
      
      //  Adjusts helicopter position
      heliPos[1] += acc * (20 - heliPos[1] * 3) / 20;
      //  Cap height
      if(heliPos[1] > 3) {
         heliPos[1] = 3;
      }
      
      //  Prevents helicopter from leaving map
      if (heliPos[0] > 5 ) heliPos[0] = 5;
      if (heliPos[0] < -5) heliPos[0] = -5;
      if (heliPos[2] > 5 ) heliPos[2] = 5;
      if (heliPos[2] < -5) heliPos[2] = -5;
      
      //  Clips helicopter with terrain
      //  Converts between player position (-5 through 5) and map position (0 through 64)
      int mapX = heliPos[0] * 6.4 + 32 ;
      int mapZ = heliPos[2] * 6.4 + 32;
      float y0 = (ymin+ymax)/2;
      //  Clips with ground
      if (heliPos[1] < ymag*(y[mapX][mapZ]-y0) * 0.01) {
         heliPos[1] = ymag*(y[mapX][mapZ]-y0) * 0.01;
         landed = 1;
         acc = 0;
      }
      
      //  Updates snow positions
      for(i = 0; i < SNOW_COUNT; i++) {
         snowflakes[i][0] += Sin(animationTick + i) / h;
         snowflakes[i][1] -= (1 + Sin(animationTick - i)) / h + 0.01;
         if(snowflakes[i][1] < -1) {
            snowflakes[i][0] = (float)(rand() % (w * 2)) / (float)w - 1;
            snowflakes[i][1] = 1;
         }
      }
      
      glutPostRedisplay();
   }
}

/*
 * Checks keydown (used in idle())
 */
void special(int key,int x,int y)
{   
   if (key == GLUT_KEY_RIGHT)
      right = 1;
   else if (key == GLUT_KEY_LEFT)
      left = 1;
   else if (key == GLUT_KEY_UP)
      up = 1;
   else if (key == GLUT_KEY_DOWN)
      down = 1;
}

/*
 * Checks keyup (used in idle())
 */
void specialup(int key,int x,int y)
{
   if (key == GLUT_KEY_RIGHT)
      right = 0;
   else if (key == GLUT_KEY_LEFT)
      left = 0;
   else if (key == GLUT_KEY_UP)
      up = 0;
   else if (key == GLUT_KEY_DOWN) 
      down = 0;
}

/*
 * Checks keydown
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view on spacebar
   else if (ch==' ') {
      heliPos[0] = 0;
      heliPos[1] = 0;
      heliPos[2] = 0;
      acc        = 0;
      th = 0;
      speed = 0;
   }
   //  Change mode on m
   else if (ch=='m') {
      mode = 1-mode;
      th = th * (1 - mode);
      ph = -45;
      th = 135;
   }
   
   //  Speed up and slow down (used in idle())
   else if (ch=='q' || ch=='Q')
      slower = 1;
   else if (ch=='w' || ch=='W') 
      faster = 1;
   
   //  Start game on keypress
   if (starting)
      starting = 0;
}

/*
 *  Checks keyup (used in idle())
 */
void key_up(unsigned char ch,int x,int y)
{
   if (ch=='q' || ch=='Q') {
      slower = 0;
   }
   else if (ch=='w' || ch=='W') {
      faster = 0;
   }
}

/*
 * Taken from ex18, with very minor changes made
 */
void reshape(int width,int height)
{
   w = width;
   h = height;
   asp = (height>0) ? (double)width/height : 1;
   glViewport(0,0, width,height);
   Project(fov,asp,dim);
   glutPostRedisplay();
}

/*
 * Taken from ex17, with no changes made
 */
void ReadDEM(char* file)
{
   int i,j;
   FILE* f = fopen(file,"r");
   if (!f) Fatal("Cannot open file %s\n",file);
   for (j=0;j<=64;j++)
      for (i=0;i<=64;i++)
      {
         if (fscanf(f,"%f",&y[i][j])!=1) Fatal("Error reading saddleback.dem\n");
         if (y[i][j] < ymin) ymin = y[i][j];
         if (y[i][j] > ymax) ymax = y[i][j];
      }
   fclose(f);
}

/*
 *  Randomized snowflake positions and raises trees to terrain height
 */
void init() {
   int i;
   float y0 = (ymin+ymax)/2;
   for(i = 0; i < treeSize; i++) {
      int mapX = trees[i].x * 6.4 + 32 ;
      int mapZ = trees[i].z * 6.4 + 32;
      trees[i].y = ymag*(y[mapX][mapZ]-y0) * 0.01;
   }
   srand(time(NULL));
   for(i = 0; i < SNOW_COUNT; i++) {
      snowflakes[i][0] = (float)(rand() % (w * 2)) / (float)w - 1;
      snowflakes[i][1] = (float)(rand() % (h * 2)) / (float)h - 1;
   }
}

int main(int argc,char* argv[])
{
   glutInit(&argc,argv);
   glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_ALPHA);
   glEnable(GL_CULL_FACE);
   glEnable (GL_DEPTH_TEST);
   glutInitWindowSize(w,h);
   glutCreateWindow("Alexander Curtiss");
   
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutSpecialUpFunc(specialup);
   glutKeyboardFunc(key);
   glutKeyboardUpFunc(key_up); 
   glutIdleFunc(idle);
   
   //  Loads textures
   leafTex = LoadTexBMP("leaves.bmp");
   ground = LoadTexBMP("snow.bmp");
   //  Creates map
   ReadDEM("saddleback.dem");
   init();
   
   //  Sets up fog
   GLfloat fogColor[4] = {1, 1, 1, 1.0};
   glEnable (GL_FOG);
   glFogi (GL_FOG_MODE, GL_EXP2);
   glFogfv (GL_FOG_COLOR, fogColor);
   glHint (GL_FOG_HINT, GL_NICEST);
   
   //  Load objects
   body = LoadOBJ("body.obj");
   doors = LoadOBJ("doors.obj");
   prop = LoadOBJ("prop.obj");
   
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   
   glutMainLoop();
   return 0;
}