/*
 *  Final project test bed
 *  Alexander Curtiss
 *  
 *  Displays particules with attributes stored in textures
 *
 *  Key bindings:
 *  mouse      Change view angle
 *  m/M        Toggle between viewing particles and viewing their position texture
 *  a          Toggle axes
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */

#include "CSCIx239.h"

//  Number of particles
#define TERRAIN_SIZE 256
#define TERRAIN_SCALE 4

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

float axis_length=1;       //  What it says on the tin
int display_axis=1;        //  Display axes
int fov=55;                //  Field of view
int mode=0;                //  Change view
int th=0;                  //  Azimuth of view angle
int ph=0;                  //  Elevation of view angle
int n;                     //  Particle count
int W, H;                  //  Viewport size
int mX, mY;                //  Mouse coordinates
float tick = 0;            //  Used to control frame rate
double asp=1;              //  Aspect ratio
double dim=0.5;            //  Size of world
float wind_scale = 1.0;    //
int testval = 0;
float relative_x;
float relative_y;
int up_pressed = 0;
int down_pressed = 0;
int left_pressed = 0;
int right_pressed = 0;
int space_pressed = 0;
float height;
float user_position[3] = {0,0,0};
int shader[SHADER_COUNT] = {0,0,0,0,0}; //  Shader programs
unsigned int textures[5] = {0,0,0,0,0}; //  Texture location (first one unused)
unsigned int heightmap_texture;
float* heightmap;
char* text[] = {"Bump map","Particles","Water"};
char* Name[] = {"","","","","particle_numbers",NULL};
float particle_numbers[2*TERRAIN_SIZE*TERRAIN_SIZE]; //  Used to identify particles

GLuint terrain_element_buffer;

/*
 *  Get particles ready
 */
void init_particles(void)
{
   //  Each particle needs to know where on the texture to look, so this sets their coordinates
   float* particle_counter = particle_numbers;
   int i,j;
   n = TERRAIN_SIZE;
   for (i=0;i<n;i++) {
      float column = (float)i / (float)n;
      for (j=0;j<n;j++) {
         float row = (float)j / (float)n;
         
         *particle_counter++  = column;
         *particle_counter++  = row;
      }
   }
   
   unsigned int indices[(TERRAIN_SIZE-1)*(TERRAIN_SIZE-1)*6];
   
   unsigned int* index = indices;
   for(i = 0; i < TERRAIN_SIZE-1; i++) {
      for(j = 0; j < TERRAIN_SIZE-1; j++) {
         unsigned int current = (i * TERRAIN_SIZE) + j;
         *index++ = current;
         *index++ = current + 1;
         *index++ = current + 1 + TERRAIN_SIZE;   
         
         *index++ = current;
         *index++ = current + TERRAIN_SIZE;
         *index++ = current + 1 + TERRAIN_SIZE;
      }
   }
   
   glGenBuffers(1, &terrain_element_buffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_element_buffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, (TERRAIN_SIZE-1)*(TERRAIN_SIZE-1)*6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
}

/*
 *  Gets textures ready
 */
void init_textures() {
   //  Creates the heightmap texture
   LoadHeightmapBMP("heightmap.bmp", &heightmap, GL_TEXTURE0 + TERRAIN);
   LoadTexBMP("choppy_water_texture.bmp", GL_TEXTURE0 + CHOPPY_WATER);
   LoadTexBMP("smooth_water_texture.bmp", GL_TEXTURE0 + SMOOTH_WATER);
   LoadTexBMP("rock.bmp", GL_TEXTURE0 + ROCK);
   
   //  Creates 3D wind texture
   CreateNoise3D(GL_TEXTURE3);
   
   //  Stores values before they're made into a texture
   unsigned char* buf = malloc(TERRAIN_SIZE*TERRAIN_SIZE*4);
   int i,j;
   
   //  Velocity texture - begin as a constant
   for (i=0;i<TERRAIN_SIZE;i++) {
      for (j=0;j<TERRAIN_SIZE;j++) {
         int loc = (i + (TERRAIN_SIZE * j)) * 4;
         buf[loc+0] = 50;
         buf[loc+1] = 100;
         buf[loc+2] = 50;
         buf[loc+3] = 255;
      }
   }
   glActiveTexture(GL_TEXTURE0 + VELOCITY);
   //  Create texture location
   glGenTextures(1,&textures[VELOCITY]);
   //  Bind texture
   glBindTexture(GL_TEXTURE_2D,textures[VELOCITY]);
   //  Write texture
   glTexImage2D(GL_TEXTURE_2D,0,4,TERRAIN_SIZE,TERRAIN_SIZE,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
   if (glGetError()) Fatal("Error in glTexImage2D\n");
   //  Scale linearly when image size doesn't match
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   
   // Position texture - begin as random value
   for (i=0;i<TERRAIN_SIZE;i++) {
      for (j=0;j<TERRAIN_SIZE;j++) {
         int loc = (j + (TERRAIN_SIZE * i)) * 4;
         buf[loc+0] = rand()%256;
         buf[loc+1] = rand()%256;
         buf[loc+2] = rand()%256;
         buf[loc+3] = 255;
      }
   }
   glActiveTexture(GL_TEXTURE0 + POSITION);
   //  Create texture location
   glGenTextures(1,&textures[POSITION]);
   //  Bind texture
   glBindTexture(GL_TEXTURE_2D,textures[POSITION]);
   //  Write texture
   glTexImage2D(GL_TEXTURE_2D,0,4,TERRAIN_SIZE,TERRAIN_SIZE,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
   if (glGetError()) Fatal("Error in glTexImage2D\n");
   //  Scale linearly when image size doesn't match
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   
   free(buf);
}

/*
 *  Draw ground with heightmap
 */
void draw_terrain()
{
   int id;
   
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_TEXTURE_2D);
   glUseProgram(shader[3]);
   
   //  Pass textures to particle shader
   id = glGetUniformLocation(shader[3], "heightmap_texture");
   if (id>=0) glUniform1i(id,TERRAIN);
   id = glGetUniformLocation(shader[3], "rock_texture");
   if (id>=0) glUniform1i(id,ROCK);
   //id = glGetUniformLocation(shader[3], "testval");
   //if (id>=0) glUniform1i(id,testval);
   
   //  Pass textures to particle shader
   id = glGetUniformLocation(shader[3], "terrain_size");
   if (id>=0) glUniform1f(id,(float)TERRAIN_SIZE);
   id = glGetUniformLocation(shader[3], "terrain_scale");
   if (id>=0) glUniform1f(id,(float)TERRAIN_SCALE);
   
   
   //  Set particle size
   glPointSize(3);
   //  Point attribute array to local array
   glVertexAttribPointer(PARTICLE_ARRAY,2,GL_FLOAT,GL_FALSE,0,particle_numbers);
   //  Enable array used by DrawArrays
   glEnableVertexAttribArray(PARTICLE_ARRAY);
   //  Draw arrays
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_element_buffer);
   
   glDrawElements(GL_TRIANGLES, (TERRAIN_SIZE-1)*(TERRAIN_SIZE-1)*6, GL_UNSIGNED_INT, 0);
   
   glDisableVertexAttribArray(PARTICLE_ARRAY);
   
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_DEPTH_TEST);
}

/*
 *  Draw particles
 */
void draw_particles(void)
{
   int id;
   
   glEnable(GL_TEXTURE_2D);
   glUseProgram(shader[0]);
   
   //  Pass textures to particle shader
   id = glGetUniformLocation(shader[0], "velocity_texture");
   if (id>=0) glUniform1i(id,VELOCITY);
   id = glGetUniformLocation(shader[0], "position_texture");
   if (id>=0) glUniform1i(id,POSITION);
   id = glGetUniformLocation(shader[0], "wind_texture");
   if (id>=0) glUniform1i(id,WIND);
   
   //  Set particle size
   glPointSize(3);
   glVertexAttribPointer(PARTICLE_ARRAY,2,GL_FLOAT,GL_FALSE,0,particle_numbers);
   //  Point attribute array to local array
   //  Enable array used by DrawArrays
   glEnableVertexAttribArray(PARTICLE_ARRAY);
   //  Draw arrays
   glDrawArrays(GL_POINTS,0,TERRAIN_SIZE*TERRAIN_SIZE);
   
   glDisableVertexAttribArray(PARTICLE_ARRAY);
   
   glDisable(GL_TEXTURE_2D);
}

/*
 *  Draw ground with heightmap
 */
void draw_water()
{
   int id;
   
   glEnable(GL_TEXTURE_2D);
   glUseProgram(shader[4]);
   
   //  Pass textures to particle shader
   id = glGetUniformLocation(shader[4], "tick");
   if (id>=0) glUniform1f(id,tick);
   //  Pass textures to particle shader
   id = glGetUniformLocation(shader[4], "choppy_water");
   if (id>=0) glUniform1i(id,CHOPPY_WATER);
   //  Pass textures to particle shader
   id = glGetUniformLocation(shader[4], "smooth_water");
   if (id>=0) glUniform1i(id,SMOOTH_WATER);
   
   //  Set particle size
   glPointSize(3);
   //  Point attribute array to local array
   glVertexAttribPointer(PARTICLE_ARRAY,2,GL_FLOAT,GL_FALSE,0,particle_numbers);
   //  Enable array used by DrawArrays
   glEnableVertexAttribArray(PARTICLE_ARRAY);
   //  Draw arrays
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_element_buffer);
   glDrawElements(GL_TRIANGLES, (TERRAIN_SIZE-1)*(TERRAIN_SIZE-1)*6, GL_UNSIGNED_INT, 0);
   
   glDisableVertexAttribArray(PARTICLE_ARRAY);
   
   glDisable(GL_TEXTURE_2D);
}

/*
 *  Redraw textures to update positions and velocities
 */
void update_textures(int sh) {
   /*int id;
   glEnable(GL_TEXTURE_2D);
   
   //  Set shader
   glUseProgram(shader[sh]);
   //  Pass textures
   id = glGetUniformLocation(shader[sh], "velocity_texture");
   if (id>=0) glUniform1i(id,VELOCITY);
   id = glGetUniformLocation(shader[sh], "position_texture");
   if (id>=0) glUniform1i(id,POSITION);
   id = glGetUniformLocation(shader[sh], "scale");
   if (id>=0) glUniform1i(id,wind_scale);
   //  Disable depth
   glDisable(GL_DEPTH_TEST);
   //  Identity projections
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   //  Draw square on screen to display updated texture
   glBegin(GL_QUADS);
   glTexCoord2f(0,0); glVertex2f(-1,-1);
   glTexCoord2f(0,1); glVertex2f(-1,+1);
   glTexCoord2f(1,1); glVertex2f(+1,+1);
   glTexCoord2f(1,0); glVertex2f(+1,-1);
   glEnd();
   
   //  Re-render updated texture to original
   glBindTexture(GL_TEXTURE_2D,textures[sh]);
   glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,0,0,W,H,0);
   
   //  Clear screen for the rest of the image
   glClear(GL_COLOR_BUFFER_BIT);
   
   //  Reset defaults
   glUseProgram(shader[DEFAULT]);
   glDisable(GL_TEXTURE_2D);
   */
}

void draw_axis() {
   glDisable(GL_TEXTURE_2D);
   glUseProgram(0);
   glColor3f(1,1,1);
   
   glBegin(GL_LINES);
   glVertex3d(0.0,0.0,0.0);
   glVertex3d(axis_length,0.0,0.0);
   glVertex3d(0.0,0.0,0.0);
   glVertex3d(0.0,axis_length,0.0);
   glVertex3d(0.0,0.0,0.0);
   glVertex3d(0.0,0.0,axis_length);
   glEnd();
   
   //  Label axes
   glRasterPos3d(axis_length,0.0,0.0);
   Print("X");
   glRasterPos3d(0.0,axis_length,0.0);
   Print("Y");
   glRasterPos3d(0.0,0.0,axis_length);
   Print("Z");
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   
   //  Update the textures
   //update_textures(POSITION);
   //  Bugged
   //update_textures(VELOCITY);
   
   glLoadIdentity();
   
   user_position[1] = height + 0.2;
   double user_x = user_position[0];//Sin(th)*Cos(ph);
   double user_y = user_position[1];//        *Sin(ph);
   double user_z = user_position[2];//*Cos(th)*Cos(ph);
   
   glUseProgram(shader[DEFAULT]);
   
   if(mode) gluLookAt(user_x,user_y,user_z,user_x + (Sin(th) * Cos(ph)),user_y + Sin(ph), user_z + (Cos(th) * Cos(ph)), 0,Cos(ph),0);
   else gluLookAt(Sin(th)*Cos(ph)*TERRAIN_SCALE, Sin(ph)*TERRAIN_SCALE, Cos(th)*Cos(ph)*TERRAIN_SCALE, 0,0,0, 0,Cos(ph),0);
   
   //  Draw bumpmap
   //if (mode == 0)
   draw_terrain();
   
   //  Draw snow particles
   //if (mode == 1)
      //draw_particles();
   
   //  Draw water
   //if (mode == 2)
      //draw_water();
   
   //  Draw water
   //if (display_axis)
   draw_axis();

   //glDisable(GL_TEXTURE_2D);
   glUseProgram(DEFAULT);
   
   //  Display parameters
   glWindowPos2i(5,5);
   Print("FPS: %d || Height: %f || %d, %d || %d", FramesPerSecond(), height, relative_x, relative_y, testval);

   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Tell GLUT it is necessary to redisplay the scene
   //  Lock to 60 FPS
   float time = 0.001*glutGet(GLUT_ELAPSED_TIME);
   if (time < tick + 0.017)
      return;
   
   tick = time;
   
   glutPostRedisplay();
   relative_x = (user_position[0]) * TERRAIN_SCALE / 16.0 * 256.0;
   relative_y = (user_position[2]) * TERRAIN_SCALE / 16.0 * 256.0;
   
   float dX = fmod(relative_x, 1.0);
   float dY = fmod(relative_y, 1.0);
   
   /* 
    *  0          1
    *
    *
    *          dX
    *      You----|
    *       |
    *     dY|
    *       |
    *  2    -     3
    */
    
   //((0.h+2.h)/2 + 
    
   float height_0 = heightmap[(int)(relative_x - dX) + (int)(relative_y - dY) * 256];// * sqrt(pow(1.0-dX,2) + pow(1.0-dY,2));
   float height_1 = heightmap[(int)(relative_x + dX) + (int)(relative_y - dY) * 256];// * sqrt(pow(dX,2) + pow(1.0-dY,2));
   float height_2 = heightmap[(int)(relative_x - dX) + (int)(relative_y + dY) * 256];// * sqrt(pow(1.0-dX,2) + pow(dY,2));
   float height_3 = heightmap[(int)(relative_x + dX) + (int)(relative_y + dY) * 256];// * sqrt(pow(dX,2) + pow(dY,2));
   
   
   
   if (relative_x >= 0.0 && relative_x < 256
     &&relative_y >= 0.0 && relative_y < 256)
      height = (height_0 + height_1 + height_2 + height_3) / 4.0;
      //height = heightmap[(int)(relative_x + relative_y * 256)];
   else
      height = 1;
   
   float step = 0.001;
   if (space_pressed) step = 0.1;
   if (up_pressed) {
      user_position[0] += Sin(th)*step;
      user_position[2] += Cos(th)*step;
   }
   
   if (down_pressed) {
      user_position[0] -= Sin(th)*step;
      user_position[2] -= Cos(th)*step;
   }
   
   if (left_pressed) {
      th += 5;
   }
   
   if (right_pressed) {
      th -= 5;
   }
}
   
/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
  W = width;
  H = height;
  //  Ratio of the width to the height of the window
  asp = (height>0) ? (double)width/height : 1;
  //  Set the viewport to the entire window
  glViewport(0,0, width,height);
  
  Project(fov, asp, dim);
}
  
  /*
  *  GLUT calls this routine when an arrow key is pressed
  */
void special_key(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      right_pressed = 1;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      left_pressed = 1;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      up_pressed = 1;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      down_pressed = 1;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown ey - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Update projection
   Project(fov, asp, dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special_keyup(int key,int x,int y)
{
   if (key == GLUT_KEY_RIGHT)
      right_pressed = 0;
   else if (key == GLUT_KEY_LEFT)
      left_pressed = 0;
   else if (key == GLUT_KEY_UP)
      up_pressed = 0;
   else if (key == GLUT_KEY_DOWN)
      down_pressed = 0;
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void normal_key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   else if (ch == 'M' || ch == 'm')
      mode = !mode;
   else if (ch == 'F')
      fov++;
   else if (ch == 'f')
      fov--;
   else if (ch == 't')
      testval++;
   else if (ch == 'T')
      testval--;
   else if (ch == ' ')
      space_pressed = 1;
   Project(fov, asp, dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}
 
void normal_keyup(unsigned char ch) {
   if (ch == ' ')
      space_pressed = 0;
}
 
void mouse_click(int button, int state, int x, int y)
{
   if(!state) {
      mX = x;
      mY = y;
   }
}

void mouse_active_move(int x, int y)
{
   th += mX - x;
   ph += mY - y;
   mX = x;
   mY = y;
}

void mouse_passive_move(int x, int y)
{
   //th += mX - x;
   //ph += mY - y;
   //mX = x;
   //mY = y;
}

//
//  Create Shader Program with Location Names
//
int CreateShaderProgLoc(char* VertFile,char* FragFile,char* Name[])
{
   int k;
   //  Create program
   int prog = glCreateProgram();
   //  Create and compile vertex shader
   if (VertFile) CreateShader(prog,GL_VERTEX_SHADER,VertFile);
   //  Create and compile fragment shader
   if (FragFile) CreateShader(prog,GL_FRAGMENT_SHADER,FragFile);
   //  Set names
   for (k=0;Name[k];k++)
      if (Name[k][0])
         glBindAttribLocation(prog,k,Name[k]);
   ErrCheck("CreateShaderProg");
   //  Link program
   glLinkProgram(prog);
   //  Check for errors
   PrintProgramLog(prog);
   //  Return name
   return prog;
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_ALPHA);
   glutInitWindowSize(600,600);
   glutCreateWindow("Alexander Curtiss");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
   if (!GLEW_VERSION_2_0) Fatal("OpenGL 2.0 not supported\n");
#endif
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special_key);
   glutSpecialUpFunc(special_keyup);
   glutKeyboardFunc(normal_key);
   glutMouseFunc(mouse_click);
   glutMotionFunc(mouse_active_move);
   glutPassiveMotionFunc(mouse_passive_move);
   glutIdleFunc(idle);
   
   glEnable(GL_TEXTURE_2D);
   //  Particle shader mostly uses vert, while texture updating shaders use frag
   shader[0] = CreateShaderProgLoc("particle.vert","particle.frag",Name);
   shader[1] = CreateShaderProgLoc(NULL,"velocity.frag",Name);
   shader[2] = CreateShaderProgLoc(NULL,"position.frag",Name);
   shader[3] = CreateShaderProgLoc("heightmap.vert","heightmap.frag",Name);
   shader[4] = CreateShaderProgLoc("water.vert","water.frag",Name);
   //  Initialize texturess
   init_textures();
   //  Initialize particles
   init_particles();
   
   ErrCheck("init");
   glutMainLoop();
   return 0;
}