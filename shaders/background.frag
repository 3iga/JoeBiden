#version 330 core
in vec2 vUV;
out vec4 fragColor;
uniform vec3 uForward,uRight,uUp,uLightDirection;
uniform float uAspect,uTanHalfFov;
void main() {
    vec2 p=vUV*2.0-1.0;
    vec3 ray=normalize(uForward+uRight*p.x*uAspect*uTanHalfFov+uUp*p.y*uTanHalfFov);
    float height=clamp(ray.y*.7+.45,0.0,1.0);
    vec3 color=mix(vec3(.025,.20,.25),vec3(.07,.43,.47),height);
    float sun=pow(max(dot(ray,normalize(uLightDirection)),0.0),24.0);
    color+=vec3(.35,.49,.40)*sun;
    fragColor=vec4(pow(color,vec3(.91)),1.0);
}
