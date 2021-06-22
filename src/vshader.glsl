#version 150

in vec3 vPosition;
out vec4 color;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec3 uColor;

void main()
{
    gl_Position = Projection * ModelView * vec4(vPosition,1.0);
    color = vec4(uColor, 1.0);
}
