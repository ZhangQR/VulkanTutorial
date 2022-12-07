#version 450
layout( lines ) in;
layout( line_strip, max_vertices = 12 ) out;
void main() {
for( int vertex = 0; vertex < 3; ++vertex ) {
    gl_Position = gl_in[vertex].gl_Position + vec4( 0.0, -0.1, 0.0, 0.0 );
    EmitVertex();
    gl_Position = gl_in[vertex].gl_Position + vec4( -0.1, 0.1, 0.0, 0.0 );
    EmitVertex();
    gl_Position = gl_in[vertex].gl_Position + vec4( 0.1, 0.1, 0.0, 0.0 );
    EmitVertex();
    gl_Position = gl_in[vertex].gl_Position + vec4( 0.0, -0.1, 0.0, 0.0 );
    EmitVertex();
    EndPrimitive();
}
}