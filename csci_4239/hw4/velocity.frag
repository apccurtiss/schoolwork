// Large point texture

#version 120

uniform sampler2D position;
uniform sampler2D velocity;

void main()
{
   vec4 vel = texture2D(velocity,gl_TexCoord[0].xy);
   vec4 pos = texture2D(position,gl_TexCoord[0].xy);
   float velocityX = vel.x;
   float velocityY = vel.y;
   float velocityZ = vel.z;
   
   /*if (pos.y >= 0.8) {
      velocityY = 0;
   }
   
   velocityY += 0.01;
   
   if (velocityY > 1)
      velocityY = 1;
   
   */
   gl_FragColor = vel;//vec4(velocityX, velocityY, velocityZ,1);
}
