#version 330 core
in float vDistance;
out vec4 fragColor;
void main() {
    vec2 q=gl_PointCoord*2.0-1.0;
    float r=length(q);if(r>1.0) discard;
    float rim=pow(r,5.0)*(1.0-smoothstep(.86,1.0,r));
    float shine=exp(-length(q-vec2(-.3,-.3))*10.0)*.7;
    float alpha=(rim*.32+shine)*exp(-vDistance*.035);
    fragColor=vec4(.55,.85,.92,alpha);
}
