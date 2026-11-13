#version 330 core
in vec3 vWorld;
layout(location=0) out uint objectId;
uniform int uPickId;
uniform vec3 uEye;
uniform float uVisibility;
void main() {
    if(length(vWorld-uEye)>=uVisibility)discard;
    objectId=uint(uPickId); // zero for terrain/coral: they still write depth and occlude fish.
}
