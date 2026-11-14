#version 330 core
in vec3 vWorld,vNormal;
in vec2 vUV;
in float vLocalHeight;
in vec4 vTint;
out vec4 fragColor;
uniform sampler2D uDiffuse,uNormalMap,uCaustics,uRockDiffuse;
uniform vec2 uWorldOffset;
uniform vec3 uEye,uLightDirection,uAmbient,uLightColor,uFogColor;
uniform bool uLighting,uTexturing,uNormalMapping,uShowCaustics,uFog;
uniform int uFishPattern,uFishVariant,uMaterialKind;
uniform bool uEnhanced;
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
    if(uFishPattern==-2) albedo=mix(albedo,vec3(.73,.77,.75),smoothstep(.48,.72,vUV.y));
    vec3 geometricN=normalize(vNormal);
    if(!gl_FrontFacing)geometricN=-geometricN;
    vec3 N=geometricN;
    if(uNormalMapping&&uTexturing) {
        N=mappedNormal(N,uv);
        // Surface detail fades with distance instead of sparkling in the background.
        if(uEnhanced)N=normalize(mix(geometricN,N,(uMaterialKind==0?.25:.75)*(1.0-smoothstep(20.0,48.0,distanceToEye)*.78)));
    }
    float shininess=uShininess,specular=uSpecular;
    if(uEnhanced) {
        if(uMaterialKind==0) {
            // Texture mapping (week 8): sand on horizontal surfaces, rock on steep banks.
            float slope=1.0-smoothstep(.65,.96,max(geometricN.y,0.0));
            vec3 rockColor=vec3(.43,.48,.40);
            if(uTexturing)rockColor*=texture(uRockDiffuse,uv*.65).rgb*1.8;
            albedo=mix(mix(albedo,vec3(.73,.73,.61),.60),rockColor,slope*.92);
            shininess=10.0;specular=.055;
        } else if(uMaterialKind==1) {
            float tip=smoothstep(.12,2.2,vLocalHeight);
            albedo*=mix(vec3(.62,.68,.76),vec3(1.06,1.05,1.0),tip);
            shininess=22.0;specular=.11;
        } else if(uMaterialKind==2) {
            float top=smoothstep(.10,.85,geometricN.y);
            albedo*=mix(vec3(.78,.86,.91),vec3(1.06,1.0,.77),top);
            shininess=12.0;specular=.065;
        } else if(uMaterialKind==3) {
            shininess=82.0;specular=.72;
        } else if(uMaterialKind==4) {
            albedo*=mix(vec3(.48,.70,.66),vec3(.98,1.08,.74),smoothstep(0.0,3.5,vLocalHeight));
            shininess=20.0;specular=.10;
        }
    }
    vec3 L=normalize(uLightDirection),V=normalize(uEye-vWorld);
    vec3 hv=L+V;vec3 H=dot(hv,hv)>1e-10?normalize(hv):N;
    float lambert=max(dot(N,L),0.0);
    float spec=lambert>0?pow(max(dot(N,H),0.0),shininess)*specular:0;
    vec3 ambient=uAmbient;
    if(uEnhanced) {
        // Simple directional ambient fill, not SSAO or a shadow map.
        ambient*=mix(.52,1.12,clamp(geometricN.y*.5+.5,0.0,1.0));
    }
    vec3 lit=albedo*(ambient+uLightColor*lambert*(uEnhanced?1.02:.83))+uLightColor*spec*.65;
    if(!uLighting)lit=albedo;
    if(uShowCaustics&&uLighting) {
        vec2 p=(vWorld.xz+uWorldOffset)*.12+vWorld.y*.009;
        float a=texture(uCaustics,p+vec2(uTime*.013,uTime*.009)).r;
        float b=texture(uCaustics,p*1.17+vec2(-uTime*.011,uTime*.006)).r;
        float light=min(a,b)*2.2+max(a,b)*.20;
        float fade=uEnhanced?1.0-smoothstep(18.0,52.0,distanceToEye)*.7:1.0;
        lit+=albedo*vec3(.65,1,.90)*light*pow(max(N.y,0),.6)*(uEnhanced?.58:.65)*uDaylight*fade;
    }
    if(uEnhanced&&uLighting) {
        // Stylized depth tint. This is an inexpensive color treatment, not water physics.
        float haze=1.0-exp(-distanceToEye*.008);
        lit*=mix(vec3(1),vec3(.56,.90,1.0),haze);
        if(uMaterialKind==1) {
            float tips=smoothstep(1.3,2.6,vLocalHeight);
            lit+=albedo*tips*.14*(1.0-uDaylight); // modest unlit tips, no bloom pass
        }
    }
    lit+=albedo*uEmission;
    if(uFog)lit=mix(lit,uFogColor,smoothstep(uFogStart,uVisibility,distanceToEye));
    if(uEnhanced)lit=lit/(vec3(1.0)+max(lit-vec3(.72),vec3(0.0))*.32);
    fragColor=vec4(pow(max(lit,vec3(0)),vec3(.91)),1);
}
