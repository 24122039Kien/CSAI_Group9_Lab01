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
flat out float vMaterial;

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
    vMaterial = iExtra.y;
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
flat in float vMaterial;

uniform float uTime;
uniform vec3 uEye;
uniform vec3 uLightDir;   // hướng tia sáng (từ đèn tới vật)
uniform vec3 uFogColor;
uniform vec2 uFogRange;
uniform int uTheme;       // 0 Classic, 1 Minecraft

out vec4 outColor;

float hash2(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

// Chữ "TNT" 3x5 điểm ảnh trên dải trắng của khối TNT (x, y: texel trong ô 16x16, y tính từ trên).
bool tntLetter(int x, int y)
{
    if (y < 6 || y > 10) return false;
    int r = y - 6, g = -1, cx = 0;
    if (x >= 2 && x <= 4)        { g = 0; cx = x - 2; }
    else if (x >= 6 && x <= 8)   { g = 1; cx = x - 6; }
    else if (x >= 10 && x <= 12) { g = 0; cx = x - 10; }
    if (g < 0) return false;
    int bits = (g == 0) ? (r == 0 ? 7 : 2) : ((r == 1 || r == 2) ? 7 : 5);
    return ((bits >> (2 - cx)) & 1) == 1;
}

// Texture pixel kiểu Minecraft: 16 texel cho mỗi đơn vị chiều dài, mỗi texel một màu có nhiễu.
// face: 0 mặt trên, 1 mặt bên, 2 mặt dưới. rowTop: (mặt bên) số texel tính từ mép trên.
vec3 mcMaterial(int mat, vec2 uv, vec2 sz, int face, vec3 tint, vec3 accent, out float glow)
{
    glow = 0.0;
    vec2 tf = floor(uv * sz * 16.0);
    vec2 tl = mod(tf, 16.0);
    vec2 tile = floor(vWorldPos.xy + 0.5);
    float n = hash2(tf + tile * 13.0);
    float n2 = hash2(tf * 1.7 + tile * 5.3 + 3.1);
    float rowTop = (1.0 - uv.y) * sz.y * 16.0;
    vec3 c = tint;
    if (mat == 1) {                        // sàn: khối cỏ
        vec3 grass = vec3(0.37, 0.63, 0.25) * (0.80 + 0.32 * n);
        vec3 dirt = vec3(0.55, 0.39, 0.26) * (0.78 + 0.32 * n);
        float fringe = 2.0 + floor(n2 * 2.0);
        c = face == 0 ? grass : (face == 2 ? dirt : (rowTop < fringe ? grass * 0.92 : dirt));
    } else if (mat == 2) {                 // ô dễ vỡ: đất nung có vết nứt
        c = vec3(0.86, 0.49, 0.21) * (0.84 + 0.24 * n);
        if (face == 0) {
            bool crack = (mod(tl.x + tl.y, 9.0) < 1.0 && n2 > 0.30) ||
                         (mod(tl.x - tl.y + 32.0, 11.0) < 1.0 && n > 0.45);
            if (crack) c *= 0.45;
        }
    } else if (mat == 3) {                 // công tắc thường: đá cuội + tấm áp lực
        vec2 st = floor(tf / 4.0);
        c = vec3(0.50) * (0.70 + 0.35 * hash2(st + tile * 7.0)) * (0.92 + 0.12 * n);
        if (mod(tf.x, 4.0) < 1.0 || mod(tf.y, 4.0) < 1.0) c *= 0.75;
        if (face == 0 && tl.x >= 4.0 && tl.x <= 11.0 && tl.y >= 4.0 && tl.y <= 11.0) {
            bool rim = tl.x == 4.0 || tl.x == 11.0 || tl.y == 4.0 || tl.y == 11.0;
            float pulse = 0.5 + 0.5 * sin(uTime * 3.0);
            c = rim ? vec3(0.30) : mix(vec3(0.70), accent, 0.35) * (0.95 + 0.08 * n);
            if (!rim) glow = 0.25 + 0.25 * pulse;
        }
    } else if (mat == 4) {                 // công tắc nặng: obsidian + chữ X vàng
        c = vec3(0.10, 0.07, 0.16) * (0.75 + 0.5 * n);
        if (n2 > 0.93) c += vec3(0.25, 0.12, 0.35);
        if (face == 0 && tl.x >= 2.0 && tl.x <= 13.0 && tl.y >= 2.0 && tl.y <= 13.0 &&
            (abs(tl.x - tl.y) < 1.0 || abs(tl.x + tl.y - 15.0) < 1.0)) {
            c = vec3(0.98, 0.78, 0.22);
            glow = 0.35 + 0.2 * sin(uTime * 2.0);
        }
    } else if (mat == 5) {                 // split switch: khối cổng tối, lấp lánh
        c = vec3(0.04, 0.07, 0.11) * (0.8 + 0.4 * n);
        if (hash2(tf + floor(uTime * 5.0)) > 0.93) { c = vec3(0.35, 0.95, 1.0); glow = 0.8; }
        if (face == 0 && tl.y >= 3.0 && tl.y <= 12.0 &&
            (tl.x == 3.0 || tl.x == 12.0 || ((tl.y == 3.0 || tl.y == 12.0) && (tl.x == 4.0 || tl.x == 11.0)))) {
            c = vec3(0.30, 0.90, 1.0); glow = 0.6;
        }
    } else if (mat == 6) {                 // cầu: ván gỗ sồi
        float row = floor(tl.y / 4.0);
        c = vec3(0.68, 0.52, 0.31) * (0.88 + 0.16 * n);
        if (mod(tl.y, 4.0) == 3.0 || mod(tl.x + row * 8.0, 16.0) == 15.0) c *= 0.68;
    } else if (mat == 7) {                 // khối người chơi: TNT
        int x = int(tl.x), y = 15 - int(tl.y);
        if (face == 0 || face == 2) {
            c = vec3(0.80, 0.78, 0.72) * (0.85 + 0.2 * n);
            if (tl.x >= 6.0 && tl.x <= 9.0 && tl.y >= 6.0 && tl.y <= 9.0) c = vec3(0.25, 0.22, 0.20);
        } else {
            c = tint * (0.84 + 0.2 * n);
            if (y >= 5 && y <= 11) c = vec3(0.92, 0.91, 0.88) * (0.92 + 0.1 * n);
            if (tntLetter(x, y)) c = vec3(0.10);
        }
    } else if (mat == 8) {                 // khối nhỏ đang điều khiển: khối vàng
        c = vec3(0.98, 0.82, 0.25) * (0.90 + 0.14 * n);
        if (tl.x == 0.0 || tl.y == 15.0) c *= 1.15;
        if (tl.x == 15.0 || tl.y == 0.0) c *= 0.72;
        glow = 0.18;
    } else if (mat == 9) {                 // khung lỗ đích: đá cổng phát sáng
        c = mix(vec3(0.20, 0.30, 0.28), tint, 0.5) * (0.8 + 0.4 * n);
    }
    return c;
}

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
        outColor = vec4(0.0, 0.0, 0.0, uAlpha * s * (uTheme == 1 ? 0.6 : 1.0));
        return;
    }

    int mat = int(vMaterial + 0.5);
    if (uTheme == 1) {
        // Theme Minecraft: texture pixel + đổ bóng phẳng theo mặt (trên sáng nhất, dưới tối nhất).
        int face = N.z > 0.5 ? 0 : (N.z < -0.5 ? 2 : 1);
        int texFace = face;
        vec2 tuv = uv, tsz = sz;
        if (mat == 7 && max(uScale.x, max(uScale.y, uScale.z)) > 1.5 * min(uScale.x, min(uScale.y, uScale.z))) {
            // Khối TNT 1x1x2: trục dài là trục x của vật thể. Hai mặt đầu là "nắp"; mặt bên đếm
            // hàng theo trục dài để dải trắng + chữ TNT luôn nằm ngang mỗi ô 1x1 dù khối đứng hay nằm.
            texFace = an.x > 0.5 ? 0 : 1;
            if (texFace == 1) { tuv = uv.yx; tsz = sz.yx; }
        }
        float g2 = 0.0;
        vec3 tex = mat > 0 ? mcMaterial(mat, tuv, tsz, texFace, uColor, uAccent, g2) : uColor;
        float shade = face == 0 ? 1.0 : (face == 2 ? 0.5 : (abs(N.x) > 0.5 ? 0.80 : 0.64));
        vec3 col = vUnlit > 0.5 ? tex : tex * shade;
        col += tex * (g2 + (mat > 0 ? 0.0 : uEmissive)) + uColor * uEmissive * (mat > 0 ? 0.35 : 0.0);
        float d = length(uEye - vWorldPos);
        col = mix(col, uFogColor, smoothstep(uFogRange.x, uFogRange.y, d) * 0.85);
        outColor = vec4(col, uAlpha);
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
uniform int uTheme;
out vec4 outColor;

float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

void main()
{
    vec2 uv = vUV;
    if (uTheme == 1) {
        // Bầu trời Minecraft: gradient xanh + mây khối trôi ngang + mặt trời vuông.
        vec3 sky = mix(vec3(0.72, 0.84, 1.0), vec3(0.45, 0.63, 0.98), smoothstep(0.25, 1.0, uv.y));
        vec2 px = uv * uRes;
        vec2 cell = floor((px + vec2(uTime * 12.0, 0.0)) / 16.0);
        float cluster = hash(floor(cell / vec2(7.0, 2.0)) + 17.0);
        float cloud = step(0.80, cluster) * step(0.25, hash(cell)) * step(0.62, uv.y);
        sky = mix(sky, vec3(0.97, 0.98, 1.0), cloud * 0.9);
        vec2 sun = abs(px - vec2(0.82, 0.86) * uRes);
        if (max(sun.x, sun.y) < 0.045 * uRes.y) sky = vec3(1.0, 0.97, 0.75);
        outColor = vec4(sky, 1.0);
        return;
    }
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
