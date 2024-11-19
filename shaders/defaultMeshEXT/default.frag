#version 460 core

layout (location = 0) in PerVertexData
{
    vec4 color;
} fragIn;

layout(location = 0) out vec4 FragColor;

vec3 rec709_eotf(vec3 v_){
    mat2x3 v = mat2x3((v_ / 4.5), pow((v_ + 0.0999) / 1.099, vec3(2.22222)));
    return vec3 (
        v[v_[0] < 0.081 ? 0 : 1][0],
        v[v_[1] < 0.081 ? 0 : 1][1],
        v[v_[2] < 0.081 ? 0 : 1][2]
    );
}

void main()
{
    FragColor = vec4(rec709_eotf(fragIn.color.rgb), 1.0);
}