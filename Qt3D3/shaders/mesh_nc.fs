varying mediump vec3 vertNormal, halfVector;
varying mediump vec4 diffuse, ambient;
varying vec4 preColor;

// // varying light
varying mediump vec4 varLight0_position;
varying mediump vec4 varLight0_specular;
// // varying material
varying mediump vec4 varMat_specular;
varying float varHigh_shininess;

void main(void)
{
    vec3 n,halfV,lightDir;
    float NdotL,NdotHV;
 
    lightDir = vec3(varLight0_position);
 
    vec4 color = ambient;

    n = normalize(vertNormal);
 
    NdotL = max(dot(n,normalize(lightDir)),0.0);

    if (NdotL > 0.0) {
        color += diffuse * NdotL;
        halfV = normalize(halfVector);
        NdotHV = max(dot(n,halfV),0.0);
        color += varMat_specular *
				varLight0_specular *
                pow(NdotHV, varHigh_shininess);
    }
	gl_FragColor = color * preColor;
}