uniform sampler2D yTexture;
uniform sampler2D uTexture;
uniform sampler2D vTexture;

varying vec2 vTexCoord;

const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);
const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);
const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);
const vec3 offset = vec3(-0.0625, -0.5, -0.5);

void main (void)
{
    float y = texture2D(yTexture, vTexCoord).r;
    float u = texture2D(uTexture, vTexCoord).r;
    float v = texture2D(vTexture, vTexCoord).r;

    vec3 yuv = vec3(y,u,v);
    yuv += offset;

    float r = dot(yuv, R_cf);
    float g = dot(yuv, G_cf);
    float b = dot(yuv, B_cf);
    gl_FragColor = vec4(r,g,b,0.0);
}
