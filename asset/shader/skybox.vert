#version 450

layout(location = 0) in vec3 iVertPos;

layout(std140, binding = 0) uniform CameraBlock {
    mat4 uViewMatrix;
    mat4 uProjMatrix;
    mat4 uInvViewProjMatrix;
    vec3 uCameraPos;
};

layout(location = 0) out vec3 oFragDir; // for cube map sampling

void main() {
    oFragDir = iVertPos;
    mat4 viewMatrixWithoutTranslation = mat4(mat3(uViewMatrix)); // remove translation from view matrix
    vec4 fragPos = uProjMatrix * viewMatrixWithoutTranslation * vec4(iVertPos, 1.0);
    gl_Position = fragPos.xyww; // set w component to depth for proper perspective correction
}
