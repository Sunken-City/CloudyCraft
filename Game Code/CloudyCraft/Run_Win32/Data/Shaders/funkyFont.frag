#version 410 core

uniform sampler2D gDiffuseTexture;
uniform bool gPop;
uniform vec4 gColor1;
uniform vec4 gColor2;
uniform bool gRainbow;
uniform float gTime;

in vec2 passUV0;
in vec2 passNormalizedGlyphPosition;
in vec2 passNormalizedStringPosition;
in float passNormalizedFragPosition;

out vec4 outColor;

void main()
{
    vec4 diffuse = texture(gDiffuseTexture, passUV0);

    if (gTime < passNormalizedStringPosition.x)
    {
        discard;
    }

    float wavyInterpolationValue = (sin(passNormalizedGlyphPosition.y + passNormalizedFragPosition + gTime) + 1.0f) / 2.0f; //Diagonal two-color rainbow
    float normalInterpolationValue = (sin(passNormalizedFragPosition + gTime) + 1.0f) / 2.0f; //Normal two-color rainbow
    float wiggleInterpolationValue = (sin(passNormalizedFragPosition + sin(passNormalizedGlyphPosition.y + (gTime * 10.0f)) + gTime) + 1.0f) / 2.0f; //Super wiggle
    //outColor = mix(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), interpolationValue) * diffuse; //mix(gColor1, gColor2, passNormalizedStringPosition.x);
    outColor = mix(gColor1, gColor2, wiggleInterpolationValue) * diffuse;
}