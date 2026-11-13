#version 330 core
in float vDistance;
out vec4 fragColor;
uniform float uFogStart,uVisibility,uDaylight;
uniform bool uFog;
void main() {
    if(vDistance>=uVisibility)discard;
    vec2 q=gl_PointCoord*2-1;float r=length(q);if(r>1)discard;
    float rim=pow(r,5)*(1-smoothstep(.86,1,r));
    float shine=exp(-length(q-vec2(-.3,-.3))*10)*.7;
    float fade=uFog?1-smoothstep(uFogStart,uVisibility,vDistance):1;
    fragColor=vec4(vec3(.55,.85,.92)*mix(.2,1,uDaylight),(rim*.32+shine)*fade*.65);
}
