#ifndef TYPE_H
#define TYPE_H

#include <glad.h>

#define PI 3.141592653589
#define EPSILON 0.00001
#define INF 1e10

#define FALSE 0
#define TRUE  1

#define NEGX 0
#define POSX 1
#define NEGY 2
#define POSY 3
#define NEGZ 4
#define POSZ 5

typedef GLboolean bool;
typedef GLbyte    i8;
typedef GLubyte   u8;
typedef GLshort   i16;
typedef GLushort  u16;
typedef GLint     i32;
typedef GLuint    u32;
typedef GLint64   i64;
typedef GLuint64  u64; 

typedef GLhalf    f16;
typedef GLfloat   f32;
typedef GLdouble  f64;

#endif