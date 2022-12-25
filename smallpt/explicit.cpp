#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2009
#include <stdlib.h> // Make : g++ -O3 -fopenmp explicit.cpp -o explicit
#include <stdio.h>  // Remove "-fopenmp" for g++ version < 4.2
#include <x86intrin.h>
#include <assert.h>
#ifdef _WIN32
// implement erand48() for Windows
#include <random>
std::default_random_engine generator;
std::uniform_real_distribution<float> distr(0.0, 1.0);
float erand48(unsigned short *X)
{
    return distr(generator);
}
#endif
template <typename T>
struct Vec_
{                   // Usage: time ./explicit 16 && xv image.ppm
    T x, y, z;      // position, also color (r,g,b)
    Vec_(T x_ = 0, T y_ = 0, T z_ = 0)
    {
        x = x_;
        y = y_;
        z = z_;
    }
    template <typename U>
    Vec_(const Vec_<U> &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }
    Vec_ operator+(const Vec_ &b) const { return Vec_(x + b.x, y + b.y, z + b.z); }
    Vec_ operator-(const Vec_ &b) const { return Vec_(x - b.x, y - b.y, z - b.z); }
    Vec_ operator*(T b) const { return Vec_(x * b, y * b, z * b); }
    Vec_ operator/(T b) const { return Vec_(x / b, y / b, z / b); }
    Vec_ mult(const Vec_ &b) const { return Vec_(x * b.x, y * b.y, z * b.z); }
    Vec_ &norm() { return *this = *this / sqrt(x * x + y * y + z * z); }
    T dot(const Vec_ &b) const { return x * b.x + y * b.y + z * b.z; } // cross:
    Vec_ operator%(Vec_ &b) { return Vec_(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
};
typedef Vec_<float> Vec;
typedef Vec_<double> Vecd;
typedef __attribute((vector_size(32))) int v4si;
void debug(int line, int depth, __m256 x)
{
    v4si p = (v4si)x;
    for (int i = 0; i < 8; i++)
        printf("%d ", p[i]);
    printf("depth = %d, line = %d\n", depth, line);
}
const float eps = 1e-6;
// convert to a AVX vector
struct Vec_avx
{
    __m256 x, y, z;
    Vec_avx(__m256 x_ = _mm256_setzero_ps(), __m256 y_ = _mm256_setzero_ps(), __m256 z_ = _mm256_setzero_ps()) : x(x_), y(y_), z(z_) {}
    Vec_avx(const Vec &v) : x(_mm256_set1_ps(v.x)), y(_mm256_set1_ps(v.y)), z(_mm256_set1_ps(v.z)) {}
    Vec_avx operator+(const Vec_avx &b) const { return Vec_avx(x + b.x, y + b.y, z + b.z); }
    Vec_avx operator-(const Vec_avx &b) const { return Vec_avx(x - b.x, y - b.y, z - b.z); }
    Vec_avx operator*(__m256 b) const { return Vec_avx(x * b, y * b, z * b); }
    Vec_avx operator/(__m256 b) const { return Vec_avx(x / b, y / b, z / b); }
    Vec_avx mult(const Vec_avx &b) const { return Vec_avx(x * b.x, y * b.y, z * b.z); }
    Vec_avx &norm()
    {
        __m256 dist = _mm256_sqrt_ps(x * x + y * y + z * z);
        return *this = *this / dist;
    }
    __m256 dot(const Vec_avx &b) const { return x * b.x + y * b.y + z * b.z; } // cross
    Vec_avx operator%(Vec_avx &b) { return Vec_avx(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
    Vec_avx blend(const Vec_avx &b, __m256 mask) const
    {
        return Vec_avx(_mm256_blendv_ps(x, b.x, mask), _mm256_blendv_ps(y, b.y, mask), _mm256_blendv_ps(z, b.z, mask));
    }
};
struct Vec_avxd
{
    __m256d x, y, z;
    Vec_avxd(__m256d x_ = _mm256_setzero_pd(), __m256d y_ = _mm256_setzero_pd(), __m256d z_ = _mm256_setzero_pd()) : x(x_), y(y_), z(z_) {}
    Vec_avxd(const Vecd &v) : x(_mm256_set1_pd(v.x)), y(_mm256_set1_pd(v.y)), z(_mm256_set1_pd(v.z)) {}
    Vec_avxd(const Vec_avx &v, int start) : x(_mm256_cvtps_pd(_mm256_extractf128_ps(v.x, start))),
                                            y(_mm256_cvtps_pd(_mm256_extractf128_ps(v.y, start))),
                                            z(_mm256_cvtps_pd(_mm256_extractf128_ps(v.z, start))) {}
    Vec_avxd operator+(const Vec_avxd &b) const { return Vec_avxd(x + b.x, y + b.y, z + b.z); }
    Vec_avxd operator-(const Vec_avxd &b) const { return Vec_avxd(x - b.x, y - b.y, z - b.z); }
    Vec_avxd operator*(__m256d b) const { return Vec_avxd(x * b, y * b, z * b); }
    Vec_avxd operator/(__m256d b) const { return Vec_avxd(x / b, y / b, z / b); }
    Vec_avxd mult(const Vec_avxd &b) const { return Vec_avxd(x * b.x, y * b.y, z * b.z); }
    Vec_avxd &norm()
    {
        __m256d dist = _mm256_sqrt_pd(x * x + y * y + z * z);
        return *this = *this / dist;
    }
    __m256d dot(const Vec_avxd &b) const { return x * b.x + y * b.y + z * b.z; } // cross
    Vec_avxd operator%(Vec_avxd &b) { return Vec_avxd(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
    Vec_avxd blend(const Vec_avxd &b, __m256d mask) const
    {
        return Vec_avxd(_mm256_blendv_pd(x, b.x, mask), _mm256_blendv_pd(y, b.y, mask), _mm256_blendv_pd(z, b.z, mask));
    }
};
struct Ray
{
    Vec o, d;
    Ray(Vec o_, Vec d_) : o(o_), d(d_) {}
};
// using AVX
struct Ray_avx
{
    Vec_avx o, d;
    Ray_avx(Vec_avx o_, Vec_avx d_) : o(o_), d(d_) {}
};
struct Ray_avxd
{
    Vec_avxd o, d;
    Ray_avxd(Vec_avxd o_, Vec_avxd d_) : o(o_), d(d_) {}
};
enum Refl_t
{
    DIFF,
    SPEC,
    REFR
}; // material types, used in radiance()
struct Sphere
{
    float rad;   // radius
    Vec p, e, c; // position, emission, color
    Refl_t refl; // reflection type (DIFFuse, SPECular, REFRactive)
    Sphere(float rad_, Vec p_, Vec e_, Vec c_, Refl_t refl_) : rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}
    float intersect(const Ray &r) const
    {                     // returns distance, 0 if nohit
        Vecd op = Vecd(p) - r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        double t, eps = 1e-4, b = op.dot(r.d), det = b * b - op.dot(op) + rad * rad;
        if (det < 0)
            return 0;
        else
            det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
    }
    __m256 intersect_avx(const Ray_avx &r) const
    {
        __m256 ret;
        for (int i = 0; i < 2; i++)
        {
            Vec_avxd od(r.o, i), dd(r.d, i);
            Vec_avxd op = Vec_avxd(p) - od; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
            __m256d t, eps = _mm256_set1_pd(1e-4), b = op.dot(dd), det = b * b - op.dot(op) + _mm256_set1_pd(rad * rad);
            __m256d mask = _mm256_cmp_pd(det, _mm256_setzero_pd(), _CMP_LT_OQ);
            det = _mm256_sqrt_pd(det);
            __m256d mask2 = _mm256_cmp_pd(b - det, eps, _CMP_GT_OQ);
            __m256d mask3 = _mm256_cmp_pd(b + det, eps, _CMP_GT_OQ);
            __m256d ans = _mm256_blendv_pd(_mm256_blendv_pd(_mm256_setzero_pd(), b + det, mask3), b - det, mask2);
            ans = _mm256_blendv_pd(ans, _mm256_setzero_pd(), mask);
            ret = _mm256_insertf128_ps(ret, _mm256_cvtpd_ps(ans), i);
        }
        return ret;
    }
};
Sphere spheres[] = {
    // Scene: radius, position, emission, color, material
    Sphere(1e5, Vec(1e5 + 1, 40.8, 81.6), Vec(), Vec(.75, .25, .25), DIFF),   // Left
    Sphere(1e5, Vec(-1e5 + 99, 40.8, 81.6), Vec(), Vec(.25, .25, .75), DIFF), // Rght
    Sphere(1e5, Vec(50, 40.8, 1e5), Vec(), Vec(.75, .75, .75), DIFF),         // Back
    Sphere(1e5, Vec(50, 40.8, -1e5 + 170), Vec(), Vec(), DIFF),               // Frnt
    Sphere(1e5, Vec(50, 1e5, 81.6), Vec(), Vec(.75, .75, .75), DIFF),         // Botm
    Sphere(1e5, Vec(50, -1e5 + 81.6, 81.6), Vec(), Vec(.75, .75, .75), DIFF), // Top
    // Sphere(16.5, Vec(27, 16.5, 47), Vec(), Vec(1, 1, 1) * .999, SPEC),        // Mirr
    // Sphere(16.5, Vec(73, 16.5, 78), Vec(), Vec(1, 1, 1) * .999, REFR),        // Glas
    Sphere(1.5, Vec(50, 81.6 - 16.5, 81.6), Vec(4, 4, 4) * 100, Vec(), DIFF), // Lite
};
int numSpheres = sizeof(spheres) / sizeof(Sphere);
inline float clamp(float x) { return x < 0 ? 0 : x > 1 ? 1
                                                       : x; }
inline int toInt(float x) { return int(pow(clamp(x), 1 / 2.2) * 255 + .5); }
inline bool intersect(const Ray &r, float &t, int &id)
{
    float n = sizeof(spheres) / sizeof(Sphere), d, inf = t = 1e20;
    for (int i = int(n); i--;)
        if ((d = spheres[i].intersect(r)) && d < t)
        {
            t = d;
            id = i;
        }
    return t < inf;
}
// AVX version
inline __m256 intersect_avx(const Ray_avx &r, __m256 &t, __m256 &id)
{
    float inf = 1e20;
    t = _mm256_set1_ps(inf);
    for (int i = 0; i < numSpheres; i++)
    {
        __m256 d = spheres[i].intersect_avx(r);
        __m256 mask = _mm256_cmp_ps(d, _mm256_set1_ps(0), _CMP_GT_OQ);
        __m256 mask2 = _mm256_cmp_ps(d, t, _CMP_LT_OQ);
        mask = _mm256_and_ps(mask, mask2);
        t = _mm256_blendv_ps(t, d, mask);
        id = _mm256_blendv_ps(id, _mm256_set1_ps(i), mask);
    }
    return _mm256_cmp_ps(t, _mm256_set1_ps(inf), _CMP_LT_OQ);
}
Vec radiance(const Ray &r, int depth, unsigned short *Xi, int E = 1)
{
    float t;    // distance to intersection
    int id = 0; // id of intersected object
    if (!intersect(r, t, id))
        return Vec();                // if miss, return black
    const Sphere &obj = spheres[id]; // the hit object
    Vec x = r.o + r.d * t, n = (x - obj.p).norm(), nl = n.dot(r.d) < 0 ? n : n * -1, f = obj.c;
    float p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y
                                                       : f.z; // max refl
    if (++depth > 5 || !p)
        if (erand48(Xi) < p)
            f = f * (1 / p);
        else
            return obj.e * E;
    if (obj.refl == DIFF)
    { // Ideal DIFFUSE reflection
        float r1 = 2 * M_PI * erand48(Xi), r2 = erand48(Xi), r2s = sqrt(r2);
        Vec w = nl, u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm(), v = w % u;
        Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm();

        // Loop over any lights
        Vec e;
        for (int i = 0; i < numSpheres; i++)
        {
            const Sphere &s = spheres[i];
            if (s.e.x <= 0 && s.e.y <= 0 && s.e.z <= 0)
                continue; // skip non-lights

            Vec sw = s.p - x, su = ((fabs(sw.x) > .1 ? Vec(0, 1) : Vec(1)) % sw).norm(), sv = sw % su;
            float cos_a_max = sqrt(1 - s.rad * s.rad / (x - s.p).dot(x - s.p));
            float eps1 = erand48(Xi), eps2 = erand48(Xi);
            float cos_a = 1 - eps1 + eps1 * cos_a_max;
            float sin_a = sqrt(1 - cos_a * cos_a);
            float phi = 2 * M_PI * eps2;
            Vec l = su * cos(phi) * sin_a + sv * sin(phi) * sin_a + sw * cos_a;
            l.norm();
            if (intersect(Ray(x, l), t, id) && id == i)
            { // shadow ray
                float omega = 2 * M_PI * (1 - cos_a_max);
                e = e + f.mult(s.e * l.dot(nl) * omega) * M_1_PI; // 1/pi for brdf
            }
        }

        return obj.e * E + e + f.mult(radiance(Ray(x, d), depth, Xi, 0));
    }
    else if (obj.refl == SPEC) // Ideal SPECULAR reflection
        return obj.e + f.mult(radiance(Ray(x, r.d - n * 2 * n.dot(r.d)), depth, Xi));
    Ray reflRay(x, r.d - n * 2 * n.dot(r.d)); // Ideal dielectric REFRACTION
    bool into = n.dot(nl) > 0;                // Ray from outside going in?
    float nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.d.dot(nl), cos2t;
    if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) // Total internal reflection
        return obj.e + f.mult(radiance(reflRay, depth, Xi));
    Vec tdir = (r.d * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).norm();
    float a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : tdir.dot(n));
    float Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
    return obj.e + f.mult(depth > 2 ? (erand48(Xi) < P ? // Russian roulette
                                           radiance(reflRay, depth, Xi) * RP
                                                       : radiance(Ray(x, tdir), depth, Xi) * TP)
                                    : radiance(reflRay, depth, Xi) * Re + radiance(Ray(x, tdir), depth, Xi) * Tr);
}
struct Sphere_avx
{
    __m256 rad;      // radius
    Vec_avx p, e, c; // position, emission, color
    __m256 refl;     // reflection type (DIFFuse, SPECular, REFRactive)
    Sphere_avx(__m256 id)
    {
        for (int i = 0; i < 8; i++)
        {
            int idx = (int)id[i];
            rad[i] = spheres[idx].rad;
            p.x[i] = spheres[idx].p.x;
            p.y[i] = spheres[idx].p.y;
            p.z[i] = spheres[idx].p.z;
            e.x[i] = spheres[idx].e.x;
            e.y[i] = spheres[idx].e.y;
            e.z[i] = spheres[idx].e.z;
            c.x[i] = spheres[idx].c.x;
            c.y[i] = spheres[idx].c.y;
            c.z[i] = spheres[idx].c.z;
            refl[i] = spheres[idx].refl;
        }
    }
};
inline __m256 erand48v(unsigned short *Xi)
{
    __m256 ans;
    for (int i = 0; i < 8; i++)
        ans[i] = erand48(Xi);
    return ans;
}
inline void erand48tri(unsigned short *Xi, __m256 &c, __m256 &s)
{
    c = erand48v(Xi) * 2 - 1;
    __m256 sign = _mm256_cmp_ps(erand48v(Xi), _mm256_set1_ps(0.5), _CMP_LT_OQ);
    s = _mm256_sqrt_ps(1 - c * c);
    s = _mm256_blendv_ps(s, -s, sign);
}
#define CHKMASK(mask) debug(__LINE__, depth, mask)
// AVX version
/*
const int MAX_DEPTH = 100;
struct saveinfo
{
    Vec_avx pre, f, ans;
    __m256 mask;
} s[MAX_DEPTH];
Vec_avx radiance_avx(Ray_avx r, __m256 mask, saveinfo s[], unsigned short *Xi, int E = 1)
{
    int depth = 0;
    for (;;)
    {
        Vec_avx ans;
        if (_mm256_testz_ps(mask, mask))
            break;
        __m256 t;                      // distance to intersection
        __m256 id = _mm256_set1_ps(0); // id of intersected object
        mask = _mm256_and_ps(mask, intersect_avx(r, t, id));
        // for mask[i] == 0, the corresponding value in ans is not updated
        Sphere_avx obj(id); // the hit objects
        Vec_avx x = r.o + r.d * t;
        Vec_avx n = (x - obj.p).norm();
        Vec_avx nl = n.blend(n * _mm256_set1_ps(-1), _mm256_cmp_ps(r.d.dot(n), _mm256_setzero_ps(), _CMP_LT_OQ));
        Vec_avx f = obj.c;
        __m256 p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y
                                                            : f.z; // max refl
        __m256 mask2 = _mm256_and_ps(mask, _mm256_or_ps((__m256)_mm256_set1_epi32(++depth > 5 ? 0xffffffff : 0), _mm256_cmp_ps(p, _mm256_set1_ps(0), _CMP_EQ_OQ)));
        __m256 mask3 = _mm256_cmp_ps(erand48v(Xi), p, _CMP_LT_OQ);
        f = f.blend(f * (1.0 / p), _mm256_and_ps(mask2, mask3));
        ans = ans.blend(obj.e * _mm256_set1_ps(E), _mm256_andnot_ps(mask3, mask2));
        mask = _mm256_andnot_ps(_mm256_andnot_ps(mask3, mask2), mask);
        __m256 mask4 = _mm256_cmp_ps(obj.refl, _mm256_set1_ps(0), _CMP_EQ_OQ); // Ideal DIFFUSE reflection
        // __m256 r1 = erand48v(Xi) * 2 * M_PI;
        __m256 r2 = erand48v(Xi), r2s = _mm256_sqrt_ps(r2);
        Vec_avx w = nl;
        __m256 mask5 = _mm256_or_ps(_mm256_cmp_ps(w.x, _mm256_set1_ps(0.1), _CMP_GT_OQ), _mm256_cmp_ps(w.x, _mm256_set1_ps(-0.1), _CMP_LT_OQ));
        Vec_avx u = (Vec_avx(_mm256_set1_ps(1)).blend(Vec_avx(_mm256_set1_ps(0), _mm256_set1_ps(1)), mask5) % w).norm();
        Vec_avx v = w % u;
        __m256 r1c, r1s;
        erand48tri(Xi, r1c, r1s);
        Vec_avx d = (u * r1c * r2s + v * r1s * r2s + w * _mm256_sqrt_ps(1 - r2)).norm();

        // Loop over any lights
        Vec_avx e;
        for (int i = 0; i < numSpheres; i++)
        {
            const Sphere &s = spheres[i];
            if (s.e.x <= 0 && s.e.y <= 0 && s.e.z <= 0)
                continue; // skip non-lights

            Vec_avx sw = Vec_avx(s.p) - x;
            __m256 mask6 = _mm256_or_ps(_mm256_cmp_ps(sw.x, _mm256_set1_ps(0.1), _CMP_GT_OQ), _mm256_cmp_ps(sw.x, _mm256_set1_ps(-0.1), _CMP_LT_OQ));
            Vec_avx su = (Vec_avx(_mm256_set1_ps(1)).blend(Vec_avx(_mm256_set1_ps(0), _mm256_set1_ps(1)), mask6) % sw).norm();
            Vec_avx sv = sw % su;
            __m256 cos_a_max = _mm256_sqrt_ps(1 - s.rad * s.rad / (x - Vec_avx(s.p)).dot(x - Vec_avx(s.p)));
            __m256 eps1 = erand48v(Xi);
            __m256 cos_a = 1 - eps1 + eps1 * cos_a_max;
            __m256 sin_a = _mm256_sqrt_ps(1 - cos_a * cos_a);
            // __m256 phi = 2 * M_PI * erand48v(Xi);
            __m256 phic, phis;
            erand48tri(Xi, phic, phis);
            Vec_avx l = su * phic * sin_a + sv * phis * sin_a + sw * cos_a;
            l.norm();
            __m256 t1, id1;
            __m256 isect = intersect_avx(Ray_avx(x, l), t1, id1);
            __m256 mask7 = _mm256_and_ps(isect, _mm256_cmp_ps(id1, _mm256_set1_ps(i), _CMP_EQ_OQ));
            __m256 omega = 2 * (float)M_PI * (1 - cos_a_max);
            e = e.blend(e + f.mult(Vec_avx(s.e) * l.dot(nl) * omega) * _mm256_set1_ps(M_1_PI), mask7);
        }
        // ans = ans.blend(obj.e * _mm256_set1_ps(E) + e + f.mult(radiance_avx(Ray_avx(x, d), _mm256_and_ps(mask4, mask), depth, Xi)), _mm256_and_ps(mask4, mask));
        mask = _mm256_and_ps(mask4, mask);
        s[depth].ans = ans;
        s[depth].mask = mask;
        s[depth].pre = obj.e * _mm256_set1_ps(E) + e;
        s[depth].f = f;
        r = Ray_avx(x, d);
    }
    Vec_avx ans;
    for (; depth; depth--)
    {
        ans = s[depth].ans.blend(s[depth].pre + s[depth].f.mult(ans), s[depth].mask);
    }
    return ans;
}
*/
Vec_avx radiance_avx(const Ray_avx &r, __m256 mask, int depth, unsigned short *Xi, int E = 1)
{
    Vec_avx ans;
    if (_mm256_testz_ps(mask, mask))
        return ans;
    __m256 t;  // distance to intersection
    __m256 id = _mm256_set1_ps(0); // id of intersected object
    mask = _mm256_and_ps(mask, intersect_avx(r, t, id));
    // for mask[i] == 0, the corresponding value in ans is not updated
    Sphere_avx obj(id); // the hit objects
    Vec_avx x = r.o + r.d * t;
    Vec_avx n = (x - obj.p).norm();
    Vec_avx nl = n.blend(n * _mm256_set1_ps(-1), _mm256_cmp_ps(r.d.dot(n), _mm256_setzero_ps(), _CMP_GE_OQ));
    Vec_avx f = obj.c;
    __m256 p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y
                                                        : f.z; // max refl
    __m256 mask2 = _mm256_and_ps(mask, _mm256_or_ps((__m256)_mm256_set1_epi32(++depth > 5 ? 0xffffffff : 0), _mm256_cmp_ps(p, _mm256_set1_ps(0), _CMP_EQ_OQ)));
    __m256 mask3 = _mm256_cmp_ps(erand48v(Xi), p, _CMP_LT_OQ);
    f = f.blend(f * (1.0 / p), _mm256_and_ps(mask2, mask3));
    ans = ans.blend(obj.e * _mm256_set1_ps(E), _mm256_andnot_ps(mask3, mask2));
    mask = _mm256_andnot_ps(_mm256_andnot_ps(mask3, mask2), mask);
    __m256 mask4 = _mm256_cmp_ps(obj.refl, _mm256_set1_ps(0), _CMP_EQ_OQ); // Ideal DIFFUSE reflection
    // __m256 r1 = erand48v(Xi) * 2 * M_PI;
    __m256 r2 = erand48v(Xi), r2s = _mm256_sqrt_ps(r2);
    Vec_avx w = nl;
    __m256 mask5 = _mm256_or_ps(_mm256_cmp_ps(w.x, _mm256_set1_ps(0.1), _CMP_GT_OQ), _mm256_cmp_ps(w.x, _mm256_set1_ps(-0.1), _CMP_LT_OQ));
    Vec_avx u = (Vec_avx(_mm256_set1_ps(1)).blend(Vec_avx(_mm256_set1_ps(0), _mm256_set1_ps(1)), mask5) % w).norm();
    Vec_avx v = w % u;
    __m256 r1c, r1s;
    erand48tri(Xi, r1c, r1s);
    Vec_avx d = (u * r1c * r2s + v * r1s * r2s + w * _mm256_sqrt_ps(1 - r2)).norm();

    // Loop over any lights
    Vec_avx e;
    __m256 epss[10], phicc[10], phiss[10];
    for (int i = 0; i < numSpheres; i++)
    {
        const Sphere &s = spheres[i];
        if (s.e.x <= 0 && s.e.y <= 0 && s.e.z <= 0)
            continue; // skip non-lights

        Vec_avx sw = Vec_avx(s.p) - x;
        __m256 mask6 = _mm256_or_ps(_mm256_cmp_ps(sw.x, _mm256_set1_ps(0.1), _CMP_GT_OQ), _mm256_cmp_ps(sw.x, _mm256_set1_ps(-0.1), _CMP_LT_OQ));
        Vec_avx su = (Vec_avx(_mm256_set1_ps(1)).blend(Vec_avx(_mm256_set1_ps(0), _mm256_set1_ps(1)), mask6) % sw).norm();
        Vec_avx sv = sw % su;
        __m256 cos_a_max = _mm256_sqrt_ps(1 - s.rad * s.rad / (x - Vec_avx(s.p)).dot(x - Vec_avx(s.p)));
        __m256 eps1 = erand48v(Xi);
        epss[i] = eps1;
        __m256 cos_a = 1 - eps1 + eps1 * cos_a_max;
        __m256 sin_a = _mm256_sqrt_ps(1 - cos_a * cos_a);
        // __m256 phi = 2 * M_PI * erand48v(Xi);
        __m256 phic, phis;
        erand48tri(Xi, phic, phis);
        phicc[i] = phic;
        phiss[i] = phis;
        Vec_avx l = su * phic * sin_a + sv * phis * sin_a + sw * cos_a;
        l.norm();
        __m256 t1, id1;
        __m256 isect = intersect_avx(Ray_avx(x, l), t1, id1);
        __m256 mask7 = _mm256_and_ps(isect, _mm256_cmp_ps(id1, _mm256_set1_ps(i), _CMP_EQ_OQ));
        __m256 omega = 2 * (float)M_PI * (1 - cos_a_max);
        e = e.blend(e + f.mult(Vec_avx(s.e) * l.dot(nl) * omega) * _mm256_set1_ps(M_1_PI), mask7);
    }

    /* compare answer with reference
    for (int i = 0; i < 8; i++)
    {
        float t_ref;
        int id_ref;
        Ray r_ref(Vec(r.o.x[i], r.o.y[i], r.o.z[i]), Vec(r.d.x[i], r.d.y[i], r.d.z[i]));
        if (!intersect(r_ref, t_ref, id_ref))
        {
            if (mask[i])
            {
                if (ans.x[i] != 0 || ans.y[i] != 0 || ans.z[i] != 0)
                {
                    printf("i=%d, ans=(%f,%f,%f)\n", i, ans.x[i], ans.y[i], ans.z[i]);
                    exit(1);
                }
            }
            continue;
        }
        const Sphere &obj_ref = spheres[id_ref];
        Vec x_ref = r_ref.o + r_ref.d * t_ref;
        Vec n_ref = (x_ref - obj_ref.p).norm();
        Vec nl_ref = n_ref.dot(r_ref.d) < 0 ? n_ref : n_ref * -1;
        Vec f_ref = obj_ref.c;
        float p_ref = f_ref.x > f_ref.y && f_ref.x > f_ref.z ? f_ref.x : f_ref.y > f_ref.z ? f_ref.y
                                                                                           : f_ref.z; // max refl
        if (depth > 5 || !p_ref)
            if (rnd[i] < p_ref)
                f_ref = f_ref * (1 / p_ref);
            else
            {
                Vec ans_ref = obj_ref.e * E;
                if (mask[i])
                {
                    if (fabs((ans_ref.x - ans.x[i]) / ans_ref.x) > eps || fabs((ans_ref.y - ans.y[i]) / ans_ref.y) > eps || fabs((ans_ref.z - ans.z[i]) / ans_ref.z) > eps)
                    {
                        printf("i=%d, ans=(%f,%f,%f), ans_ref=(%f,%f,%f)\n", i, ans.x[i], ans.y[i], ans.z[i], ans_ref.x, ans_ref.y, ans_ref.z);
                        printf("p_ref=%f, p=%f\n", p_ref, p[i]);
                        exit(1);
                    }
                }
            }
        if (obj_ref.refl == DIFF)
        {
            Vec w_ref = nl_ref;
            Vec u_ref = ((fabs(w_ref.x) > .1 ? Vec(0, 1) : Vec(1)) % w_ref).norm();
            Vec v_ref = w_ref % u_ref;
            Vec d_ref = (u_ref * r1c[i] * r2s[i] + v_ref * r1s[i] * r2s[i] + w_ref * sqrt(1 - r2[i])).norm();
            Vec e_ref;
            for (int j = 0; j < numSpheres; j++)
            {
                const Sphere &s_ref = spheres[j];
                if (s_ref.e.x <= 0 && s_ref.e.y <= 0 && s_ref.e.z <= 0)
                    continue; // skip non-lights

                Vec sw_ref = s_ref.p - x_ref;
                Vec su_ref = ((fabs(sw_ref.x) > .1 ? Vec(0, 1) : Vec(1)) % sw_ref).norm();
                Vec sv_ref = sw_ref % su_ref;
                float cos_a_max_ref = sqrt(1 - s_ref.rad * s_ref.rad / (x_ref - s_ref.p).dot(x_ref - s_ref.p));
                float eps1_ref = epss[j][i];
                float cos_a_ref = 1 - eps1_ref + eps1_ref * cos_a_max_ref;
                float sin_a_ref = sqrt(1 - cos_a_ref * cos_a_ref);
                Vec l = su_ref * phicc[j][i] * sin_a_ref + sv_ref * phiss[j][i] * sin_a_ref + sw_ref * cos_a_ref;
                l.norm();
                float t_ref;
                int id_ref;
                if (intersect(Ray(x_ref, l), t_ref, id_ref) && id_ref == j)
                {
                    float omega_ref = 2 * (float)M_PI * (1 - cos_a_max_ref);
                    e_ref = e_ref + f_ref.mult(s_ref.e * l.dot(w_ref) * omega_ref) * M_1_PI;
                }
            }
            if (mask[i])
            {
                if (fabs((e_ref.x - e.x[i]) / e_ref.x) > eps || fabs((e_ref.y - e.y[i]) / e_ref.y) > eps || fabs((e_ref.z - e.z[i]) / e_ref.z) > eps)
                {
                    printf("i=%d, e=(%f,%f,%f), e_ref=(%f,%f,%f)\n", i, e.x[i], e.y[i], e.z[i], e_ref.x, e_ref.y, e_ref.z);
                    printf("obj.p=(%f,%f,%f), obj_ref.p=(%f,%f,%f)\n", obj.p.x[i], obj.p.y[i], obj.p.z[i], obj_ref.p.x, obj_ref.p.y, obj_ref.p.z);
                    printf("id=%f, id_ref=%d\n", id[i], id_ref);
                    printf("t=%f, t_ref=%f\n", t[i], t_ref);
                    exit(1);
                }
            }
        }
    }
    // puts("done");
    */

    ans = ans.blend(obj.e * _mm256_set1_ps(E) + e + f.mult(radiance_avx(Ray_avx(x, d), _mm256_and_ps(mask4, mask), depth, Xi)), _mm256_and_ps(mask4, mask));
    __m256 mask6 = _mm256_cmp_ps(obj.refl, _mm256_set1_ps(1), _CMP_EQ_OQ); // Ideal SPECULAR reflection
    Ray_avx reflRay(x, r.d - n * _mm256_set1_ps(2) * n.dot(r.d));
    ans = ans.blend(obj.e + f.mult(radiance_avx(reflRay, _mm256_and_ps(mask6, mask), depth, Xi)), _mm256_and_ps(mask6, mask));
    __m256 mask7 = _mm256_cmp_ps(obj.refl, _mm256_set1_ps(2), _CMP_EQ_OQ); // Ideal dielectric REFRACTION
    mask = _mm256_and_ps(mask7, mask);
    __m256 into = _mm256_cmp_ps(n.dot(nl), _mm256_set1_ps(0), _CMP_GT_OQ); // Ray from outside going in?
    __m256 nc = _mm256_set1_ps(1);
    __m256 nt = _mm256_set1_ps(1.5);
    __m256 nnt = _mm256_blendv_ps(nt / nc, nc / nt, into);
    __m256 ddn = r.d.dot(nl);
    __m256 cos2t = 1 - nnt * nnt * (1 - ddn * ddn);
    __m256 mask8 = _mm256_cmp_ps(cos2t, _mm256_set1_ps(0), _CMP_LT_OQ);
    ans.blend(obj.e + f.mult(radiance_avx(reflRay, _mm256_and_ps(mask8, mask), depth, Xi)), _mm256_and_ps(mask8, mask));
    mask = _mm256_andnot_ps(mask8, mask);
    Vec_avx tdir = (r.d * nnt - n * (_mm256_blendv_ps(_mm256_set1_ps(-1), _mm256_set1_ps(1), into) * (ddn * nnt + _mm256_sqrt_ps(cos2t)))).norm();
    __m256 a = nt - nc;
    __m256 b = nt + nc;
    __m256 R0 = a * a / (b * b);
    __m256 c = 1 - _mm256_blendv_ps(tdir.dot(n), -ddn, into);
    __m256 Re = R0 + (1 - R0) * c * c * c * c * c;
    __m256 Tr = 1 - Re;
    __m256 P = .25 + .5 * Re;
    __m256 RP = Re / P;
    __m256 TP = Tr / (1 - P);
    if (depth > 2)
    {
        // ans = ans.blend(obj.e + f.mult(erand48v(Xi) < P ? radiance_avx(reflRay, depth, Xi) * RP : radiance_avx(Ray_avx(x, tdir), depth, Xi) * TP), mask);
        __m256 mask9 = _mm256_cmp_ps(erand48v(Xi), P, _CMP_LT_OQ);
        ans = ans.blend(obj.e + f.mult(radiance_avx(reflRay, _mm256_and_ps(mask9, mask), depth, Xi) * RP), _mm256_and_ps(mask9, mask));
        ans = ans.blend(obj.e + f.mult(radiance_avx(Ray_avx(x, tdir), _mm256_andnot_ps(mask9, mask), depth, Xi) * TP), _mm256_andnot_ps(mask9, mask));
    }
    else
    {
        ans = ans.blend(obj.e + f.mult(radiance_avx(reflRay, mask, depth, Xi) * Re + radiance_avx(Ray_avx(x, tdir), mask, depth, Xi) * Tr), mask);
    }
    return ans;
}
int main(int argc, char *argv[])
{
    int w = 1024, h = 768, samps = argc == 2 ? atoi(argv[1]) / 4 : 1; // # samples
    Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm());        // cam pos, dir
    Vec cx = Vec(w * .5135 / h), cy = (cx % cam.d).norm() * .5135, r, *c = new Vec[w * h];
#pragma omp parallel for schedule(dynamic, 1) private(r) // OpenMP
    for (int y = 0; y < h; y++)
    { // Loop over image rows
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * 4, 100. * y / (h - 1));
        for (unsigned short x = 0, Xi[3] = {0, 0, y * y * y}; x < w; x++) // Loop cols
            for (int sy = 0, i = (h - y - 1) * w + x; sy < 2; sy++)       // 2x2 subpixel rows
                for (int sx = 0; sx < 2; sx++, r = Vec())
                { // 2x2 subpixel cols
                    for (int s = 0; s < samps; s += 8)
                    {
                        Vec_avx o_avx, d_avx;
                        for (int simd = 0; simd < 8; simd++)
                        {
                            float r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                            float r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                            Vec d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
                                    cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.d;
                            // r = r + radiance(Ray(cam.o + d * 140, d.norm()), 0, Xi) * (1. / samps);
                            o_avx.x[simd] = cam.o.x + d.x * 140;
                            o_avx.y[simd] = cam.o.y + d.y * 140;
                            o_avx.z[simd] = cam.o.z + d.z * 140;
                            d.norm();
                            d_avx.x[simd] = d.x;
                            d_avx.y[simd] = d.y;
                            d_avx.z[simd] = d.z;
                        } // Camera rays are pushed ^^^^^ forward to start in interior
                        Vec_avx r_avx = radiance_avx(Ray_avx(o_avx, d_avx), (__m256)_mm256_set1_epi32(0xffffffff), 0, Xi);
                        for (int simd = 0; simd < 8; simd++)
                            r = r + Vec(r_avx.x[simd], r_avx.y[simd], r_avx.z[simd]) * (1. / samps);
                    }
                    c[i] = c[i] + Vec(clamp(r.x), clamp(r.y), clamp(r.z)) * .25;
                }
    }
    FILE *f = fopen("image.ppm", "w"); // Write image to PPM file.
    fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
    for (int i = 0; i < w * h; i++)
        fprintf(f, "%d %d %d ", toInt(c[i].x), toInt(c[i].y), toInt(c[i].z));
}
