#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=3) in mat4 aInstanceModel;
layout(location=7) in vec4 aInstanceTint;
uniform mat4 uModel, uView, uProjection;
uniform vec4 uTint;
uniform bool uInstanced;
uniform float uTime, uSway;
out vec3 vWorld, vNormal;
out vec2 vUV;
out float vLocalHeight;
out vec4 vTint;
void main() {
    mat4 model = uInstanced ? aInstanceModel : uModel;
    vec3 p=aPosition;
    // An analytic wave bends the leaf mesh. This is animation, not fluid simulation.
    float phase=uTime*1.25+p.y*1.8+model[3].x*.4;
    p.x += uSway*sin(phase)*p.y*p.y*.035;
    vec3 localNormal=aNormal;
    float dxdy=uSway*.035*(2*aPosition.y*sin(phase)+1.8*aPosition.y*aPosition.y*cos(phase));
    // Inverse-transpose of this simple shear deformation.
    localNormal.y -= dxdy*localNormal.x;
    vec4 world=model*vec4(p,1.0);
    vWorld=world.xyz;
    vNormal=normalize(transpose(inverse(mat3(model)))*localNormal);
    vUV=aUV;
    vLocalHeight=aPosition.y;
    vTint=uInstanced ? aInstanceTint : uTint;
    gl_Position=uProjection*uView*world;
}
