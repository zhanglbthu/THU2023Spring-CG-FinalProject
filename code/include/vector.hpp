#ifdef VECTOR_HPP
#define VECTOR_HPP

struct Vec
{
    double x, y, z;
    Vec(double x0 = 0, double y0 = 0, double z0 = 0)
    {
        x = x0;
        y = y0;
        z = z0;
    }
    Vec operator+(const Vec &b) const { return Vec(x + b.x, y + b.y, z + b.z); }
    Vec operator-(const Vec &b) const { return Vec(x - b.x, y - b.y, z - b.z); }
    Vec operator*(double b) const { return Vec(x * b, y * b, z * b); }
    Vec operator/(double b) const { return Vec(x / b, y / b, z / b); }
    Vec mult(const Vec &b) const { return Vec(x * b.x, y * b.y, z * b.z); }
    Vec &norm() { return *this = *this * (1 / sqrt(x * x + y * y + z * z)); }
    double length() { return sqrt(x * x + y * y + z * z); }
    double dot(const Vec &b) const { return x * b.x + y * b.y + z * b.z; }
    Vec operator%(const Vec &b) const { return Vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
    //	double& operator[](size_t i) { return data[i]; }
    const double &operator[](size_t i) const { return i == 0 ? x : (i == 1 ? y : z); }
};

#endif // VECTOR_HPP