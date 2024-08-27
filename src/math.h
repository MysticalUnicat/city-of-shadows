#ifndef __LIBRARY_CORE_MATH_H__
#define __LIBRARY_CORE_MATH_H__

typedef struct {
  union {
    float values[2];
    struct {
      union { float x; float r; float s; };
      union { float y; float g; float t; };
    };
  };
} Vec2;

typedef struct {
  union {
    float values[3];
    struct {
      union { float x; float r; float s; };
      union { float y; float g; float t; };
      union { float z; float b; float p; };
    };
  };
} Vec3;

typedef struct {
  union {
    float values[4];
    struct {
      union { float x; float r; float s; };
      union { float y; float g; float t; };
      union { float z; float b; float p; };
      union { float w; float a; float q; };
    };
  };
} Vec4;

//
// 2D
//             one | e0 | e1 | e2 | e01 | e20 | e12 | e012
// line:             e0   e1   e2
// point:                           e01   e20   e12
// rotor:      one                              e12
// translator: one                  e01   e20
// motor:      one                  e01   e20   e12
// 
typedef struct {
  union {
    float values[3];
    Vec3 vector;
    struct {
      union { float w; float e0; };
      union { float x; float e1; };
      union { float y; float e2; };
    };
  };
} Line2D;

typedef struct {
  union {
    float values[3];
    Vec3 vector;
    struct {
      union { float w; float e12; };
      union { float x; float e20; };
      union { float y; float e01; };
    };
  };
} Point2D;

typedef struct {
  union {
    float values[2];
    Vec2 vector;
    struct {
      union { float w; float one; };
      union { float z; float e12; };
    };
  };
} Complex, Rotor2D;

typedef struct {
  union {
    float values[3];
    Vec3 vector;
    struct {
      union { float w; float one; };
      union { float x; float e20; };
      union { float y; float e01; };
    };
  };
} Translator2D;

typedef struct {
  union {
    float values[4];
    Vec4 vector;
    struct {
      union { float w; float one; };
      union { float x; float e20; };
      union { float y; float e01; };
      union { float z; float e12; };
    };
  };
} Motor2D;

#define PGA2D_ONE(X)  _Generic((X), float: X, Line2D: 0, Point2D: 0, Complex: (X).one, Rotor2D: (X).one, Translator2D: (X).one, Motor2D: (X).one)
#define PGA2D_E0(X)   _Generic((X), float: 0, Line2D: (X).e0, Point2D: 0, Complex: 0, Rotor2D: 0, Translator2D: 0, Motor2D: 0)
#define PGA2D_E1(X)   _Generic((X), float: 0, Line2D: (X).e1, Point2D: 0, Complex: 0, Rotor2D: 0, Translator2D: 0, Motor2D: 0)
#define PGA2D_E01(X)  _Generic((X), float: 0, Line2D: (X).e2, Point2D: 0, Complex: 0, Rotor2D: 0, Translator2D: (X).e01, Motor2D: (X).e01)
#define PGA2D_E20(X)  _Generic((X), float: 0, Line2D: 0, Point2D: (X).e20, Complex: 0, Rotor2D: 0, Translator2D: (X).e20, Motor2D: (X).e20)
#define PGA2D_E12(X)  _Generic((X), float: 0, Line2D: 0, Point2D: (X).e12, Complex: (X).e12, Rotor2D: (X).e12, Translator2D: 0, Motor2D: (X).e12)
#define PGA2D_E012(X) _Generic((X), float: 0, Line2D: 0, Point2D: 0, Complex: 0, Rotor2D: 0, Translator2D: 0, Motor2D: 0)

//
// 3D
//             one | e0 | e1 | e2 | e3 | e01 | e02 | e03 | e12 | e31 | e23 | e021 | e013 | e032 | e123 | e0123
// plane:            e0   e1   e2   e3
// point:                                                                    e021   e013   e032   e123
// rotor:      one                                         e12   e31   e23
// translator: one                       e01   e02   e03
// motor:      one                       e01   e02   e03   e12   e31   e23                               e0123
// 
typedef struct {
  union {
    float values[4];
    Vec4 vector;
    struct {
      union { float w; float e0; };
      union { float x; float e1; };
      union { float y; float e2; };
      union { float z; float e3; };
    };
  };
} Plane3D;

typedef struct {
  union {
    float values[4];
    Vec4 vector;
    struct {
      union { float w; float e123; };
      union { float x; float e032; };
      union { float y; float e013; };
      union { float z; float e021; };
    };
  };
} Point3D;

typedef struct {
  union {
    float values[4];
    Vec4 vector;
    struct {
      union { float w; float one; };
      union { float x; float e23; };
      union { float y; float e31; };
      union { float z; float e12; };
    };
  };
} Quaternion, Rotor3D;

typedef struct {
  union {
    float values[4];
    Vec4 vector;
    struct {
      union { float w; float one; };
      union { float x; float e01; };
      union { float y; float e02; };
      union { float z; float e03; };
    };
  };
} Translator3D;

typedef struct {
  union {
    float values[8];
    Vec4 vectors[2];
    struct {
      union {
        Vec4 vector_1;
        struct {
          float one;
          float e23;
          float e31;
          float e12;
        };
      };
      union {
        Vec4 vector_2;
        struct {
          float e0123;
          float e01;
          float e02;
          float e03;
        };
      };
    };
  };
} DualQuaternion, Motor3D;

//

#endif

