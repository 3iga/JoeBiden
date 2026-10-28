#version 330 core
in vec3 vWorld,vNormal;
in vec2 vUV;
in vec4 vTint;
out vec4 fragColor;
uniform sampler2D uDiffuse,uNormalMap,uCaustics;
uniform vec3 uEye,uLightDirection;
uniform bool uLighting,uTexturing,uNormalMapping,uShowCaustics,uFog;
uniform float uTime,uUVScale,uShininess,uSpecular,uEmission;

vec3 mappedNormal(vec3 N,vec2 uv) {
    // Reconstruct tangent/bitangent from screen derivatives; no extra mesh attribute.
    vec3 dp1=dFdx(vWorld), dp2=dFdy(vWorld);
    vec2 duv1=dFdx(uv), duv2=dFdy(uv);
    float determinant=duv1.x*duv2.y-duv1.y*duv2.x;
    if(abs(determinant)<1e-9) return N;
    vec3 T=(dp1*duv2.y-dp2*duv1.y)/determinant;
    vec3 B=(-dp1*duv2.x+dp2*duv1.x)/determinant;
    T=normalize(T-N*dot(N,T));
    B=normalize(B-N*dot(N,B)-T*dot(T,B));
    vec3 sampleN=texture(uNormalMap,uv).xyz*2.0-1.0;
    return normalize(mat3(T,B,N)*sampleN);
}
void main() {
    vec2 uv=vUV*uUVScale;
    vec3 albedo=vTint.rgb*(uTexturing ? texture(uDiffuse,uv).rgb : vec3(1.0));
    vec3 N=normalize(vNormal);
    if(!gl_FrontFacing) N=-N; // Thin leaves/fins are intentionally two-sided.
    if(uNormalMapping && uTexturing) N=mappedNormal(N,uv);
    vec3 L=normalize(uLightDirection);
    vec3 V=normalize(uEye-vWorld),H=normalize(L+V);
    float lambert=max(dot(N,L),0.0);
    float spec=lambert>0.0 ? pow(max(dot(N,H),0.0),uShininess)*uSpecular : 0.0;
    vec3 ambient=vec3(.32,.43,.46);
    vec3 lit=albedo*(ambient+vec3(1.0,.94,.76)*lambert*.83)+vec3(.72,.92,1)*spec;
    if(!uLighting) lit=albedo;
    if(uShowCaustics && uLighting) {
        // Project an animated, repeating texture in WORLD XZ coordinates.
        // Two drifting layers imitate underwater caustics; no ray tracing or screen-space solver.
        vec2 p=vWorld.xz*.12+vWorld.y*.009;
        float a=texture(uCaustics,p+vec2(uTime*.013,uTime*.009)).r;
        float b=texture(uCaustics,p*1.17+vec2(-uTime*.011,uTime*.006)).r;
        float light=min(a,b)*2.2+max(a,b)*.20;
        float facing=pow(max(N.y,0.0),.6);
        lit += albedo*vec3(.65,1.0,.90)*light*facing*.70;
    }
    lit+=albedo*uEmission;
    if(uFog) {
        float distanceToEye=length(uEye-vWorld);
        float amount=1.0-exp(-distanceToEye*.037);
        vec3 fogColor=mix(vec3(.025,.23,.28),vec3(.045,.32,.35),clamp(vWorld.y/12.0,0.0,1.0));
        lit=mix(lit,fogColor,clamp(amount,0.0,.98));
    }
    // A mild display curve; this prototype is not an HDR post-processing pipeline.
    lit=pow(max(lit,vec3(0.0)),vec3(.91));
    fragColor=vec4(lit,1.0);
}
