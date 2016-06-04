#version 410 core

uniform float gWave;
uniform bool gShake;
uniform bool gDamp;
uniform float gTime;
uniform vec3 gUpVector;
uniform vec3 gRightVector;

uniform mat4 gModel;
uniform mat4 gProj;
uniform mat4 gView;

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0;
in vec2 inNormalizedGlyphPosition;
in vec2 inNormalizedStringPosition;
in float inNormalizedFragPosition;

out vec2 passUV0;
out vec2 passNormalizedGlyphPosition;
out vec2 passNormalizedStringPosition;
out float passNormalizedFragPosition;


void main()
{
    passUV0 = inUV0;
    passNormalizedGlyphPosition = inNormalizedGlyphPosition;
    passNormalizedStringPosition = inNormalizedStringPosition;
    passNormalizedFragPosition = inNormalizedFragPosition;
    vec4 pos = vec4(inPosition, 1.f);
    pos = pos * gModel * gView * gProj;
    vec3 changePos = pos.xyz;
    changePos += sin(gTime * 5.f + inNormalizedFragPosition) * gWave * .01f * gUpVector;
    if (gShake)
    {
        changePos += sin(gTime * 40.f) * .001f * gRightVector;
    }
    gl_Position = vec4(changePos, 1.f);
}