//
//  Terrain color
//  

uniform sampler2D heightmap_texture;
varying vec4 position;

const vec4 grass = vec4(0.6,1.0,0.6,1);
const vec4 snow = vec4(1,1,1,1);

void main(void)
{
   gl_FragColor = mix(grass, snow, position.y * 3.0);
   //vec4(position.y * 2.0, 0.5-position.y, 0.0, 1.0);
}
