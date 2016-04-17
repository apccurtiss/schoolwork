#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <GL/glut.h>

/*
 *  Draws a tree at selected location
 */
static void drawTree(float x, float y, float z, float w, float h, float deform, int deformAngle, unsigned int leafTex) {
   
   glPushMatrix();
   glTranslated(x,0,z);
   glColor3f(0.3 , 0.6 , 0.3);
   glEnable(GL_TEXTURE_2D);
   glNormal3d(0,1,0);

   int th;
   float width, height;
   glBindTexture(GL_TEXTURE_2D,leafTex);
   for (height = y; height < h; height += 0.6)
   {
      glPushMatrix();
      //  Deformation used to allow tree bending
      glTranslated(Sin(deform) * height,0,0);
      glRotated(deformAngle,0,1,0);
      glRotated(deform * height, 1, 0, 1);
      
      //  Width slowly tapers with height
      width = w / sqrt(height);
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=40)
      {
         glNormal3d(Cos(th),0,Sin(th));
         glTexCoord2d(0,0);
         glVertex3d(0,height,0);
         glTexCoord2d(1,1);
         glVertex3d(Cos(th + 40) * width,height - 1,Sin(th + 40) * width);
         glTexCoord2d(0,1);
         glVertex3d(Cos(th + 40) * width,height - 1,Sin(th + 40) * width);
         glTexCoord2d(1,0);
         glVertex3d(0,height,0);
      }
      glEnd();
      glPopMatrix();
   }
   
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

/*
 * Taken directly from ex18.c, with minor modifications
 */
static void Vertex(int th,int ph)
{
   double x =  -Sin(th)*Cos(ph);
   double y =  Sin(ph);
   double z =  Cos(th)*Cos(ph);
   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

/*
 *  A sphere drawn around the player to give fog something to cling to
 */
void skySphere(double x,double y,double z,double r)
{
   int ph, th;
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(r,r,r);
   
   glColor4f(1,1,1,1);
   for (ph=-90;ph<90;ph+=5)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=5)
      {
         Vertex(th, ph);
         Vertex(th,ph+5);
      }
      glEnd();
   }
   glPopMatrix();
}

