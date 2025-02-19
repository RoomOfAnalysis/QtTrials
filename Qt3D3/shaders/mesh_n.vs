uniform mat4 matrix;

attribute highp vec4 vertex;
attribute mediump vec3 normal;
varying mediump vec3 vertNormal;
 
void main()
{
	gl_Position = matrix * vertex;
	// absolute normal value
	vec3 temp = normalize(gl_NormalMatrix * normal);
	vertNormal = vec3(abs(temp.x), abs(temp.y), abs(temp.z));
}
