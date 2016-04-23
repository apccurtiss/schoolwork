// Large point texture

#version 120

uniform sampler2D position_texture;
uniform sampler2D velocity_texture;
uniform sampler3D wind_texture;

void main()
{
   float draw = 1;
   vec4 velocity = texture2D(velocity_texture,gl_TexCoord[0].xy);
   vec4 position = texture2D(position_texture,gl_TexCoord[0].xy);
   vec4 wind = texture3D(wind_texture,position.xyz);
   float position_x = position.x - wind.x/30;// - (velocity.x * 0.01) - (wind.y * 0.01);//velocity.x * 0.01;
   float position_y = position.y - wind.y/30; //*-(velocity.y * 0.01)*/ - (wind.x * 0.01);//- velocity.y * 0.01;
   float position_z = position.z - wind.z/30;// - velocity.z * 0.01;
   
   if (position_x <= 0) {
      position_x = 0.8;
      draw = 0.0;
   }
   if (position.y <= 0) {
      position_y = 0.8;
      draw = 0.0;
   }
   if (position_z <= 0) {
      position_z = 0.8;
      draw = 0.0;
   }
   
   gl_FragColor = vec4(position_x, position_y, position_z, draw);
}