#version 440

// Nala's body. Everything the mascot is made of -- silhouette, eyes and the
// notification badge -- is a signed distance field evaluated per pixel, so the
// shape stays crisp at any size and morphs between forms by blending distances
// rather than by interpolating vertices.

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    vec4 bodyColor;
    vec4 bodyColor2;
    vec4 eyeColor;
    vec4 badgeColor;

    // Silhouette: blend `formMix` of the way from `formA` to `formB`.
    float formA;
    float formB;
    float formMix;

    // Whole-body deformation.
    float squashX;
    float squashY;
    float bodyScale;
    // Inverse of the 2x2 that projects her onto the screen, as
    // (m00, m01, m10, m11). Carries both the in-plane lean and the tumble,
    // which is a rotation in three dimensions: a flat plate turning away from
    // the viewer foreshortens, and an in-plane rotation alone cannot.
    vec4 bodyTransform;

    // Eyes, in units of the body radius, relative to the body centre. Each
    // carries its own scale because the two sit at different places on the
    // sphere and so foreshorten by different amounts.
    vec2 eyeLeft;
    vec2 eyeRight;
    vec2 eyeLeftScale;
    vec2 eyeRightScale;
    float eyeLeftAngle;
    float eyeRightAngle;
    float eyeWidth;
    float eyeHeight;
    float eyeRound;

    // Extras.
    float badge;      // notification dot, 0..1
    float dotsSpread; // "..." separation, 0..1
    float dotsShrink; // how far the core has collapsed into the "..." run
    float dotsPhase;  // travelling emphasis around the "..." run
    float keyPulse;   // energy pulse on keyboard strike, 0..1
    float furType;    // 0 = none, 1 = calico, 2 = tabby, 3 = leopard, 4 = cow, 5 = siamese, 6 = tiger
    float furParam1;
    float furParam2;
};

// Every constant below is a half-extent in the item's [-1,1] space, measured
// from the reference animation frame by frame. See docs/animation.md.
const float kRadius = 0.529; // idle body radius
const float kDotGap = 0.285; // "..." dot spacing
const float kDotSide = 0.092;
const float kDotCore = 0.092; // all three are the same size when quiet

float sdCircle(vec2 p, float r) { return length(p) - r; }

float sdEllipse(vec2 p, vec2 r) {
    // Cheap bounded approximation: exact enough for a silhouette this smooth.
    float k = length(p / r);
    return (k - 1.0) * min(r.x, r.y);
}

float sdRoundBox(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

float sdSegment(vec2 p, vec2 a, vec2 b, float r) {
    vec2 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

float sdHexagon(vec2 p, float r) {
    const vec3 k = vec3(-0.866025404, 0.5, 0.577350269);
    p = abs(p);
    p -= 2.0 * min(dot(k.xy, p), 0.0) * k.xy;
    p -= vec2(clamp(p.x, -k.z * r, k.z * r), r);
    return length(p) * sign(p.y);
}

float sdTriangle(vec2 p, float r) {
    const float k = 1.7320508;
    p.x = abs(p.x) - r;
    p.y = p.y + r / k;
    if (p.x + k * p.y > 0.0)
        p = vec2(p.x - k * p.y, -k * p.x - p.y) * 0.5;
    p.x -= clamp(p.x, -2.0 * r, 0.0);
    return -length(p) * sign(p.y);
}

vec2 rotate(vec2 p, float a) {
    float c = cos(a), s = sin(a);
    return vec2(c * p.x - s * p.y, s * p.x + c * p.y);
}

// Smooth union -- lets separate lumps fuse into one another the way the real
// mascot's parts do when it reassembles.
float smoothUnion(float a, float b, float k) {
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}

// The "..." run. At spread 0 with a full core this is exactly the idle circle,
// which is what makes the collapse into three dots one continuous motion.
// How emphasised dot `i` is right now, 0..1.
float dotPulse(int i) {
    float w = max(0.0, cos(dotsPhase - float(i) * 2.0943951));
    return w * w;
}

// The emphasised dot swells as well as darkening: measured across the
// reference's run it goes from 0.046 R when quiet to 0.058-0.061 R when loud.
// Gated on the spread so the swell cannot distort the collapse itself.
float dotScale(int i) {
    return mix(1.0, 1.0 + 0.30 * dotPulse(i),
               smoothstep(0.55, 1.0, dotsSpread));
}

// Opacity. The quiet dots sit around half darkness in the reference (0.49 to
// 0.58) rather than a third, against the loud one's 0.64 to 0.88.
float dotWave(int i) { return 0.55 + 0.40 * dotPulse(i); }

float dotCore() { return mix(kRadius, kDotCore, dotsShrink) * dotScale(1); }
float dotSide(int i) { return kDotSide * dotsSpread * dotScale(i); }
float dotGap() { return kDotGap * dotsSpread; }

float sdDots(vec2 p, float spread, float shrink) {
    float gap = kDotGap * spread;
    float d = sdCircle(p, mix(kRadius, kDotCore, shrink) * dotScale(1));
    d = min(d, sdCircle(p - vec2(gap, 0.0), kDotSide * spread * dotScale(2)));
    d = min(d, sdCircle(p + vec2(gap, 0.0), kDotSide * spread * dotScale(0)));
    return d;
}

// In the reference the three dots pulse in turn, so the run reads as a
// progress indicator rather than as punctuation.
//
// A pixel takes the emphasis of the dot it actually belongs to -- picked by
// smallest distance field, not nearest centre. While the core is still most of
// her body the two side dots sit inside it, and going by centres would hand
// half the body to them and grey it out on the way in.
float dotsEmphasis(vec2 p) {
    float core = dotCore(), gap = dotGap();

    // The core only joins the pulse once it is genuinely a dot; until then it
    // is still her body, and her body is solid.
    float formed = smoothstep(0.80, 1.0, dotsShrink);

    float best = sdCircle(p, core);
    float emphasis = mix(1.0, dotWave(1), formed);

    float dLeft = sdCircle(p + vec2(gap, 0.0), dotSide(0));
    if (dLeft < best) { best = dLeft; emphasis = dotWave(0); }

    float dRight = sdCircle(p - vec2(gap, 0.0), dotSide(2));
    if (dRight < best) { best = dRight; emphasis = dotWave(2); }

    return emphasis;
}

// The stem holds a constant width rather than tapering: sampled down the
// reference it reads 0.250, 0.255, 0.255, 0.255, 0.255, 0.255, 0.252, 0.250.
// Tapering it, which is the intuitive thing to do, costs about 0.02 of IoU.
float sdExclaim(vec2 p) {
    p = rotate(p, 0.27);
    float stem = sdSegment(p, vec2(0.0, 0.217), vec2(0.0, -0.030), 0.068);
    float point = sdCircle(p - vec2(0.0, -0.215), 0.062);
    return min(stem, point);
}

// Round at the top, drawn down to a point at the bottom. The reference is
// widest 40% of the way down, which is where the ball centre sits.
float sdTeardrop(vec2 p) {
    p = rotate(p, 0.07); // the tip drifts left, as the reference's does
    float ball = sdCircle(p - vec2(0.0, 0.060), 0.225);
    float tip = sdSegment(p, vec2(0.0, 0.060), vec2(0.0, -0.272), 0.026);
    return smoothUnion(ball, tip, 0.15);
}

// Wider at the bottom than the top, rather than a plain ellipse.
float sdEgg(vec2 p) {
    p.y += 0.025;
    float taper = 1.0 - 0.15 * clamp(p.y / 0.50, -1.0, 1.0);
    return sdEllipse(vec2(p.x / taper, p.y), vec2(0.410, 0.50)) * taper;
}

// 5-point rounded cute star
float sdStar5(vec2 p, float r, float rf) {
    const vec2 k1 = vec2(0.809016994375, -0.587785252292);
    const vec2 k2 = vec2(-k1.x, k1.y);
    p.x = abs(p.x);
    p -= 2.0 * max(dot(k1, p), 0.0) * k1;
    p -= 2.0 * max(dot(k2, p), 0.0) * k2;
    p.x = abs(p.x);
    p.y -= r;
    vec2 ba = rf * vec2(-k1.y, k1.x) - vec2(0.0, 1.0);
    float h = clamp(dot(p, ba) / dot(ba, ba), 0.0, r);
    return length(p - ba * h) * sign(p.y * ba.x - p.x * ba.y);
}

// Smooth cute plump mascot heart
float sdHeart(vec2 p) {
    p.x = abs(p.x);
    p.y += 0.06;
    float circles = length(p - vec2(0.18, 0.12)) - 0.25;
    float bot = sdTriangle(vec2(p.x, -p.y - 0.15), 0.38) - 0.12;
    return min(circles, bot);
}

// Fluffy puffy cloud
float sdCloud(vec2 p) {
    p.y += 0.04;
    float cMid = sdCircle(p - vec2(0.0, 0.10), 0.28);
    float cLeft = sdCircle(p - vec2(-0.24, -0.02), 0.22);
    float cRight = sdCircle(p - vec2(0.24, -0.02), 0.22);
    float cBottom = sdRoundBox(p - vec2(0.0, -0.12), vec2(0.26, 0.10), 0.10);
    return min(min(cMid, min(cLeft, cRight)), cBottom);
}

// 5-petal flower blossom
float sdBlossom(vec2 p) {
    float angle = atan(p.y, p.x);
    float r = length(p);
    float wave = 0.32 + 0.13 * cos(5.0 * angle);
    return r - wave;
}

float form(vec2 p, float which) {
    int f = int(which + 0.5);
    if (f == 1) return sdEgg(p);
    // Pointy-top, and rounded hard: the reference hexagon is much softer than
    // a textbook one, and measures taller than it is wide.
    if (f == 2) return sdHexagon(rotate(p, 1.5707963), 0.33) - 0.16;
    if (f == 3) return sdTriangle(p, 0.35) - 0.175;
    if (f == 4) return sdExclaim(p);
    if (f == 5) return sdTeardrop(p);
    if (f == 6) return sdDots(p, dotsSpread, dotsShrink);
    if (f == 7) return sdCircle(p, 0.083);
    if (f == 8) return sdStar5(p, 0.42, 0.50) - 0.06;
    if (f == 9) return sdHeart(p);
    if (f == 10) return sdCloud(p);
    if (f == 11) return sdBlossom(p);
    return sdCircle(p, kRadius);
}

float isDots(float which) { return int(which + 0.5) == 6 ? 1.0 : 0.0; }

// Which forms carry a face.
float faceWeight(float which) {
    int f = int(which + 0.5);
    return (f == 0 || f == 1 || f == 2 || f == 3 || f == 8 || f == 9 || f == 10 || f == 11) ? 1.0 : 0.0;
}

// A single eye: a rounded vertical slit that can flatten to a blink dash or
// open out into a near-circle.
float sdEye(vec2 p, vec2 centre, vec2 squash, float lean) {
    vec2 extent = vec2(eyeWidth, eyeHeight) * squash * kRadius;
    float r = min(extent.x, extent.y) * eyeRound;
    return sdRoundBox(rotate(p - centre * kRadius, lean), extent, r);
}

// --- Procedural fur patterns -----------------------------------------------

float furHash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float furNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = furHash(i);
    float b = furHash(i + vec2(1.0, 0.0));
    float c = furHash(i + vec2(0.0, 1.0));
    float d = furHash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float furFbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    mat2 rot = mat2(0.8, -0.6, 0.6, 0.8);
    for (int i = 0; i < 4; ++i) {
        v += a * furNoise(p);
        p = rot * p * 2.02;
        a *= 0.5;
    }
    return v;
}

vec3 computeCalico(vec2 p) {
    // Calico cat: bold distinct ginger & espresso patches with fur feathering, creamy white belly/muzzle
    float strandNoise = (furNoise(p * 20.0) - 0.5) * 0.07 + (furNoise(p * 40.0) - 0.5) * 0.035;

    // Ginger patch on right ear/head and upper flank
    vec2 pGinger = p - vec2(0.22, 0.16);
    float dGinger = length(pGinger * vec2(1.1, 0.9)) + furNoise(p * 4.2) * 0.16 + strandNoise;
    float gingerMask = smoothstep(0.38, 0.26, dGinger);

    // Espresso patch on left ear/head and shoulder
    vec2 pBlack = p - vec2(-0.25, 0.18);
    float dBlack = length(pBlack * vec2(1.2, 0.85)) + furNoise((p + vec2(3.1, 1.7)) * 4.2) * 0.16 + strandNoise;
    float blackMask = smoothstep(0.36, 0.24, dBlack) * (1.0 - gingerMask * 0.8);

    // Warm secondary patch on right hip
    vec2 pPatch3 = p - vec2(0.28, -0.22);
    float dPatch3 = length(pPatch3 * vec2(1.3, 1.0)) + furNoise((p + vec2(7.2, 4.3)) * 4.5) * 0.14 + strandNoise;
    float patch3 = smoothstep(0.24, 0.15, dPatch3) * (1.0 - gingerMask) * (1.0 - blackMask);

    // Muzzle & chest stay clean warm white
    float bellyWhite = smoothstep(-0.02, -0.32, p.y) * smoothstep(0.38, 0.0, abs(p.x));
    gingerMask *= (1.0 - bellyWhite * 0.75);
    blackMask *= (1.0 - bellyWhite * 0.75);

    vec3 whiteFur = vec3(0.96, 0.95, 0.92);
    vec3 gingerFur = mix(vec3(0.90, 0.48, 0.17), vec3(0.78, 0.38, 0.13), clamp(p.y * 0.8 + 0.5, 0.0, 1.0));
    vec3 blackFur = mix(vec3(0.16, 0.13, 0.12), vec3(0.24, 0.18, 0.16), clamp(p.y * 0.6 + 0.4, 0.0, 1.0));

    vec3 col = whiteFur;
    col = mix(col, gingerFur, gingerMask);
    col = mix(col, blackFur, blackMask);
    col = mix(col, blackFur, patch3 * 0.85);
    return col;
}

vec3 computeTabby(vec2 p) {
    float belly = smoothstep(0.1, -0.45, p.y);
    vec3 baseCoat = mix(vec3(0.68, 0.56, 0.46), vec3(0.88, 0.82, 0.74), belly);

    float bend = sin(p.y * 7.0 + p.x * 2.0) * 0.12;
    float stripeNoise = furFbm(vec2(p.x * 4.0, p.y * 6.0));
    float wave = sin((p.x + bend) * 16.0 + stripeNoise * 3.0);
    float stripeMask = smoothstep(0.25, 0.75, wave) * (1.0 - belly * 0.6);

    vec3 stripeColor = vec3(0.22, 0.17, 0.14);
    return mix(baseCoat, stripeColor, stripeMask * 0.85);
}

vec3 computeLeopard(vec2 p) {
    vec2 uv = p * 7.5;
    vec2 g = floor(uv);
    vec2 f = fract(uv);
    float d1 = 1.0;

    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 pt = vec2(furHash(g + neighbor), furHash(g + neighbor + 31.41));
            vec2 diff = neighbor + pt - f;
            float dist = length(diff);
            if (dist < d1) {
                d1 = dist;
            }
        }
    }

    float noisePerturb = furNoise(uv * 2.0) * 0.15;
    float d = d1 + noisePerturb;

    float ring = smoothstep(0.18, 0.28, d) * smoothstep(0.44, 0.34, d);
    float centerSpot = smoothstep(0.30, 0.08, d);

    vec3 goldenFur = mix(vec3(0.94, 0.76, 0.44), vec3(0.82, 0.60, 0.32), clamp(0.5 - p.y, 0.0, 1.0));
    vec3 centerColor = vec3(0.68, 0.38, 0.14);
    vec3 ringColor = vec3(0.16, 0.12, 0.10);

    vec3 col = mix(goldenFur, centerColor, centerSpot);
    col = mix(col, ringColor, ring);
    return col;
}

vec3 computeCow(vec2 p) {
    // True Holstein cow: large, sweeping, fluid black patches with soft fur-edge definition
    float hairEdge = (furNoise(p * 28.0) - 0.5) * 0.02;

    // Spot 1: Large asymmetrical patch over upper-left ear/head
    vec2 p1 = (p - vec2(-0.28, 0.32)) * vec2(1.1, 0.88);
    float d1 = length(p1) + (furNoise(p * 4.8) - 0.5) * 0.08 + hairEdge;
    float spot1 = smoothstep(0.28, 0.24, d1);

    // Spot 2: Large fluid patch wrapping the lower-right flank
    vec2 p2 = (p - vec2(0.26, -0.22)) * vec2(0.95, 1.15);
    float d2 = length(p2) + (furNoise((p + vec2(3.7, 5.1)) * 4.5) - 0.5) * 0.09 + hairEdge;
    float spot2 = smoothstep(0.32, 0.28, d2);

    // Spot 3: Small cute satellite spot on upper-right crest
    vec2 p3 = (p - vec2(0.28, 0.34)) * vec2(1.2, 0.95);
    float d3 = length(p3) + (furNoise((p + vec2(7.1, 2.3)) * 6.0) - 0.5) * 0.06 + hairEdge;
    float spot3 = smoothstep(0.16, 0.12, d3);

    float totalSpots = clamp(spot1 + spot2 + spot3, 0.0, 1.0);

    vec3 whiteCoat = vec3(0.97, 0.97, 0.98);
    vec3 blackSpot = vec3(0.11, 0.11, 0.13);

    return mix(whiteCoat, blackSpot, totalSpots);
}

vec3 computeSiamese(vec2 p) {
    // Seal-point Siamese: warm pearl ivory body with classic face mask, ear tips, and paws
    vec3 bodyCream = mix(vec3(0.96, 0.92, 0.86), vec3(0.89, 0.83, 0.75), clamp(0.5 - p.y * 0.6, 0.0, 1.0));

    // Dark face mask covering eyes, bridge, and muzzle
    vec2 facePos = (p - vec2(0.0, 0.05)) * vec2(0.85, 1.15);
    float faceDist = length(facePos) + (furNoise(p * 14.0) - 0.5) * 0.05;
    float faceMask = smoothstep(0.32, 0.06, faceDist);

    // Ear points at upper corners
    float earLeft = smoothstep(0.30, 0.06, length((p - vec2(-0.28, 0.36)) * vec2(1.0, 1.25)));
    float earRight = smoothstep(0.30, 0.06, length((p - vec2(0.28, 0.36)) * vec2(1.0, 1.25)));
    float earPoints = max(earLeft, earRight);

    // Paw / tail points at very bottom
    float paws = smoothstep(-0.25, -0.48, p.y);

    float point = clamp(faceMask * 0.92 + earPoints * 0.88 + paws * 0.75, 0.0, 1.0);

    vec3 sealBrown = vec3(0.18, 0.12, 0.10);
    vec3 warmMocha = vec3(0.38, 0.25, 0.20);
    vec3 pointCol = mix(warmMocha, sealBrown, clamp(point * 1.3 - 0.2, 0.0, 1.0));

    return mix(bodyCream, pointCol, point);
}

vec3 computeDogFur(vec2 p, vec3 baseCol) {
    // Fluffy canine fur: directional strand flow, guard hairs, clump depth, highlights
    float flare = p.x * 0.7;
    vec2 flowUV = vec2(p.x * 22.0 + flare, p.y * 7.5 - abs(p.x) * 3.5);

    float hair1 = furNoise(flowUV);
    float hair2 = furNoise(flowUV * 1.9 + vec2(5.3, 8.1));
    float strands = hair1 * 0.6 + hair2 * 0.4;

    float clump = furFbm(vec2(p.x * 5.0, p.y * 3.5));
    float fluffLight = smoothstep(0.1, -0.4, p.y) * 0.12;

    float hairShade = (strands - 0.5) * 0.45 + (clump - 0.5) * 0.22 + fluffLight;

    vec3 shadowCol = baseCol * 0.72;
    vec3 highlightCol = mix(baseCol, vec3(1.0), 0.28);

    vec3 fur = mix(baseCol, shadowCol, clamp(-hairShade * 1.4, 0.0, 1.0));
    fur = mix(fur, highlightCol, clamp(hairShade * 1.2, 0.0, 1.0));
    return fur;
}

vec3 computeTiger(vec2 p) {
    float belly = smoothstep(0.0, -0.46, p.y);
    vec3 orangeFur = mix(vec3(0.94, 0.46, 0.08), vec3(0.96, 0.92, 0.86), belly);

    float stripeNoise = furFbm(vec2(p.x * 3.5, p.y * 7.5));
    float s = abs(sin(p.x * 12.0 + stripeNoise * 3.2));
    float stripe = smoothstep(0.68, 0.82, s) * (1.0 - belly * 0.5);

    vec3 blackStripe = vec3(0.12, 0.09, 0.08);
    return mix(orangeFur, blackStripe, stripe);
}

void main() {
    vec2 p = (qt_TexCoord0 - 0.5) * 2.0;

    // Undo the body transform so the SDFs stay in their own tidy space.
    p /= max(bodyScale, 0.0001);
    p = vec2(bodyTransform.x * p.x + bodyTransform.y * p.y,
             bodyTransform.z * p.x + bodyTransform.w * p.y);
    p /= vec2(max(squashX, 0.0001), max(squashY, 0.0001));
    p.y = -p.y; // screen y grows downward; the shapes are authored y-up

    float a = form(p, formA);
    float b = form(p, formB);
    float body = mix(a, b, formMix);

    // One pixel, measured in the same space the distances live in.
    float px = fwidth(p.x) * 1.15;

    float bodyMask = clamp(0.5 - body / px, 0.0, 1.0);

    float eyes = min(
        sdEye(p, vec2(eyeLeft.x, -eyeLeft.y), eyeLeftScale, eyeLeftAngle),
        sdEye(p, vec2(eyeRight.x, -eyeRight.y), eyeRightScale, eyeRightAngle));
    // Eyes only exist where there is body to carve them out of, and only on
    // the forms that have a face at all.
    float face = mix(faceWeight(formA), faceWeight(formB), formMix);
    float eyeMask = clamp(0.5 - eyes / px, 0.0, 1.0) * bodyMask * face;
    float eyeRimMask = 0.0;
    if (furType > 0.5) {
        float eyeRimD = eyes - px * 2.0;
        eyeRimMask = clamp(0.5 - eyeRimD / px, 0.0, 1.0) * bodyMask * face;
    }

    // Measured on the reference: radius 0.142 R, centred 1.008 R out from her
    // middle at 41.8 degrees -- so it sits right on the rim.
    float badgeD = sdCircle(p - vec2(0.397, 0.354), 0.0751 * badge);
    float badgeMask = badge > 0.001 ? clamp(0.5 - badgeD / px, 0.0, 1.0) : 0.0;

    // The badge sits on the rim and pokes outside the silhouette, so it
    // contributes coverage of its own rather than only recolouring the body.
    float gradT = clamp(0.5 - p.y / (2.0 * kRadius), 0.0, 1.0);
    vec3 baseBodyColor = mix(bodyColor.rgb, bodyColor2.rgb, gradT);
    if (furType > 0.5) {
        vec3 furCol = baseBodyColor;
        if (furType < 1.5) {
            furCol = computeCalico(p);
        } else if (furType < 2.5) {
            furCol = computeTabby(p);
        } else if (furType < 3.5) {
            furCol = computeLeopard(p);
        } else if (furType < 4.5) {
            furCol = computeCow(p);
        } else if (furType < 5.5) {
            furCol = computeSiamese(p);
        } else if (furType < 6.5) {
            furCol = computeTiger(p);
        } else {
            furCol = computeDogFur(p, baseBodyColor);
        }
        // Subtle micro-noise for soft fur texture
        float microNoise = (furNoise(p * 45.0) - 0.5) * 0.035;
        baseBodyColor = clamp(furCol + microNoise, 0.0, 1.0);
    }
    float bodyAlpha = mix(bodyColor.a, bodyColor2.a, gradT);
    float alpha = max(bodyMask * bodyAlpha, badgeMask);

    // Fade the quiet dots rather than greying them: on a dark desktop a fixed
    // grey would read as a smudge, while lower opacity reads correctly on any
    // wallpaper.
    // Gate on how far the core has actually collapsed, not on the morph: while
    // the body is still a body it must stay solid, or the whole silhouette
    // turns translucent on the way into the run.
    float dotsWeight = mix(isDots(formA), isDots(formB), formMix) *
                       smoothstep(0.15, 0.55, dotsSpread);
    if (dotsWeight > 0.001) {
        // On the way in only the emerging side dots are quiet; the core is
        // still her body, and her body is solid. Once the core has become a
        // dot in its own right it joins the pulse. Testing against the side
        // dots' own fields -- rather than against the blended silhouette,
        // which reaches past them -- is what keeps the body from dimming.
        float gap = dotGap();
        float dSide = min(sdCircle(p + vec2(gap, 0.0), dotSide(0)),
                          sdCircle(p - vec2(gap, 0.0), dotSide(2)));
        float inSide = 1.0 - step(0.0, dSide);
        float formed = smoothstep(0.80, 1.0, dotsShrink);
        float apply = dotsWeight * max(formed, inSide);
        alpha *= mix(1.0, dotsEmphasis(p), apply);
    }

    vec3 rgb = baseBodyColor;
    if (keyPulse > 0.001) {
        rgb = mix(rgb, vec3(1.0), keyPulse * 0.45);
    }
    if (eyeRimMask > 0.001) {
        rgb = mix(rgb, vec3(0.12, 0.10, 0.09), eyeRimMask);
    }
    rgb = mix(rgb, eyeColor.rgb, eyeMask);
    rgb = mix(rgb, badgeColor.rgb, badgeMask);

    fragColor = vec4(rgb * alpha, alpha) * qt_Opacity;
}
