#version 330 core
in vec3 vWorld,vNormal;
in vec2 vUV;
in vec4 vTint;
out vec4 fragColor;
uniform sampler2D uDiffuse,uNormalMap,uCaustics;
uniform vec2 uWorldOffset;
uniform vec3 uEye,uLightDirection,uAmbient,uLightColor,uFogColor;
uniform bool uLighting,uTexturing,uNormalMapping,uShowCaustics,uFog;
uniform int uFishPattern,uFishVariant;
uniform float uTime,uUVScale,uShininess,uSpecular,uEmission,uFogStart,uVisibility,uDaylight;
float stripe(float x,float center,float width) {return 1.0-smoothstep(width*.60,width,abs(x-center));}
vec3 fishPattern(vec3 color) {
    float x=cos(vUV.x*6.2831853),y=cos(vUV.y*3.14159265);
    if(uFishPattern==0) {
        float wide=max(max(stripe(x,-.55,.15),stripe(x,-.04,.15)),stripe(x,.46,.13));
        float core=max(max(stripe(x,-.55,.10),stripe(x,-.04,.10)),stripe(x,.46,.085));
        color=mix(color,vec3(.025),wide);color=mix(color,vec3(1,.96,.87),core);
        if(uFishVariant==1)color=mix(color,vec3(.07,.065,.06),(.65-core)*.45);
    } else if(uFishPattern==1) {
        float dark=(1.0-smoothstep(.12,.21,abs(y-.15-.15*sin(x*4))))*(1.0-smoothstep(.65,.87,x));
        color=mix(color,vec3(.015,.025,.07),dark*.96);
        color=mix(color,vec3(1,.85,.04),1.0-smoothstep(-.90,-.70,x));
    } else if(uFishPattern==2) {
        float bars=max(max(stripe(x,-.60,.09),stripe(x,-.12,.10)),stripe(x,.40,.11));
        color=mix(color,vec3(.97,.45,.08),bars);
        float spot=1.0-smoothstep(.13,.21,length(vec2(x+.38,y-.48)));
        color=mix(color,vec3(.04),spot);
    } else if(uFishPattern==3) {
        float lines=abs(sin(vUV.x*126.0+sin(vUV.y*50.0)*.4));
        color=mix(color,vec3(.72,.96,.74),(1-smoothstep(.025,.19,lines))*.40);
        color=mix(color,vec3(.93,.74,.63),smoothstep(.64,.96,x)*.35);
    } else if(uFishPattern==4) {
        color*=mix(.55,1.0,uDaylight);
        color=mix(color,vec3(.84,.90,.72),stripe(y,0,.04)*(1-uDaylight)*.55);
    } else if(uFishPattern==5) {
        float bars=max(stripe(x,-.36,.28),stripe(x,.47,.17));
        color=mix(color,vec3(.025,.03,.03),bars);
        color=mix(color,vec3(1,.79,.10),stripe(x,.04,.15));
    } else if(uFishPattern==6) {
        if(uFishVariant==1) {
            float rings=abs(sin(length(vec2(x*1.25,y)) * 18.0));
            color=mix(vec3(.025,.05,.25),vec3(.72,.96,1),1-smoothstep(.08,.22,rings));
        } else {
            float bands=smoothstep(-.1,.15,sin(vUV.y*80.0+x*1.8));
            color=mix(color,vec3(1,.87,.10),bands*.94);
            color=mix(color,vec3(.02,.045,.09),stripe(x,.68,.17));
        }
    } else if(uFishPattern==7) {
        color*=.80+.20*sin(vUV.y*3.14159265);
    }
    return color;
}
vec3 mappedNormal(vec3 N,vec2 uv) {
    vec3 dp1=dFdx(vWorld),dp2=dFdy(vWorld);vec2 duv1=dFdx(uv),duv2=dFdy(uv);
    float det=duv1.x*duv2.y-duv1.y*duv2.x;
    if(abs(det)<1e-9)return N;
    vec3 T=(dp1*duv2.y-dp2*duv1.y)/det,B=(-dp1*duv2.x+dp2*duv1.x)/det;
    T-=N*dot(N,T);if(dot(T,T)<1e-10)return N;T=normalize(T);
    B-=N*dot(N,B)+T*dot(T,B);if(dot(B,B)<1e-10)return N;B=normalize(B);
    return normalize(mat3(T,B,N)*(texture(uNormalMap,uv).xyz*2.0-1.0));
}
void main() {
    float distanceToEye=length(uEye-vWorld);
    // CPU conservative culling + exact fragment cutoff; no hidden far-object submissions.
    if(distanceToEye>=uVisibility)discard;
    vec2 uv=vUV*uUVScale;
    vec3 albedo=vTint.rgb*(uTexturing?texture(uDiffuse,uv).rgb:vec3(1));
    if(uFishPattern>=0)albedo=fishPattern(albedo);
    vec3 N=normalize(vNormal);if(!gl_FrontFacing)N=-N;
    if(uNormalMapping&&uTexturing)N=mappedNormal(N,uv);
    vec3 L=normalize(uLightDirection),V=normalize(uEye-vWorld);
    vec3 hv=L+V;vec3 H=dot(hv,hv)>1e-10?normalize(hv):N;
    float lambert=max(dot(N,L),0.0);
    float spec=lambert>0?pow(max(dot(N,H),0.0),uShininess)*uSpecular:0;
    vec3 lit=albedo*(uAmbient+uLightColor*lambert*.83)+uLightColor*spec*.65;
    if(!uLighting)lit=albedo;
    if(uShowCaustics&&uLighting) {
        vec2 p=(vWorld.xz+uWorldOffset)*.12+vWorld.y*.009;
        float a=texture(uCaustics,p+vec2(uTime*.013,uTime*.009)).r;
        float b=texture(uCaustics,p*1.17+vec2(-uTime*.011,uTime*.006)).r;
        float light=min(a,b)*2.2+max(a,b)*.20;
        lit+=albedo*vec3(.65,1,.90)*light*pow(max(N.y,0),.6)*.65*uDaylight;
    }
    lit+=albedo*uEmission;
    if(uFog)lit=mix(lit,uFogColor,smoothstep(uFogStart,uVisibility,distanceToEye));
    fragColor=vec4(pow(max(lit,vec3(0)),vec3(.91)),1);
}
