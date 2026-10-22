#ifndef SHADERS
#define SHADERS

// Shader nhúng thẳng vào chương trình (bản đóng gói không cần file .vert/.frag đi kèm).

// ---------- Hộp có ánh sáng: dùng cho ô sàn, khối người chơi, hạt, khung đích ----------
static const char *BOX_VERT = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;      // đỉnh của hộp đơn vị [0,1]^3
layout (location = 1) in vec3 aNormal;   // pháp tuyến của mặt (không gian vật thể)
// Dữ liệu theo từng hộp (instancing: cả lượt vẽ chỉ một lệnh glDrawArraysInstanced)
layout (location = 2) in mat4 iModel;    // chiếm location 2..5
layout (location = 6) in vec4 iColor;    // rgb + độ phát sáng
layout (location = 7) in vec4 iAccent;   // rgb màu hoa văn + alpha
layout (location = 8) in vec4 iScale;    // kích thước hộp + mã hoa văn
layout (location = 9) in vec4 iExtra;    // x: 1 = không tính ánh sáng

uniform mat4 uViewProj;

out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vObjNormal;
out vec3 vLocal;
flat out vec4 vColor;
flat out vec4 vAccent;
flat out vec4 vScale;
flat out float vUnlit;

void main()
{
    vec4 world = iModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    vNormal = normalize(transpose(inverse(mat3(iModel))) * aNormal);
    vObjNormal = aNormal;
    vLocal = aPos;
    vColor = iColor;
    vAccent = iAccent;
    vScale = iScale;
    vUnlit = iExtra.x;
    gl_Position = uViewProj * world;
}
)GLSL";

static const char *BOX_FRAG = R"GLSL(
#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vObjNormal;
in vec3 vLocal;
flat in vec4 vColor;
flat in vec4 vAccent;
flat in vec4 vScale;
flat in float vUnlit;

uniform float uTime;
uniform vec3 uEye;
uniform vec3 uLightDir;   // hướng tia sáng (từ đèn tới vật)
uniform vec3 uFogColor;
uniform vec2 uFogRange;

out vec4 outColor;

void main()
{
    vec3 uColor = vColor.rgb;
    vec3 uAccent = vAccent.rgb;
    vec3 uScale = vScale.xyz;   // kích thước hộp (đơn vị thế giới) để viền dày đều
    int uPattern = int(vScale.w + 0.5);   // 0 trơn, 1 ô cam, 2 công tắc thường, 3 công tắc nặng, 4 bóng mềm
    float uEmissive = vColor.a;
    float uAlpha = vAccent.a;

    // Tọa độ trên mặt hiện tại và khoảng cách (đơn vị thế giới) tới mép mặt.
    vec3 an = abs(vObjNormal);
    vec2 uv, sz;
    if (an.x > 0.5)      { uv = vLocal.yz; sz = uScale.yz; }
    else if (an.y > 0.5) { uv = vLocal.xz; sz = uScale.xz; }
    else                 { uv = vLocal.xy; sz = uScale.xy; }
    vec2 e = min(uv, 1.0 - uv) * sz;
    float edge = min(e.x, e.y);

    vec3 N = normalize(vNormal);
    bool top = N.z > 0.5;
    vec3 base = uColor;
    float glow = uEmissive;

    if (uPattern == 4) {   // bóng mềm dưới khối: đậm ở giữa, nhạt dần ra mép
        float s = smoothstep(0.0, 0.35, edge);
        outColor = vec4(0.0, 0.0, 0.0, uAlpha * s);
        return;
    }

    if (top) {
        vec2 c = uv - 0.5;
        if (uPattern == 1) {
            float stripe = step(0.5, fract((uv.x + uv.y) * 3.0));
            base *= mix(0.82, 1.0, stripe);
        } else if (uPattern == 2) {
            float r = length(c);
            float ring = 1.0 - smoothstep(0.035, 0.06, abs(r - 0.24));
            float dotc = 1.0 - smoothstep(0.07, 0.10, r);
            float m = max(ring, dotc);
            base = mix(base, uAccent, m);
            glow += m * (0.55 + 0.25 * sin(uTime * 3.0));
        } else if (uPattern == 3) {
            float d = abs(abs(c.x) - abs(c.y));
            float inside = 1.0 - step(0.30, max(abs(c.x), abs(c.y)));
            float m = (1.0 - smoothstep(0.045, 0.075, d)) * inside;
            base = mix(base, uAccent, m);
            glow += m * (0.45 + 0.2 * sin(uTime * 2.0));
        }
    }

    vec3 color;
    if (vUnlit > 0.5) {
        color = base;
    } else {
        vec3 L = normalize(-uLightDir);
        vec3 V = normalize(uEye - vWorldPos);
        vec3 H = normalize(L + V);
        float diff = max(dot(N, L), 0.0);
        float spec = pow(max(dot(N, H), 0.0), 48.0) * 0.35;
        float rim = pow(1.0 - max(dot(N, V), 0.0), 3.0) * 0.18;
        float sky = 0.5 + 0.5 * N.z;                      // ánh sáng môi trường: trên sáng, dưới tối
        color = base * (0.22 + 0.18 * sky + 0.70 * diff) + vec3(spec + rim);

        // Viền tối + gờ sáng ở mép mặt trên (giống Bloxorz gốc).
        float border = smoothstep(0.015, 0.045, edge);
        color *= mix(0.30, 1.0, border);
        if (top) {
            float bevel = (1.0 - smoothstep(0.045, 0.09, edge)) * border;
            color += bevel * 0.12;
        }
    }
    color += uColor * glow + uAccent * glow * 0.15;

    // Sương mù theo khoảng cách, hòa vào nền tối.
    float dist = length(uEye - vWorldPos);
    float fog = smoothstep(uFogRange.x, uFogRange.y, dist);
    color = mix(color, uFogColor, fog * 0.85);

    outColor = vec4(color, uAlpha);
}
)GLSL";

// ---------- Nền: gradient tối + sao lấp lánh, vẽ bằng một tam giác phủ màn hình ----------
static const char *BG_VERT = R"GLSL(
#version 330 core
out vec2 vUV;
void main()
{
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vUV = p;
    gl_Position = vec4(p * 2.0 - 1.0, 1.0, 1.0);
}
)GLSL";

static const char *BG_FRAG = R"GLSL(
#version 330 core
in vec2 vUV;
uniform float uTime;
uniform vec2 uRes;
out vec4 outColor;

float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

void main()
{
    vec2 uv = vUV;
    vec2 c = uv - vec2(0.5, 0.55);
    float vig = 1.0 - smoothstep(0.15, 0.95, length(c * vec2(uRes.x / uRes.y, 1.0)));
    vec3 col = mix(vec3(0.0, 0.0, 0.01), vec3(0.03, 0.04, 0.08), vig);

    // Sao: lưới ô 3px, mỗi ô có xác suất nhỏ chứa một ngôi sao nhấp nháy.
    vec2 cell = floor(uv * uRes / 3.0);
    float h = hash(cell);
    if (h > 0.9965) {
        float tw = 0.5 + 0.5 * sin(uTime * (1.0 + 3.0 * fract(h * 91.0)) + h * 50.0);
        col += vec3(0.55, 0.6, 0.75) * tw * fract(h * 7.0);
    }
    outColor = vec4(col, 1.0);
}
)GLSL";

#endif
