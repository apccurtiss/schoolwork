//
//  Water color
//  

uniform float tick;
uniform sampler2D choppy_water;
uniform sampler2D smooth_water;

varying float textureX;
varying float textureY;
varying float waveheight;

vec4 water1 = vec4(1.0,1.0,1.0,1.0);
vec4 water2 = vec4(0.0,0.0,0.0,1.0);
vec4 water3 = vec4(0,0.4,0.7,1.0);//vec4(0.7,0.7,1.0,1.0);
vec4 water4 = vec4(0,0.7,1.0,1.0);//vec4(0.5,0.9,1.0,1.0);


void main(void)
{
   vec2 choppy_coordinates = vec2(textureX, mod((textureY + (tick * 0.4)), 1.0));
   vec4 choppy_texture = texture2D(choppy_water, choppy_coordinates);
   
   vec2 smooth_coordinates = vec2(mod((textureX + (tick * 0.2)), 1.0), textureY);
   vec4 smooth_texture = texture2D(smooth_water, smooth_coordinates);
   
   vec4 water_color = mix(water2, water4, smooth_texture.r);
   water_color += mix(water2, water3, choppy_texture.r);
   
   if (waveheight > 0.1 && choppy_texture.r > 0.6 && smooth_texture.r > 0.6) {
      water_color = water1;
   }
   gl_FragColor = water_color;
}
