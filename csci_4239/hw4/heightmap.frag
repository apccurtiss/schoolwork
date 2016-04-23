//
//  Terrain color
//  

uniform sampler2D heightmap_texture;   
uniform sampler2D rock_texture;
varying vec2 texture_coordinates;
varying vec4 position;
varying vec3 norm;
const vec4 grass = vec4(0.6,1.0,0.6,1);
const vec4 snow = vec4(1,1,1,1);

void main(void)
{
   gl_FragColor = mix(grass, snow, position.y * 3.0);
   
   gl_FragColor = mix(gl_FragColor, texture2D(rock_texture, texture_coordinates), abs(1.0-norm.y));
   
   if (position.y < 0.05) { 
      gl_FragColor = vec4(0.2,0.6,0.5,0.5);
   }
   
   //vec4(position.y * 2.0, 0.5-position.y, 0.0, 1.0);
}
