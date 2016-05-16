#version 410 core

uniform mat4 gModel;
uniform mat4 gView;
uniform mat4 gProj;
uniform int gPassNumber; //Render pass number

in vec3 inPosition;
in vec4 inColor;
in vec2 inUV0;
in vec4 inFloatData0;

out vec4 passColor;
out vec2 passUV0;
flat out vec4 passFloatData0;
noperspective out vec2 passScreenCoord;
noperspective out float logZResult;

void main(void)
{
  passColor = inColor;
  passUV0 = inUV0;
  passFloatData0 = inFloatData0;

  vec4 pos = vec4(inPosition, 1.0f);
  pos = pos * gModel * gView * gProj; 

  //Convert from Clip space to NDC
  passScreenCoord = pos.xy / pos.w;
  passScreenCoord = (passScreenCoord + 1) / 2.0f; //Convert from 0 to 1 -> -1 to 1

  gl_Position = pos;
  
  //Make the depth buffer linear so that blocks can't render in front of the portal, regardless of the distance.
  //Based off of code from: http://outerra.blogspot.com/2012/11/maximizing-depth-buffer-range-and.html
  float near = 0.1f;
  float far = 1000.0f;
  float C = 0.01;
  float FC = 1.0 / log(far * C + 1);
  logZResult = log(gl_Position.w * C + 1) * FC;
  gl_Position.z = (2 * logZResult - 1) * gl_Position.w;
}
