in vec2 VertexPosition;

uniform mat4 Transform;

void main()
{
    gl_Position = Transform * vec4(VertexPosition.x, VertexPosition.y, 0.0, 1.0);
}
