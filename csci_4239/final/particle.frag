//
//  Particle engine - derived from Confetti Cannon derived from Orange Book
//  

uniform sampler3D wind_texture;
varying vec4 snow_position;

void main(void)
{
   //vec4 wind = texture3D(wind_texture,snow_position.xyz);
   gl_FragColor = vec4(0.8,0.9,1.0,1.0);
}
