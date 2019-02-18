/*------------------------------------------------------------------------
  Header for Some Vector Classes
  ------------------------------------------------------------------------*/
#ifndef vecmat_h_included
#define vecmat_h_included

#include <math.h>
#include <cmath>
/*------------------------------------------------------------------------
	vec2d : 2d Vector
  ------------------------------------------------------------------------*/
class vec2d
{
public:
	double 	elem[2];

public:
    vec2d(){}
    vec2d(double x, double y){elem[0]=x; elem[1]=y;}
    vec2d(double x){elem[0]=elem[1]=x;}

    double operator()(int x) const {return elem[x];}
    double &operator()(int x) {return elem[x];}

    vec2d operator *(const double x) const {vec2d res(*this); res.elem[0]*=x; res.elem[1]*=x; return res;}
    vec2d operator /(const double x) const {vec2d res(*this); res.elem[0]/=x; res.elem[1]/=x; return res;}
    vec2d operator +(const vec2d &x) const {vec2d res(*this); res.elem[0]+=x.elem[0]; res.elem[1]+=x.elem[1]; return res;}
    vec2d operator -(const vec2d &x) const {vec2d res(*this); res.elem[0]-=x.elem[0]; res.elem[1]-=x.elem[1]; return res;}
    vec2d operator -() const {vec2d res(*this); res.elem[0] = - res.elem[0]; res.elem[1] = -res.elem[1]; return res;}
    vec2d &operator *=(const double x) {elem[0]*=x; elem[1]*=x; return (*this);}
    vec2d &operator /=(const double x) {elem[0]/=x; elem[1]/=x; return (*this);}
    vec2d &operator +=(const vec2d &x) {elem[0]+=x.elem[0]; elem[1]+=x.elem[1]; return (*this);}
    vec2d &operator -=(const vec2d &x) {elem[0]-=x.elem[0]; elem[1]-=x.elem[1]; return (*this);}
    bool operator ==(const vec2d &x) const {return((elem[0] == x.elem[0])&&(elem[1] == x.elem[1]));}
	bool operator !=(const vec2d &x) const { 
		return((elem[0] != x.elem[0]) || (elem[1] != x.elem[1])); 
	}

    double Magnitude(void) const {return(sqrt((elem[0]*elem[0])+(elem[1]*elem[1])));}
    double Magnitude2(void) const {return((elem[0]*elem[0])+(elem[1]*elem[1]));}
    double Normalise(void) { double x = Magnitude(); elem[0]/=x; elem[1]/=x; return x;}
    vec2d Normalised(void) const {vec2d res(*this); res.Normalise(); return res;}

    double Dot(const vec2d &x) const {return ( (elem[0]*x.elem[0]) + (elem[1]*x.elem[1]) );}
	double Dist(const vec2d &x) const
	{
		const double x_diff = elem[0] - x(0);
		const double y_diff = elem[1] - x(1);
		return std::sqrt(x_diff * x_diff + y_diff * y_diff);
	}
};

/*------------------------------------------------------------------------
	vec3d : 3d Vector
  ------------------------------------------------------------------------*/
class vec3d
{
public:
	double 	elem[3];

public:
    vec3d(){}
    vec3d(double x, double y, double z){elem[0]=x; elem[1]=y;elem[2]=z;}
    vec3d(double x){elem[0]=elem[1]=elem[2]=x;}

    double operator()(int x) const {return elem[x];}
    double &operator()(int x) {return elem[x];}

    vec3d operator *(const double x) const {vec3d res(*this); res.elem[0]*=x; res.elem[1]*=x; res.elem[2]*=x; return res;}
    vec3d operator /(const double x) const {vec3d res(*this); res.elem[0]/=x; res.elem[1]/=x; res.elem[2]/=x; return res;}
    vec3d operator +(const vec3d &x) const {vec3d res(*this); res.elem[0]+=x.elem[0]; res.elem[1]+=x.elem[1]; res.elem[2]+=x.elem[2]; return res;}
    vec3d operator -(const vec3d &x) const {vec3d res(*this); res.elem[0]-=x.elem[0]; res.elem[1]-=x.elem[1]; res.elem[2]-=x.elem[2]; return res;}
    vec3d &operator *=(const double x) {elem[0]*=x; elem[1]*=x; elem[2]*=x; return (*this);}
    vec3d &operator /=(const double x) {elem[0]/=x; elem[1]/=x; elem[2]/=x; return (*this);}
    vec3d &operator +=(const vec3d &x) {elem[0]+=x.elem[0]; elem[1]+=x.elem[1]; elem[2]+=x.elem[2]; return (*this);}
    vec3d &operator -=(const vec3d &x) {elem[0]-=x.elem[0]; elem[1]-=x.elem[1]; elem[2]-=x.elem[2]; return (*this);}

    double Magnitude(void) const {return(sqrt((elem[0]*elem[0])+(elem[1]*elem[1])+(elem[2]*elem[2])));}
    double Magnitude2(void) const {return((elem[0]*elem[0])+(elem[1]*elem[1])+(elem[2]*elem[2]));}
    double Normalise(void) { double x = Magnitude(); elem[0]/=x; elem[1]/=x; elem[2]/=x; return x;}
    vec3d Normalised(void) const {vec3d res(*this); res.Normalise(); return res;}

    double Dot(const vec3d &x) const {return ( (elem[0]*x.elem[0]) + (elem[1]*x.elem[1]) + (elem[2]*x.elem[2]) );}
    vec3d Cross(const vec3d &x) const
    {
    	vec3d res;
    	res.elem[0] = elem[1]*x.elem[2] - elem[2]*x.elem[1];
    	res.elem[1] = elem[2]*x.elem[0] - elem[0]*x.elem[2];
    	res.elem[2] = elem[0]*x.elem[1] - elem[1]*x.elem[0];
		return res;
    }
};

#endif