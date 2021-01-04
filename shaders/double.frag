#version 440 core
#extension ARB_gpu_shader_fp64 : enable

precision highp float;

in vec3 vColor;
in vec3 vNormal;
in vec2 vTexCoords;

uniform sampler2D texture_diffuse1;
uniform vec2 constant;
uniform int iters;
uniform double threshold;
uniform int jul;
uniform vec4 color;
uniform double xmin;
uniform double xmax;
uniform double ymin;
uniform double ymax;
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

int mandel(double p1, double p2)
{
    double start1 = 0;
    double start2 = 0;
    
    for (int i = 0; i < iters; ++i) {
        double s1 = start1*start1 - start2*start2 + p1;
        double s2 = 2*start1*start2 + p2;
        
        start1 = s1;
        start2 = s2;
        
        if (start1*start1 + start2*start2 > threshold)
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

int julia(double p1, double p2)
{
    
    for (int i = 0; i < iters; ++i) {
        double pp1 = p1*p1 - p2*p2 + constant.x;
        double pp2 = 2*p1*p2 + constant.y;
        
        p1 = pp1;
        p2 = pp2;
        
        if (p1*p1 + p2*p2 > threshold)
            return i;
    }
    
    return fill * iters;
}
  
void main()
{
	vec2 point = vec2(xmin+(xmax-xmin)*vTexCoords.x, ymin+(ymax-ymin)*vTexCoords.y);
	
	double p1 = xmin + (xmax-xmin)*double(vTexCoords.x);
	double p2 = ymin + (ymax-ymin)*double(vTexCoords.y);
	
	float val = 0;
	
	if (jul == 0)
        val = mandel(p1,p2)/float(iters);
	else
        val = julia(p1,p2)/float(iters);

	FragColor = val * color;
}
