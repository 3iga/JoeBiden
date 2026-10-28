#version 330 core
layout(location=0) in vec2 aPosition;
layout(location=1) in vec4 aColor;
uniform vec2 uWindow;
out vec4 vColor;
void main() {
    vec2 p=aPosition/uWindow;
    gl_Position=vec4(p.x*2.0-1.0,1.0-p.y*2.0,0.0,1.0);vColor=aColor;
}
