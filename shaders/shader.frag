#version 330 core
precision highp float;

in vec3 vColor;
in vec3 vNormal;
in vec2 vTexCoords;

uniform sampler2D texture_diffuse1;
uniform vec2 constant;
uniform int iters;
uniform float threshold;
uniform int jul;
uniform vec4 color;
uniform highp float xmin;
uniform highp float xmax;
uniform highp float ymin;
uniform highp float ymax;
uniform int fill;

out vec4 FragColor; 
  
vec2 sqrd(vec2 num)
{
    return vec2(num.x*num.x - num.y*num.y, 2*num.x*num.y);
}

float length(vec2 num)
{
    return num.x*num.x + num.y*num.y;
}

int mandel(vec2 num)
{
    vec2 start = vec2(0,0);
    
    for (int i = 0; i < iters; ++i) {
        start = sqrd(start) + num;
        if (length(start) > threshold)
            return i;
    }
    
    return fill * iters;
}

int julia(vec2 num)
{
    vec2 start = num;
    
    for (int i = 0; i < iters; ++i) {
        start = sqrd(start) + constant;
        if (length(start) > threshold)
            return i;
    }
    
    return fill * iters;
}
  
void main()
{
	vec2 point = vec2(xmin+(xmax-xmin)*vTexCoords.x, ymin+(ymax-ymin)*vTexCoords.y);	
	float val = 0;
	
	if (jul == 0)
        val = mandel(point)/float(iters);
	else
        val = julia(point)/float(iters);

	FragColor = val * color;
}
