#version 410 core

uniform vec4 gColor;
uniform sampler2D gDiffuseTexture;
uniform sampler2D gEmissiveTexture; //The portal depth texture from last pass.
uniform int gPassNumber; //Render pass number

in vec4 passColor;
in vec2 passUV0;
flat in vec4 passFloatData0;
noperspective in vec2 passScreenCoord;
noperspective in float logZResult;

out vec4 outColor;
out float outPortalDepth;

void main()
{
    vec4 diffuse = texture(gDiffuseTexture, passUV0); //The diffuse texture for the blocks
    float isPortalValue = passFloatData0.x; //A custom float parameter that denotes whether or not the vertex is a portal. 1.0f means portal, 0.0f means no portal.
    float portalDepth = texture(gEmissiveTexture, passScreenCoord).r; //Max depth (white) if not portal, otherwise holds the depth value of the portal
    
    gl_FragDepth = logZResult;

    //If the pixel is in front of a portal's depth value, discard it.
    if(gl_FragDepth < portalDepth)
    {
        discard;
    }
    else
    {
        //If this is a portal fragment, write out the portal's depth for our next pass.
        //Only things behind this depth get written out.
        if(isPortalValue == 1.0f) 
        {
            outPortalDepth = gl_FragDepth;
        }
        //Otherwise, write out the farthest distance so that nothing gets drawn at this pixel next pass.
        else
        {
            outPortalDepth = 1.0f;
        }
    }
    outColor = mix(diffuse * passColor, gColor * passColor, isPortalValue);
    if(gPassNumber == 1)
    {
        outColor = diffuse * passColor;
    }
}
