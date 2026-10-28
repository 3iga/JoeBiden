#version 330 core
layout(location=0) in vec4 aPositionSize;
uniform mat4 uView,uProjection;
uniform float uHeight;
out float vDistance;
void main() {
    vec4 eye=uView*vec4(aPositionSize.xyz,1.0);
    gl_Position=uProjection*eye;
    gl_PointSize=clamp(aPositionSize.w*uHeight/max(-eye.z,.1),1.0,34.0);
    vDistance=length(eye.xyz);
}
