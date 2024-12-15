#ifndef TYPE_H
#define TYPE_H

#include <glad.h>

#define PI 3.141592653589

#define FALSE 0
#define TRUE  1

#define RIGHT  0
#define LEFT   1
#define UP     2
#define DOWN   3
#define TOP    4
#define BOTTOM 5

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