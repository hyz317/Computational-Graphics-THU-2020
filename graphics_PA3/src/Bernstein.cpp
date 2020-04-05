#include "Bernstein.hpp"

bool Bernstein::equal(float a, float b)
{
    if (a > b - EPS && a < b + EPS) return true;
    return false;
}

Bernstein::Bernstein(int n, int k, float* t, int t_size)
{
    this->n = n;
    this->k = k;
    this->t = t;
    this->t_size = t_size;

    tpad = new float[t_size + k];
    for (int i = 0; i < t_size; i++)
        tpad[i] = t[i];
    for (int i = 0; i < k; i++)
        tpad[t_size + i] = t[t_size - 1];
}

float* Bernstein::bezier_knot(int k)
{
    float* array = new float[2 * k + 2];
    for (int i = 0; i < k + 1; i++)
        array[i] = 0;
    for (int i = k + 1; i < 2 * k + 2; i++)
        array[i] = 1;
    return array;
}

float* Bernstein::bspline_knot(int n, int k)
{
    float* array = new float[n + k + 2];
    for (int i = 0; i < n + k + 2; i++)
        array[i] = i / (float) (n + k + 1);
    return array;
}

int Bernstein::get_bpos(float mu)
{
    if (mu < t[0] || mu > t[t_size - 1])
        throw "ValueError";
    if (equal(mu, t[0])) {
        // std::cout << "EQUAL!!!\n";
        int bpos = 0;
        for (; bpos < t_size && equal(mu, t[bpos]); bpos++);
        return bpos - 1;
    }
    else {
        int bpos = 0;
        for (; bpos < t_size && t[bpos] < mu; bpos++);
        return bpos - 1;
    }
}

int Bernstein::get_range_point_num()
{
    return n + 1 - k;
}


std::pair<float, float> Bernstein::get_valid_range()
{
    return std::make_pair(t[k], t[n + 1]);
}


std::pair<float*, float*> Bernstein::evaluate(float mu)
{
    int bpos = get_bpos(mu);
    // std::cout << bpos << std::endl;
    float s[k + 2] = { 0 };
    float ds[k + 1] = { 0 };
    s[k] = 1;
    for (int i = 0; i < k + 1; i++)
        ds[i] = 1;
    for (int p = 1; p < k + 1; p++) {
        for (int ii = k - p; ii < k + 1; ii++) {
            int i = ii + bpos - k;
            float w1, w2, dw1, dw2;
            if (equal(tpad[i + p], tpad[i])) {
                w1 = mu; dw1 = 1;
            } else {
                w1 = (mu - tpad[i]) / (tpad[i + p] - tpad[i]);
                dw1 = 1 / (tpad[i + p] - tpad[i]);
            }
            if (equal(tpad[i + p + 1], tpad[i + 1])) {
                w2 = 1 - mu; dw2 = -1;
            } else {
                w2 = (tpad[i + p + 1] - mu) / (tpad[i + p + 1] - tpad[i + 1]);
                dw2 = -1 / (tpad[i + p + 1] - tpad[i + 1]);
            }
            if (p == k) 
                ds[ii] = (dw1 * s[ii] + dw2 * s[ii + 1]) * p;
            s[ii] = w1 * s[ii] + w2 * s[ii + 1];
            // std::cout << "w1: " << w1 << " w2: " << w2 << std::endl;
            // std::cout << "p: " << p << " ii: " << ii << " s:["; for (int j = 0; j < k + 2; j++) std::cout << s[j] << ", "; std::cout << "]\n";
        }
    }

    float* out_s = new float[n + 1];
    float* out_ds = new float[n + 1];

    for (int i = 0; i < n + 1; i++) {
        out_s[i] = 0;
        out_ds[i] = 0;
    }

    // std::cout << "bpos: " << bpos << " start: " << std::max(0, bpos - k) << " end: " << std::min(n, bpos) + 1 << std::endl;

    for (int i = std::max(0, bpos - k), j = 0; i < std::min(n, bpos) + 1; i++, j++) {
        out_s[i] = s[j];
        out_ds[i] = ds[j];
    }

    // for (int i = 0; i < n + 1; i++)
        // std::cout << out_s[i] << ' ';

    // std::cout << '\n';

    return std::make_pair(out_s, out_ds);
}


Bernstein::~Bernstein()
{
    delete[] t;
    delete[] tpad;
}
