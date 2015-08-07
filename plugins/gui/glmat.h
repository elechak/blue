
#ifndef _GLMAT
#define _GLMAT

#include <GL/gl.h>
#include <math.h>
#include <stdio.h>

void glmat_printMat(const GLdouble *mat);
void glmat_showMats();
void glmat_vecCross(GLdouble * a, GLdouble * b , GLdouble * out);
void glmat_vecNormalizev(GLdouble vec[3]);
void glmat_vecNormalize(GLdouble *px ,GLdouble *py,GLdouble *pz  );
void glmat_orthag(GLdouble * mat);
void glmat_mult(GLdouble * a, GLdouble * b, GLdouble * out);
void glmat_multInplace(GLdouble * a, GLdouble * b);
void glmat_scale(GLdouble * mat, GLdouble sx, GLdouble sy, GLdouble sz);
void glmat_translate(GLdouble * mat, GLdouble dx, GLdouble dy, GLdouble dz);
void glmat_rotate(GLdouble * mat, GLdouble angle, GLdouble x,GLdouble y, GLdouble z);
void glmat_loadIdent(GLdouble * mat);
GLdouble glmat_det(const GLdouble * mat);
int glmat_inv(const GLdouble * mat, GLdouble *out);

void glmat_apply3(GLdouble * transform, GLdouble * x, GLdouble *y, GLdouble *z);
void glmat_apply3v(GLdouble * transform, GLdouble * vec);
void glmat_apply4(GLdouble * transform, GLdouble * x, GLdouble *y, GLdouble *z, GLdouble *w);
void glmat_apply4v(GLdouble * transform, GLdouble * vec);

int glmat_win2obj(GLdouble win[4]);
int glmat_obj2win(GLdouble obj[4]);


#endif
