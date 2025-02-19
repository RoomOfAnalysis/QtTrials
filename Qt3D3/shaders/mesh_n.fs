varying mediump vec3 vertNormal;
void main(void)
{
	gl_FragColor = vec4(vertNormal.xyz, 1);
}
