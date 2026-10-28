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
layout (location = 9) in vec4 iExtra;    // x: 1 = không tính ánh sáng, y: vật liệu

uniform mat4 uViewProj;

out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vObjNormal;
out vec3 vLocal;
flat out mat3 vNormalMat;   // đổi pháp tuyến vật thể -> thế giới (cho cạnh vát tính trong fragment)
flat out vec4 vColor;
flat out vec4 vAccent;
flat out vec4 vScale;
flat out float vUnlit;
flat out float vMaterial;

void main()
{
    vec4 world = iModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    vNormalMat = transpose(inverse(mat3(iModel)));
    vNormal = normalize(vNormalMat * aNormal);
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

// ---------- Lượt bóng đổ: chỉ ghi độ sâu nhìn từ nguồn sáng ----------
static const char *SHADOW_VERT = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in mat4 iModel;
layout (location = 7) in vec4 iAccent;
uniform mat4 uLightVP;
out float vAlpha;
void main()
{
    vAlpha = iAccent.a;
    gl_Position = uLightVP * iModel * vec4(aPos, 1.0);
}
)GLSL";

static const char *SHADOW_FRAG = R"GLSL(
#version 330 core
in float vAlpha;
void main()
{
    if (vAlpha < 0.5) discard;   // vật mờ (cột sáng, hạt) không đổ bóng
}
)GLSL";

static const char *BOX_FRAG = R"GLSL(
#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vObjNormal;
in vec3 vLocal;
flat in mat3 vNormalMat;
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
uniform int uTheme;       // 0 Classic, 1 Minecraft, 2 Original
uniform mat4 uLightVP;
uniform sampler2DShadow uShadowMap;
uniform int uShadowOn;

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

// ---------- Nhiễu giá trị (vân đá, gỉ sét, khói) ----------
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash2(i), b = hash2(i + vec2(1.0, 0.0));
    float c = hash2(i + vec2(0.0, 1.0)), d = hash2(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p)
{
    float s = 0.0, a = 0.5;
    for (int k = 0; k < 4; k++) { s += a * vnoise(p); p = p * 2.03 + vec2(17.1, 9.3); a *= 0.5; }
    return s;
}

// Bóng đổ thật từ shadow map (PCF 3x3 trên sampler so sánh có lọc tuyến tính -> mép bóng mềm).
float shadowFactor(vec3 Nf)
{
    if (uShadowOn == 0) return 1.0;
    vec4 lp = uLightVP * vec4(vWorldPos + Nf * 0.025, 1.0);
    vec3 s = lp.xyz / lp.w * 0.5 + 0.5;
    if (s.x <= 0.0 || s.x >= 1.0 || s.y <= 0.0 || s.y >= 1.0 || s.z >= 1.0) return 1.0;
    vec2 texel = 1.0 / vec2(textureSize(uShadowMap, 0));
    float sum = 0.0;
    for (int x = -1; x <= 1; x++)
        for (int y = -1; y <= 1; y++)
            sum += texture(uShadowMap, vec3(s.xy + vec2(x, y) * texel * 1.4, s.z - 0.0006));
    return sum / 9.0;
}

// Màu bầu trời phản chiếu lên bề mặt bóng (khớp với nền của từng theme).
vec3 skyColor(vec3 r)
{
    float h = clamp(r.z * 0.5 + 0.5, 0.0, 1.0);
    if (uTheme == 2)
        return mix(vec3(0.20, 0.03, 0.01), vec3(1.00, 0.52, 0.16), smoothstep(0.35, 0.95, h));
    return mix(vec3(0.01, 0.015, 0.04), vec3(0.30, 0.48, 0.75), smoothstep(0.45, 1.0, h));
}

// ---------- Theme Original: hoa văn nổi trên công tắc (hàm độ cao 0..1 trên mặt ô, c = uv - 0.5) ----------
float sdBox2(vec2 p, vec2 b) { vec2 d = abs(p) - b; return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0); }

float embossHeight(int mat, vec2 c)
{
    if (mat == 4) {                               // công tắc nặng: chữ X nổi
        vec2 r = mat2(0.7071, -0.7071, 0.7071, 0.7071) * c;
        float d = min(sdBox2(r, vec2(0.30, 0.062)), sdBox2(r.yx, vec2(0.30, 0.062))) - 0.012;
        return smoothstep(0.03, -0.03, d);
    }
    if (mat == 3) {                               // công tắc thường: nút tròn
        float r = length(c);
        return smoothstep(0.285, 0.255, r) * 0.55 + sqrt(max(0.0, 1.0 - (r / 0.21) * (r / 0.21))) * 0.45;
    }
    if (mat == 5) {                               // công tắc tách khối: hai ngoặc ( )
        float d = abs(length(c) - 0.26) - 0.045;
        d = max(d, 0.11 - abs(c.x));
        return smoothstep(0.03, -0.03, d);
    }
    return 0.0;
}

// Vật liệu theme Original. Trả về màu gốc; đặt độ bóng, phản chiếu và pháp tuyến (đã tính hoa văn nổi).
vec3 origMaterial(int mat, vec2 uv, vec2 sz, int face, inout vec3 N, out float specK, out float shin,
                  out float envK, out float occl)
{
    specK = 0.30; shin = 40.0; envK = 0.03; occl = 1.0;
    vec2 wp = vWorldPos.xy;
    if (mat == 7 || mat == 8) {                   // khối: kim loại gỉ sét
        vec2 q = uv * sz * 2.2 + vec2(float(face) * 7.3, abs(vObjNormal.x) * 3.1 + abs(vObjNormal.y) * 5.7);
        float n = fbm(q * 1.6), n2 = fbm(q * 4.5 + 11.0);
        vec3 metal = vec3(0.24, 0.17, 0.13);
        vec3 rust = vec3(0.55, 0.25, 0.09);
        vec3 dark = vec3(0.13, 0.08, 0.06);
        vec3 c = mix(metal, rust, smoothstep(0.42, 0.68, n));
        c = mix(c, dark, smoothstep(0.62, 0.80, n2) * 0.7);
        c *= 0.85 + 0.30 * n2;
        if (mat == 8) c = mix(c, vec3(0.95, 0.55, 0.18), 0.35);
        specK = 0.55; shin = 28.0; envK = 0.22;
        return c;
    }
    float n = fbm(wp * 2.6 + float(face) * 3.7);
    float vein = abs(sin((wp.x * 0.8 + wp.y * 0.5) * 5.0 + n * 7.0));
    vec3 c;
    if (mat == 2) {                               // ô cam dễ vỡ
        c = face == 0 ? vec3(0.95, 0.52, 0.14) : vec3(0.58, 0.27, 0.07);
        c *= 0.86 + 0.18 * n;
    } else if (mat == 9) {                        // khung lỗ đích: sắt tối
        c = vec3(0.22, 0.20, 0.18) * (0.8 + 0.3 * n);
        specK = 0.2;
        return c;
    } else {                                      // sàn, cầu, nền công tắc: đá xám sáng có vân
        c = face == 0 ? vec3(0.76, 0.79, 0.84) : vec3(0.36, 0.38, 0.43);
        c *= 0.88 + 0.16 * n;
        if (face == 0) c *= mix(0.93, 1.0, smoothstep(0.0, 0.08, vein));
        if (mat == 6) c *= vec3(1.0, 0.97, 0.92);
    }
    if (face == 1) c *= mix(0.62, 1.0, clamp(uv.y, 0.0, 1.0));   // mặt bên tối dần xuống đáy
    if (face == 0 && (mat == 3 || mat == 4 || mat == 5)) {
        vec2 cc = uv - 0.5;
        float e = 0.004, A = 0.045;
        float h = embossHeight(mat, cc);
        float hx = (embossHeight(mat, cc + vec2(e, 0.0)) - embossHeight(mat, cc - vec2(e, 0.0))) / (2.0 * e);
        float hy = (embossHeight(mat, cc + vec2(0.0, e)) - embossHeight(mat, cc - vec2(0.0, e))) / (2.0 * e);
        N = normalize(vec3(-A * hx, -A * hy, 1.0));
        vec2 toL = normalize(-uLightDir.xy);
        occl = 1.0 - 0.5 * clamp(embossHeight(mat, cc + toL * 0.05) - h, 0.0, 1.0);
        vec3 metal = vec3(0.56, 0.57, 0.60) * (0.9 + 0.15 * n);
        c = mix(c, metal, smoothstep(0.02, 0.25, h));
        specK = mix(specK, 0.65, h); shin = mix(shin, 60.0, h);
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

    vec3 Nf = normalize(vNormal);   // pháp tuyến phẳng của mặt
    bool top = Nf.z > 0.5;
    vec3 base = uColor;
    float glow = uEmissive;

    if (uPattern == 4) {   // bóng mềm dưới khối: đậm ở giữa, nhạt dần ra mép
        float s = smoothstep(0.0, 0.35, edge);
        outColor = vec4(0.0, 0.0, 0.0, uAlpha * s * (uTheme == 1 ? 0.6 : 1.0));
        return;
    }

    int mat = int(vMaterial + 0.5);
    float sh = vUnlit > 0.5 ? 1.0 : shadowFactor(Nf);
    float dist = length(uEye - vWorldPos);

    if (uTheme == 1) {
        // Theme Minecraft: texture pixel + đổ bóng phẳng theo mặt (trên sáng nhất, dưới tối nhất).
        int face = Nf.z > 0.5 ? 0 : (Nf.z < -0.5 ? 2 : 1);
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
        float shade = face == 0 ? 1.0 : (face == 2 ? 0.5 : (abs(Nf.x) > 0.5 ? 0.80 : 0.64));
        vec3 col = vUnlit > 0.5 ? tex : tex * shade * mix(0.58, 1.0, sh);
        col += tex * (g2 + (mat > 0 ? 0.0 : uEmissive)) + uColor * uEmissive * (mat > 0 ? 0.35 : 0.0);
        col = mix(col, uFogColor, smoothstep(uFogRange.x, uFogRange.y, dist) * 0.85);
        outColor = vec4(col, uAlpha);
        return;
    }

    // Cạnh vát: nghiêng pháp tuyến ở dải hẹp sát mép hộp -> cạnh bắt sáng, trông bo tròn và bóng.
    vec3 N = Nf;
    if (vUnlit < 0.5) {
        vec3 pl = vLocal * uScale;
        float r = min(0.05, 0.3 * min(uScale.x, min(uScale.y, uScale.z)));
        vec3 t = vec3(0.0);
        for (int k = 0; k < 3; k++)
            if (an[k] < 0.5)
                t[k] = smoothstep(r, 0.0, uScale[k] - pl[k]) - smoothstep(r, 0.0, pl[k]);
        N = normalize(vNormalMat * normalize(vObjNormal + 0.9 * t));
    }
    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uEye - vWorldPos);

    vec3 color;
    if (uTheme == 2) {
        // Theme Original: đá xám sáng, ô cam, công tắc kim loại nổi, khối sắt gỉ, trời đỏ cam.
        int face = Nf.z > 0.5 ? 0 : (Nf.z < -0.5 ? 2 : 1);
        float specK, shin, envK, occl;
        vec3 Ns = N;
        vec3 alb = mat > 0 ? origMaterial(mat, uv, sz, face, Ns, specK, shin, envK, occl) : uColor;
        if (mat == 0) { specK = 0.25; shin = 32.0; envK = 0.05; occl = 1.0; }
        if (face == 0 && (mat == 3 || mat == 4 || mat == 5)) N = Ns;
        if (vUnlit > 0.5) {
            color = alb;
        } else {
            vec3 H = normalize(L + V);
            float diff = max(dot(N, L), 0.0) * sh;
            float spec = pow(max(dot(N, H), 0.0), shin) * specK * sh;
            float fres = pow(1.0 - max(dot(N, V), 0.0), 4.0);
            vec3 amb = mix(vec3(0.14, 0.10, 0.10), vec3(0.36, 0.37, 0.41), 0.5 + 0.5 * N.z);
            color = alb * (amb + vec3(1.0, 0.95, 0.90) * 0.80 * diff) * occl;
            color += vec3(1.0, 0.85, 0.70) * spec;
            color += skyColor(reflect(-V, N)) * (envK + (envK > 0.1 ? 0.30 : 0.10) * fres) * (0.5 + 0.5 * sh);
            float border = smoothstep(0.008, 0.03, edge);
            color *= mix(0.55, 1.0, border);
        }
        if (mat != 9) color += alb * glow * 0.6 + uAccent * glow * 0.1;
    } else {
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
        if (vUnlit > 0.5) {
            color = base;
        } else {
            bool shiny = mat == 7 || mat == 8;   // khối người chơi: sơn bóng
            vec3 H = normalize(L + V);
            float diff = max(dot(N, L), 0.0) * sh;
            float spec = pow(max(dot(N, H), 0.0), shiny ? 80.0 : 48.0) * (shiny ? 0.9 : 0.24) * sh;
            float fres = pow(1.0 - max(dot(N, V), 0.0), 3.0);
            float sky = 0.5 + 0.5 * N.z;                      // ánh sáng môi trường: trên sáng, dưới tối
            color = base * (0.22 + 0.18 * sky + 0.70 * diff) + vec3(spec) + vec3(fres * (shiny ? 0.12 : 0.06));
            color += skyColor(reflect(-V, N)) * (shiny ? 0.10 + 0.45 * fres : 0.03 + 0.08 * fres);

            // Viền tối + gờ sáng ở mép mặt trên (giống Bloxorz gốc).
            float border = smoothstep(0.015, 0.045, edge);
            color *= mix(0.30, 1.0, border);
            if (top) {
                float bevel = (1.0 - smoothstep(0.045, 0.09, edge)) * border;
                color += bevel * 0.12;
            }
        }
        color += uColor * glow + uAccent * glow * 0.15;
    }

    // Sương mù theo khoảng cách, hòa vào màu nền.
    float fog = smoothstep(uFogRange.x, uFogRange.y, dist);
    color = mix(color, uFogColor, fog * 0.85);

    outColor = vec4(color, uAlpha);
}
)GLSL";

// ---------- Nền: vẽ bằng một tam giác phủ màn hình ----------
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
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1, 0)), f.x), mix(hash(i + vec2(0, 1)), hash(i + vec2(1, 1)), f.x), f.y);
}
float fbm(vec2 p)
{
    float s = 0.0, a = 0.5;
    for (int k = 0; k < 5; k++) { s += a * vnoise(p); p = p * 2.02 + vec2(13.7, 5.1); a *= 0.5; }
    return s;
}

void main()
{
    vec2 uv = vUV;
    float aspect = uRes.x / uRes.y;
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
    if (uTheme == 2) {
        // Theme Original: trời hoàng hôn đỏ cam, khói đen cuộn chậm (nhiễu fbm uốn miền).
        vec2 p = vec2(uv.x * aspect, uv.y);
        vec3 low = vec3(0.30, 0.02, 0.01), mid = vec3(0.78, 0.10, 0.03), high = vec3(0.98, 0.40, 0.08);
        vec3 col = mix(low, mid, smoothstep(0.0, 0.50, uv.y));
        col = mix(col, high, smoothstep(0.45, 1.0, uv.y));
        col += vec3(0.35, 0.16, 0.02) * exp(-6.0 * length(p - vec2(0.62 * aspect, 0.80)));
        float t = uTime * 0.025;
        vec2 q = p * 1.5 + vec2(t, -t * 0.4);
        vec2 w = vec2(fbm(q + vec2(0.0, t)), fbm(q + vec2(5.2, 1.3 - t)));
        float smoke = fbm(q * 1.2 + w * 1.6);
        float dark = smoothstep(0.48, 0.80, smoke) * (1.0 - 0.55 * smoothstep(0.30, 0.95, uv.y));
        col = mix(col, vec3(0.07, 0.015, 0.01), dark * 0.88);
        col += vec3(1.0, 0.50, 0.15) * smoothstep(0.62, 0.88, fbm(q * 2.3 + 7.0)) * 0.12 * (1.0 - dark);
        vec2 c = uv - 0.5;
        col *= 1.0 - 0.45 * dot(c, c) * 1.6;
        outColor = vec4(col, 1.0);
        return;
    }
    vec2 c = uv - vec2(0.5, 0.55);
    float vig = 1.0 - smoothstep(0.15, 0.95, length(c * vec2(aspect, 1.0)));
    vec3 col = mix(vec3(0.0, 0.0, 0.01), vec3(0.03, 0.04, 0.08), vig);
    // Tinh vân rất mờ cho chiều sâu.
    float neb = fbm(vec2(uv.x * aspect, uv.y) * 2.0 + vec2(uTime * 0.004, 0.0));
    col += vec3(0.04, 0.06, 0.12) * smoothstep(0.45, 0.85, neb) * vig;

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

// ---------- Hậu kỳ: bloom (lọc vùng sáng -> làm nhòe -> cộng lại) ----------
static const char *POST_BRIGHT_FRAG = R"GLSL(
#version 330 core
in vec2 vUV;
uniform sampler2D uTex;
out vec4 outColor;
void main()
{
    // Lấy mẫu 4 điểm (thu nhỏ một nửa không bị răng cưa), giữ phần sáng hơn ngưỡng với "đầu gối" mềm.
    vec2 t = 1.0 / vec2(textureSize(uTex, 0));
    vec3 c = 0.25 * (texture(uTex, vUV + t * vec2(-0.5, -0.5)).rgb + texture(uTex, vUV + t * vec2(0.5, -0.5)).rgb
                   + texture(uTex, vUV + t * vec2(-0.5, 0.5)).rgb + texture(uTex, vUV + t * vec2(0.5, 0.5)).rgb);
    float l = max(c.r, max(c.g, c.b));
    float k = smoothstep(0.80, 1.35, l);
    outColor = vec4(c * k, 1.0);
}
)GLSL";

static const char *POST_BLUR_FRAG = R"GLSL(
#version 330 core
in vec2 vUV;
uniform sampler2D uTex;
uniform vec2 uDir;   // (1/w, 0) hoặc (0, 1/h)
out vec4 outColor;
void main()
{
    // Gauss 9 mẫu dùng lọc tuyến tính (5 lần đọc).
    vec3 c = texture(uTex, vUV).rgb * 0.2270270270;
    c += texture(uTex, vUV + uDir * 1.3846153846).rgb * 0.3162162162;
    c += texture(uTex, vUV - uDir * 1.3846153846).rgb * 0.3162162162;
    c += texture(uTex, vUV + uDir * 3.2307692308).rgb * 0.0702702703;
    c += texture(uTex, vUV - uDir * 3.2307692308).rgb * 0.0702702703;
    outColor = vec4(c, 1.0);
}
)GLSL";

static const char *POST_COMPOSITE_FRAG = R"GLSL(
#version 330 core
in vec2 vUV;
uniform sampler2D uScene;
uniform sampler2D uBloom1;
uniform sampler2D uBloom2;
uniform float uBloomStrength;
uniform int uTheme;
out vec4 outColor;
float shoulder(float x) { return x < 0.8 ? x : 0.8 + 0.2 * (1.0 - exp(-(x - 0.8) / 0.2)); }
void main()
{
    vec3 c = texture(uScene, vUV).rgb;
    vec3 b = texture(uBloom1, vUV).rgb * 0.65 + texture(uBloom2, vUV).rgb * 0.55;
    c += b * uBloomStrength;
    // Vai mềm thay cho cắt cứng ở 1.0: vùng tối và trung tính giữ nguyên, chỗ phát sáng không bị cháy trắng.
    c = vec3(shoulder(c.r), shoulder(c.g), shoulder(c.b));
    vec2 v = vUV - 0.5;
    c *= 1.0 - (uTheme == 1 ? 0.0 : 0.22) * dot(v, v) * 2.0;
    outColor = vec4(c, 1.0);
}
)GLSL";

#endif
