// Tesco Land - 3D tycoon pre Wii (libogc + GX + ASND). Fan hra, bez loga a bez povodnej hudby.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <ctype.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <asndlib.h>

#define SR 22050
#define N 8
#define FIFO_SIZE (256 * 1024)
#define GOAL 2000
#define MAXC 12
#define SH(c, n) ((u8)((c) * (n) / 10))

static void *xfb[2];
static int fb = 0;
static GXRModeObj *rmode;
static Mtx view;

/* ---------- 5x7 font ('#' = znak eura) ---------- */
static const char CH[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ#:.-+/!>,";
static const u8 FONT[46][7] = {
    {0, 0, 0, 0, 0, 0, 0},
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
    {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E},
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C},
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
    {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11},
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
    {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
    {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04},
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}, {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11},
    {0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04}, {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},
    {0x07, 0x08, 0x1E, 0x08, 0x1E, 0x08, 0x07}, /* # = euro */
    {0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00}, /* : */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}, /* . */
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, /* - */
    {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}, /* + */
    {0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10}, /* / */
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}, /* ! */
    {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08}, /* > */
    {0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0x08}  /* , */
};

/* ---------- zakladne kreslenie ---------- */
static void vtx(float x, float y, float z, u8 r, u8 g, u8 b) {
    GX_Position3f32(x, y, z);
    GX_Color4u8(r, g, b, 255);
}

static void box(float cx, float y0, float cz, float sx, float sy, float sz, u8 r, u8 g, u8 b) {
    float x0 = cx - sx / 2, x1 = cx + sx / 2, y1 = y0 + sy, z0 = cz - sz / 2, z1 = cz + sz / 2;
    u8 rf = SH(r, 8), gf = SH(g, 8), bf = SH(b, 8);
    u8 rs = SH(r, 6), gs = SH(g, 6), bs = SH(b, 6);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 20);
    vtx(x0, y1, z0, r, g, b);  vtx(x1, y1, z0, r, g, b);  vtx(x1, y1, z1, r, g, b);  vtx(x0, y1, z1, r, g, b);
    vtx(x0, y0, z1, rf, gf, bf); vtx(x1, y0, z1, rf, gf, bf); vtx(x1, y1, z1, rf, gf, bf); vtx(x0, y1, z1, rf, gf, bf);
    vtx(x1, y0, z0, rf, gf, bf); vtx(x0, y0, z0, rf, gf, bf); vtx(x0, y1, z0, rf, gf, bf); vtx(x1, y1, z0, rf, gf, bf);
    vtx(x0, y0, z0, rs, gs, bs); vtx(x0, y0, z1, rs, gs, bs); vtx(x0, y1, z1, rs, gs, bs); vtx(x0, y1, z0, rs, gs, bs);
    vtx(x1, y0, z1, rs, gs, bs); vtx(x1, y0, z0, rs, gs, bs); vtx(x1, y1, z0, rs, gs, bs); vtx(x1, y1, z1, rs, gs, bs);
    GX_End();
}

static void begin3d(float ex, float ey, float ez, float tx, float ty, float tz) {
    Mtx44 proj;
    guPerspective(proj, 50, 640.0f / 480.0f, 0.1f, 200.0f);
    GX_LoadProjectionMtx(proj, GX_PERSPECTIVE);
    guVector cam = {ex, ey, ez}, up = {0, 1, 0}, look = {tx, ty, tz};
    guLookAt(view, &cam, &up, &look);
    GX_LoadPosMtxImm(view, GX_PNMTX0);
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
}

static void begin2d(void) {
    Mtx44 o;
    Mtx id;
    guOrtho(o, 0, 479, 0, 639, 0, 300);
    GX_LoadProjectionMtx(o, GX_ORTHOGRAPHIC);
    guMtxIdentity(id);
    GX_LoadPosMtxImm(id, GX_PNMTX0);
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
}

static void rect2(float x, float y, float w, float h, u8 r, u8 g, u8 b) {
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    vtx(x, y, -1, r, g, b); vtx(x + w, y, -1, r, g, b); vtx(x + w, y + h, -1, r, g, b); vtx(x, y + h, -1, r, g, b);
    GX_End();
}

static void text(float x, float y, float s, const char *str, u8 r, u8 g, u8 b) {
    int count = 0;
    for (const char *q = str; *q; q++) {
        const char *p = strchr(CH, toupper((unsigned char)*q));
        int i = p ? (int)(p - CH) : 0;
        for (int row = 0; row < 7; row++)
            for (int c = 0; c < 5; c++)
                if (FONT[i][row] & (0x10 >> c)) count++;
    }
    if (!count) return;
    GX_Begin(GX_QUADS, GX_VTXFMT0, count * 4);
    float cx = x;
    for (const char *q = str; *q; q++) {
        const char *p = strchr(CH, toupper((unsigned char)*q));
        int i = p ? (int)(p - CH) : 0;
        for (int row = 0; row < 7; row++)
            for (int c = 0; c < 5; c++)
                if (FONT[i][row] & (0x10 >> c)) {
                    float px = cx + c * s, py = y + row * s;
                    vtx(px, py, -1, r, g, b); vtx(px + s, py, -1, r, g, b);
                    vtx(px + s, py + s, -1, r, g, b); vtx(px, py + s, -1, r, g, b);
                }
        cx += 6 * s;
    }
    GX_End();
}

static void label(float x, float y, float s, const char *str, u8 r, u8 g, u8 b) {
    text(x + s * 0.5f, y + s * 0.5f, s, str, 0, 0, 0);
    text(x, y, s, str, r, g, b);
}

/* ---------- zvuk (vsetko generovane v kode) ---------- */
static s16 *make_song(const int *mel, const int *bass, int n, int ms, int *bytes) {
    int spn = SR * ms / 1000;
    int total = (spn * n + 15) & ~15;
    s16 *buf = (s16 *)memalign(32, total * 2);
    memset(buf, 0, total * 2);
    for (int i = 0; i < n; i++) {
        for (int voice = 0; voice < 2; voice++) {
            int note = voice == 0 ? mel[i] : bass[i];
            if (note <= 0) continue;
            float f = 440.0f * powf(2.0f, (note - 69) / 12.0f), ph = 0;
            for (int k = 0; k < spn; k++) {
                float t = (float)k / spn;
                float env = (k < 100 ? k / 100.0f : 1.0f) * (1.0f - 0.55f * t);
                if (k > spn - 150) env *= (float)(spn - k) / 150.0f;
                ph += f / SR;
                if (ph >= 1.0f) ph -= 1.0f;
                float v;
                if (voice == 0) v = (ph < 0.5f ? 1.0f : -1.0f) * 0.17f;
                else v = (ph < 0.5f ? 4 * ph - 1 : 3 - 4 * ph) * 0.22f;
                int idx = i * spn + k;
                buf[idx] = (s16)(buf[idx] + (s16)(v * env * 32767.0f));
            }
        }
    }
    DCFlushRange(buf, total * 2);
    *bytes = total * 2;
    return buf;
}

static s16 *make_sfx(const float *fr, const int *ms, int n, int sine, float vol, int *bytes) {
    int total = 16;
    for (int i = 0; i < n; i++) total += SR * ms[i] / 1000;
    total = (total + 15) & ~15;
    s16 *b = (s16 *)memalign(32, total * 2);
    memset(b, 0, total * 2);
    int pos = 0;
    for (int i = 0; i < n; i++) {
        int len = SR * ms[i] / 1000;
        float ph = 0;
        for (int k = 0; k < len; k++) {
            float env = 1.0f - (float)k / len;
            if (k < 60) env *= k / 60.0f;
            ph += fr[i] / SR;
            if (ph >= 1.0f) ph -= 1.0f;
            float v = sine ? sinf(6.2831853f * ph) : (ph < 0.5f ? 1.0f : -1.0f);
            b[pos + k] = (s16)(v * env * vol * 32767.0f);
        }
        pos += len;
    }
    DCFlushRange(b, total * 2);
    *bytes = total * 2;
    return b;
}

static void play(int v, s16 *b, int bytes, int vol) {
    ASND_SetVoice(v, VOICE_MONO_16BIT, SR, 0, b, bytes, vol, vol, NULL);
}

/* ---------- hra ---------- */
static const char *NAMES[4] = {"SHELF", "FRIDGE", "CHECKOUT", "CAFE"};
static const int COST[4] = {40, 90, 160, 300};
static const int PAY[4] = {6, 14, 26, 50};
static const u8 PAL[6][3] = {{230, 50, 50}, {50, 160, 70}, {240, 200, 40}, {60, 110, 220}, {240, 130, 40}, {170, 70, 190}};

static int grid[N][N];

typedef struct { int on; float x, z; int gx, gz, state; u8 r, g, b; } Cust;
static Cust cu[MAXC];

static void draw_building(int t, float x, float z) {
    switch (t) {
    case 0:
        box(x, 0, z, 0.85f, 0.9f, 0.4f, 220, 220, 230);
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 4; c++) {
                const u8 *p = PAL[(r * 4 + c) % 6];
                box(x - 0.3f + c * 0.2f, 0.15f + r * 0.27f, z + 0.2f, 0.15f, 0.18f, 0.12f, p[0], p[1], p[2]);
            }
        break;
    case 1:
        box(x, 0, z, 0.75f, 1.3f, 0.6f, 235, 240, 245);
        box(x, 0.1f, z + 0.31f, 0.6f, 1.1f, 0.04f, 120, 200, 240);
        box(x + 0.25f, 0.5f, z + 0.34f, 0.04f, 0.3f, 0.03f, 80, 80, 90);
        break;
    case 2:
        box(x, 0, z, 0.9f, 0.5f, 0.55f, 120, 120, 130);
        box(x - 0.15f, 0.5f, z, 0.45f, 0.03f, 0.4f, 30, 30, 35);
        box(x + 0.3f, 0.5f, z, 0.2f, 0.3f, 0.2f, 40, 40, 50);
        box(x + 0.3f, 0.62f, z + 0.1f, 0.14f, 0.1f, 0.02f, 80, 220, 120);
        break;
    default:
        box(x, 0, z, 0.9f, 0.55f, 0.6f, 150, 90, 50);
        box(x, 0.55f, z, 0.95f, 0.06f, 0.65f, 245, 235, 215);
        box(x - 0.2f, 0.61f, z, 0.12f, 0.15f, 0.12f, 255, 255, 255);
        box(x + 0.1f, 0.61f, z, 0.12f, 0.15f, 0.12f, 255, 255, 255);
        box(x + 0.1f, 0.76f, z, 0.04f, 0.12f, 0.04f, 200, 200, 200);
        break;
    }
}

static void draw_menu_scene(void) {
    box(0, -0.3f, 0, 16, 0.3f, 16, 90, 160, 80);
    box(0, 0, 0, 4, 1.6f, 3, 240, 240, 245);
    box(0, 1.25f, 0, 4.1f, 0.14f, 3.1f, 238, 28, 46);
    box(0, 1.6f, 0, 4.3f, 0.35f, 3.3f, 0, 83, 160);
    box(0, 0, 1.52f, 0.9f, 1.1f, 0.06f, 60, 60, 70);
    box(-1.3f, 0.5f, 1.52f, 1.0f, 0.7f, 0.06f, 130, 200, 240);
    box(1.3f, 0.5f, 1.52f, 1.0f, 0.7f, 0.06f, 130, 200, 240);
    box(-4, 0, 2.5f, 0.3f, 1.0f, 0.3f, 120, 80, 40);
    box(-4, 1.0f, 2.5f, 1.2f, 1.0f, 1.2f, 40, 150, 60);
    box(4.5f, 0, 2, 0.3f, 1.0f, 0.3f, 120, 80, 40);
    box(4.5f, 1.0f, 2, 1.2f, 1.0f, 1.2f, 40, 150, 60);
}

static void draw_world(int frame, int cx, int cz, int sel, int money) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            u8 c = ((i + j) & 1) ? 236 : 205;
            box(i - 3.5f, -0.1f, j - 3.5f, 0.98f, 0.1f, 0.98f, c, c, 245);
        }
    box(0, 0, -4.2f, 8.6f, 2.2f, 0.3f, 235, 235, 240);
    box(0, 1.5f, -4.0f, 8.6f, 0.5f, 0.1f, 0, 83, 160);
    box(0, 1.35f, -3.98f, 8.6f, 0.1f, 0.12f, 238, 28, 46);
    box(-4.2f, 0, 0, 0.3f, 0.8f, 8.2f, 235, 235, 240);
    box(4.2f, 0, 0, 0.3f, 0.8f, 8.2f, 235, 235, 240);
    box(0, 0, 4.4f, 1.6f, 0.03f, 0.6f, 238, 28, 46);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (grid[i][j]) draw_building(grid[i][j] - 1, i - 3.5f, j - 3.5f);
    float pl = 0.04f * sinf(frame * 0.2f);
    int ok = (grid[cx][cz] == 0 && money >= COST[sel]);
    box(cx - 3.5f, 0, cz - 3.5f, 1.0f + pl, 0.05f, 1.0f + pl, ok ? 60 : 230, ok ? 220 : 60, ok ? 90 : 60);
    if (grid[cx][cz] == 0) draw_building(sel, cx - 3.5f, cz - 3.5f);
    for (int i = 0; i < MAXC; i++)
        if (cu[i].on) {
            float bob = 0.03f * sinf(frame * 0.3f + i);
            box(cu[i].x, 0.0f + bob, cu[i].z, 0.25f, 0.45f, 0.25f, cu[i].r, cu[i].g, cu[i].b);
            box(cu[i].x, 0.45f + bob, cu[i].z, 0.2f, 0.2f, 0.2f, 240, 200, 160);
        }
}

static int built_count(void) {
    int n = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (grid[i][j]) n++;
    return n;
}

static void spawn(void) {
    int bi[N * N], bj[N * N], n = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if (grid[i][j]) { bi[n] = i; bj[n] = j; n++; }
    if (!n) return;
    for (int k = 0; k < MAXC; k++)
        if (!cu[k].on) {
            int t = rand() % n;
            cu[k].on = 1; cu[k].x = 0; cu[k].z = 5.2f; cu[k].gx = bi[t]; cu[k].gz = bj[t]; cu[k].state = 0;
            const u8 *p = PAL[rand() % 6];
            cu[k].r = p[0]; cu[k].g = p[1]; cu[k].b = p[2];
            return;
        }
}

int main(void) {
    VIDEO_Init();
    WPAD_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb[fb]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
    fb ^= 1;

    void *fifo = memalign(32, FIFO_SIZE);
    memset(fifo, 0, FIFO_SIZE);
    GX_Init(fifo, FIFO_SIZE);
    GXColor bg = {150, 200, 245, 255};
    GX_SetCopyClear(bg, 0x00ffffff);
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    f32 yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    u32 xfbH = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, xfbH);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
    if (rmode->aa) GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    else GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(xfb[fb], GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    /* zvuk */
    ASND_Init();
    int menu_m[32] = {72, 76, 79, 76, 72, 76, 79, 0, 77, 81, 84, 81, 77, 81, 84, 0,
                      79, 83, 86, 83, 79, 83, 86, 0, 84, 0, 79, 0, 76, 0, 72, 0};
    int game_m[32] = {67, 0, 72, 0, 76, 0, 72, 0, 69, 0, 72, 0, 77, 0, 72, 0,
                      67, 0, 71, 0, 74, 0, 71, 0, 72, 0, 76, 0, 79, 0, 0, 0};
    int roots[4] = {48, 53, 55, 48}, bass[32];
    for (int i = 0; i < 32; i++) bass[i] = roots[i / 8];
    int mb, gb, bb, db, zb, sb, jb;
    s16 *menu_s = make_song(menu_m, bass, 32, 190, &mb);
    s16 *game_s = make_song(game_m, bass, 32, 300, &gb);
    float f1[2] = {1568, 2093}; int m1[2] = {90, 380};
    float f2[1] = {1760};       int m2[1] = {160};
    float f3[2] = {140, 110};   int m3[2] = {120, 160};
    float f4[2] = {1047, 784};  int m4[2] = {80, 200};
    float f5[4] = {523, 659, 784, 1047}; int m5[4] = {90, 90, 90, 320};
    s16 *buy_s = make_sfx(f1, m1, 2, 0, 0.25f, &bb);
    s16 *ding_s = make_sfx(f2, m2, 1, 1, 0.3f, &db);
    s16 *buzz_s = make_sfx(f3, m3, 2, 0, 0.25f, &zb);
    s16 *sell_s = make_sfx(f4, m4, 2, 1, 0.3f, &sb);
    s16 *jing_s = make_sfx(f5, m5, 4, 0, 0.25f, &jb);
    ASND_Pause(0);

    srand((unsigned)gettime());
    s16 *cur = menu_s;
    int curb = mb;
    int state = 0, money = 120, earned = 0, served = 0, sel = 0, cx = 3, cz = 3;
    int won = 0, msgt = 0, spt = 0, frame = 0;
    char msg[48] = "", buf[64];

    while (1) {
        WPAD_ScanPads();
        u32 d = WPAD_ButtonsDown(0);
        if (d & WPAD_BUTTON_HOME) exit(0);
        if (ASND_StatusVoice(0) == SND_UNUSED) play(0, cur, curb, 80);
        GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);

        if (state == 0) {
            if (d & WPAD_BUTTON_A) {
                state = 1;
                ASND_StopVoice(0);
                cur = game_s;
                curb = gb;
                play(1, jing_s, jb, 200);
            }
            float a = frame * 0.012f;
            begin3d(sinf(a) * 9, 4.2f, cosf(a) * 9, 0, 0.8f, 0);
            draw_menu_scene();
            begin2d();
            label(20, 30, 10, "TESCO LAND", 255, 255, 255);
            if ((frame / 30) % 2 == 0) label(128, 400, 4, "PRESS A TO START", 255, 230, 60);
            label(104, 450, 2, "FAN GAME - NOT AFFILIATED WITH TESCO", 255, 255, 255);
        } else {
            if (d & WPAD_BUTTON_LEFT) { if (cx > 0) cx--; }
            if (d & WPAD_BUTTON_RIGHT) { if (cx < N - 1) cx++; }
            if (d & WPAD_BUTTON_UP) { if (cz > 0) cz--; }
            if (d & WPAD_BUTTON_DOWN) { if (cz < N - 1) cz++; }
            if (d & (WPAD_BUTTON_PLUS | WPAD_BUTTON_2)) sel = (sel + 1) % 4;
            if (d & (WPAD_BUTTON_MINUS | WPAD_BUTTON_1)) sel = (sel + 3) % 4;
            if (d & WPAD_BUTTON_A) {
                if (grid[cx][cz]) {
                    strcpy(msg, "TILE IS TAKEN"); msgt = 90; play(1, buzz_s, zb, 200);
                } else if (money < COST[sel]) {
                    strcpy(msg, "NOT ENOUGH MONEY!"); msgt = 90; play(1, buzz_s, zb, 200);
                } else {
                    money -= COST[sel]; grid[cx][cz] = sel + 1; play(1, buy_s, bb, 220);
                }
            }
            if ((d & WPAD_BUTTON_B) && grid[cx][cz]) {
                money += COST[grid[cx][cz] - 1] / 2;
                grid[cx][cz] = 0;
                play(1, sell_s, sb, 220);
            }

            int bc = built_count();
            if (bc > 0) {
                int iv = 100 - 6 * bc;
                if (iv < 20) iv = 20;
                if (++spt >= iv) { spt = 0; spawn(); }
            }
            for (int i = 0; i < MAXC; i++) {
                if (!cu[i].on) continue;
                float tx = cu[i].state == 0 ? cu[i].gx - 3.5f : 0.0f;
                float tz = cu[i].state == 0 ? cu[i].gz - 3.5f : 5.2f;
                float dx = tx - cu[i].x, dz = tz - cu[i].z, dist = sqrtf(dx * dx + dz * dz);
                if (dist < 0.1f) {
                    if (cu[i].state == 0) {
                        int t = grid[cu[i].gx][cu[i].gz];
                        if (t) {
                            money += PAY[t - 1]; earned += PAY[t - 1]; served++;
                            play(2, ding_s, db, 160);
                        }
                        cu[i].state = 1;
                    } else cu[i].on = 0;
                } else {
                    cu[i].x += 0.045f * dx / dist;
                    cu[i].z += 0.045f * dz / dist;
                }
            }
            if (!won && earned >= GOAL) {
                won = 1; strcpy(msg, "YOU WIN! TESCO LAND IS A SUCCESS"); msgt = 400;
            }

            begin3d(0, 9.0f, 10.5f, 0, 0, 0.3f);
            draw_world(frame, cx, cz, sel, money);
            begin2d();
            rect2(0, 0, 640, 68, 0, 60, 120);
            snprintf(buf, sizeof buf, "MONEY: #%d", money);
            label(10, 8, 4, buf, 255, 255, 255);
            snprintf(buf, sizeof buf, "SERVED: %d", served);
            label(380, 8, 3, buf, 255, 230, 60);
            snprintf(buf, sizeof buf, "GOAL: EARN #%d   EARNED: #%d", GOAL, earned);
            label(10, 46, 2, buf, 200, 220, 255);
            rect2(0, 420, 640, 60, 0, 60, 120);
            snprintf(buf, sizeof buf, "BUILD: %s  #%d", NAMES[sel], COST[sel]);
            label(10, 428, 3, buf, 255, 255, 255);
            label(10, 456, 2, "D-PAD MOVE  A BUILD  B SELL  +/- ITEM  HOME EXIT", 200, 220, 255);
            if (msgt > 0) { msgt--; label(30, 380, 3, msg, 255, 230, 60); }
        }

        GX_DrawDone()
