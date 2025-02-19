uniform mediump mat4 matrix;
uniform float point_size;
uniform mediump vec4 color;
attribute highp vec4 vertex;
varying mediump vec4 vertColor;

void main(void)
{
	gl_Position = matrix * vertex;
	vertColor = color;
	gl_PointSize = point_size;
}