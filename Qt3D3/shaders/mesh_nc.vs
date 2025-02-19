// uniform transformation
uniform mediump mat4 matrix;
uniform mediump vec4 color;
// uniform light
uniform mediump vec4 light0_position;
uniform mediump vec4 light0_diffuse;
uniform mediump vec4 ligth0_ambient;
uniform mediump vec4 ligth0_specular;
// uniform material
uniform mediump vec4 mat_diffuse;
uniform mediump vec4 mat_ambient;
uniform mediump vec4 mat_specular;
uniform float high_shininess;

attribute highp vec4 vertex;
attribute mediump vec3 normal;

varying mediump vec3 vertNormal, halfVector;
varying mediump vec4 diffuse, ambient;
varying vec4 preColor;

//// varying light
varying mediump vec4 varLight0_position;
varying mediump vec4 varLight0_specular;
//// varying material
varying mediump vec4 varMat_specular;
varying float varHigh_shininess;

void main(void)
{
	gl_Position = matrix * vertex;
	vertNormal = normalize(gl_NormalMatrix * normal);
	halfVector = normalize(normalize(light0_position.xyz) + vec3(0, 0, 1));
	diffuse = mat_diffuse * light0_diffuse;
	ambient = mat_ambient * ligth0_ambient;
	ambient += vec4(0.3, 0.3, 0.3, 1.0) * mat_ambient;
	preColor = color;

	varLight0_position = light0_position;
	varLight0_specular = ligth0_specular;
	varMat_specular = mat_specular;
	varHigh_shininess = high_shininess;
}
