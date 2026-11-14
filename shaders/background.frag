#version 330 core
in vec2 vUV;
out vec4 fragColor;
uniform vec3 uForward,uRight,uUp,uLightDirection,uFogColor,uLightColor;
uniform float uAspect,uTanHalfFov,uDaylight,uTime;
uniform bool uEnhanced;
void main() {
    vec2 p=vUV*2-1;
    vec3 ray=normalize(uForward+uRight*p.x*uAspect*uTanHalfFov+uUp*p.y*uTanHalfFov);
    vec3 upper=mix(vec3(.008,.035,.08),vec3(.09,.43,.48),uDaylight);
    vec3 color=mix(uFogColor,upper,pow(max(ray.y,0),.8));
    color+=uLightColor*pow(max(dot(ray,normalize(uLightDirection)),0),32.0)*.35;
    if(uEnhanced) {
        vec3 upperWater=mix(vec3(.008,.027,.068),vec3(.075,.37,.44),uDaylight);
        color=mix(uFogColor,upperWater,smoothstep(-.08,.65,ray.y));
        float shimmer=.5+.5*sin(ray.x*31.0+sin(ray.z*28.0+uTime*.18)*2.0+uTime*.24);
        color+=vec3(.08,.13,.12)*shimmer*smoothstep(.35,.85,ray.y)*uDaylight*.22;
        color+=uLightColor*pow(max(dot(ray,normalize(uLightDirection)),0),48.0)*.24;
    }
    fragColor=vec4(pow(max(color,vec3(0)),vec3(.91)),1);
}
