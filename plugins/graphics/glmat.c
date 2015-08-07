
#include "glmat.h"





void glmat_printMat(const GLdouble *mat){    
    int ccc;
    for( ccc = 0 ; ccc<16 ; ccc+=4){
        printf("  %f   %f   %f   %f\n", mat[ccc], mat[ccc+1], mat[ccc+2], mat[ccc+3]);
    }
    printf("\n");        
}


void glmat_showMats(){
    GLdouble mod_mat[16],proj_mat[16];
    GLint viewport[4];
    
    glGetDoublev(GL_MODELVIEW_MATRIX, mod_mat);
    glGetDoublev(GL_PROJECTION_MATRIX, proj_mat);
    glGetIntegerv(GL_VIEWPORT, viewport);

    printf("\n\n\n");      
    printf("Viewport:\n");
    printf("  %i   %i   %i   %i\n", viewport[0], viewport[1], viewport[2], viewport[3]);
    printf("\n");      
    
    int ccc;
    printf("Projection(T):\n");
    for( ccc = 0 ; ccc<16 ; ccc+=4){
        printf("  %f   %f   %f   %f\n", proj_mat[ccc], proj_mat[ccc+1], proj_mat[ccc+2], proj_mat[ccc+3]);
    }
    printf("\n");        
    
    printf("Model(T):\n");
    for( ccc = 0 ; ccc<16 ; ccc+=4){
        printf("  %f   %f   %f   %f\n", mod_mat[ccc], mod_mat[ccc+1], mod_mat[ccc+2], mod_mat[ccc+3]);
    }
    printf("\n");   
}


/* VECTORS */

void glmat_vecNormalizev(GLdouble vec[3]){
    GLdouble mag = sqrt( vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2] );
    if (mag == 0.0) return;
    vec[0] /= mag;
    vec[1] /= mag;
    vec[2] /= mag;
}

void glmat_vecNormalize(GLdouble *px ,GLdouble *py,GLdouble *pz  ){    
    GLdouble mag = sqrt( (*px)*(*px) + (*py)*(*py) + (*pz)*(*pz) );
    if (mag == 0.0) return;
    *px /= mag;
    *py /= mag;
    *pz /= mag;
}


void glmat_vecCross(GLdouble * a, GLdouble * b , GLdouble * out){
    out[0] = a[1]*b[2] - a[2]*b[1];
    out[1] = a[2]*b[0] - a[0]*b[2];
    out[2] = a[0]*b[1] - a[1]*b[0];
}


/* MATRIX */

void glmat_loadIdent(GLdouble * mat){
    mat[0] = mat[5]=mat[10]=mat[15] = 1.0;
    mat[1] = mat[2] = mat[3]= mat[4]= 
    mat[6]= mat[7]= mat[8]= mat[9]= 
    mat[11]= mat[12]= mat[13]= mat[14]= 0.0;
}


/* ensure that axes are orthagonal */
void glmat_orthag(GLdouble * mat){
    GLdouble *x, *y, *z, magx,magy,magz;

    x = mat;
    y = mat+4;
    z = mat+8;

    magx = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
    magy = sqrt(y[0]*y[0] + y[1]*y[1] + y[2]*y[2]);
    magz = sqrt(z[0]*z[0] + z[1]*z[1] + z[2]*z[2]);

    if ((magx ==0.0) || (magy==0.0) || (magz==0.0)) return;
    
    z[0] /=magz;
    z[1] /=magz;
    z[2] /=magz;
    y[0] /=magy;
    y[1] /=magy;
    y[2] /=magy;
    x[0] /=magx;
    x[1] /=magx;
    x[2] /=magx;

    z[0] = x[1]*y[2]  - x[2]*y[1];
    z[1] = x[2]*y[0]  - x[0]*y[2];
    z[2] = x[0]*y[1]  - x[1]*y[0];

    y[0] =  x[2]*z[1] - x[1]*z[2];
    y[1] =  x[0]*z[2] - x[2]*z[0];
    y[2] =  x[1]*z[0] - x[0]*z[1];

    x[0]*=magx; x[1]*=magx; x[2]*=magx;
    y[0]*=magy; y[1]*=magy; y[2]*=magy;
    z[0]*=magz; z[1]*=magz; z[2]*=magz;
}



GLdouble glmat_det(const GLdouble * mat){
return (    mat[12]*mat[9]*mat[6]*mat[3]-
                mat[8]*mat[13]*mat[6]*mat[3]-
                mat[12]*mat[5]*mat[10]*mat[3]+
                mat[4]*mat[13]*mat[10]*mat[3]+
                mat[8]*mat[5]*mat[14]*mat[3]-
                mat[4]*mat[9]*mat[14]*mat[3]-
                mat[12]*mat[9]*mat[2]*mat[7]+
                mat[8]*mat[13]*mat[2]*mat[7]+
                mat[12]*mat[1]*mat[10]*mat[7]-
                mat[0]*mat[13]*mat[10]*mat[7]-
                mat[8]*mat[1]*mat[14]*mat[7]+
                mat[0]*mat[9]*mat[14]*mat[7]+
                mat[12]*mat[5]*mat[2]*mat[11]-
                mat[4]*mat[13]*mat[2]*mat[11]-
                mat[12]*mat[1]*mat[6]*mat[11]+
                mat[0]*mat[13]*mat[6]*mat[11]+
                mat[4]*mat[1]*mat[14]*mat[11]-
                mat[0]*mat[5]*mat[14]*mat[11]-
                mat[8]*mat[5]*mat[2]*mat[15]+
                mat[4]*mat[9]*mat[2]*mat[15]+
                mat[8]*mat[1]*mat[6]*mat[15]-
                mat[0]*mat[9]*mat[6]*mat[15]-
                mat[4]*mat[1]*mat[10]*mat[15]+
                mat[0]*mat[5]*mat[10]*mat[15]);
}

int glmat_inv(const GLdouble * mat, GLdouble *out){
    GLdouble det = glmat_det(mat);
    if (det==0) return 0;

    out[0]= (-mat[13]*mat[10]*mat[7] +mat[9]*mat[14]*mat[7] +mat[13]*mat[6]*mat[11]-mat[5]*mat[14]*mat[11] -mat[9]*mat[6]*mat[15] +mat[5]*mat[10]*mat[15])/det;
    out[1]= ( mat[13]*mat[10]*mat[3] -mat[9]*mat[14]*mat[3] -mat[13]*mat[2]*mat[11]+mat[1]*mat[14]*mat[11] +mat[9]*mat[2]*mat[15] -mat[1]*mat[10]*mat[15])/det;    
    out[2]= (-mat[13]*mat[6]* mat[3] +mat[5]*mat[14]*mat[3] +mat[13]*mat[2]*mat[7]-mat[1]*mat[14]*mat[7] -mat[5]*mat[2]*mat[15] +mat[1]*mat[6]* mat[15])/det;
    out[3]= ( mat[9]* mat[6]* mat[3] -mat[5]*mat[10]*mat[3] -mat[9]* mat[2]*mat[7]+mat[1]*mat[10]*mat[7] +mat[5]*mat[2]*mat[11] -mat[1]*mat[6]* mat[11])/det;
    out[4]= ( mat[12]*mat[10]*mat[7] -mat[8]*mat[14]*mat[7] -mat[12]*mat[6]*mat[11]+mat[4]*mat[14]*mat[11] +mat[8]*mat[6]*mat[15] -mat[4]*mat[10]*mat[15])/det;
    out[5]= (-mat[12]*mat[10]*mat[3] +mat[8]*mat[14]*mat[3] +mat[12]*mat[2]*mat[11]-mat[0]*mat[14]*mat[11] -mat[8]*mat[2]*mat[15] +mat[0]*mat[10]*mat[15])/det;
    out[6]= ( mat[12]*mat[6]* mat[3] -mat[4]*mat[14]*mat[3] -mat[12]*mat[2]*mat[7]+mat[0]*mat[14]*mat[7] +mat[4]*mat[2]*mat[15] -mat[0]*mat[6]* mat[15])/det;
    out[7]= (-mat[8]* mat[6]* mat[3] +mat[4]*mat[10]*mat[3] +mat[8]* mat[2]*mat[7]-mat[0]*mat[10]*mat[7] -mat[4]*mat[2]*mat[11] +mat[0]*mat[6]* mat[11])/det;
    out[8]= (-mat[12]*mat[9]* mat[7] +mat[8]*mat[13]*mat[7] +mat[12]*mat[5]*mat[11]-mat[4]*mat[13]*mat[11] -mat[8]*mat[5]*mat[15] +mat[4]*mat[9]* mat[15])/det;
    out[9]= ( mat[12]*mat[9]* mat[3] -mat[8]*mat[13]*mat[3] -mat[12]*mat[1]*mat[11]+mat[0]*mat[13]*mat[11] +mat[8]*mat[1]*mat[15] -mat[0]*mat[9]* mat[15])/det;
    out[10]=(-mat[12]*mat[5]* mat[3] +mat[4]*mat[13]*mat[3] +mat[12]*mat[1]*mat[7]-mat[0]*mat[13]*mat[7] -mat[4]*mat[1]*mat[15] +mat[0]*mat[5]* mat[15])/det;
    out[11]=( mat[8]* mat[5]* mat[3] -mat[4]*mat[9]* mat[3] -mat[8]* mat[1]*mat[7]+mat[0]*mat[9]* mat[7] +mat[4]*mat[1]*mat[11] -mat[0]*mat[5]* mat[11])/det;
    out[12]=( mat[12]*mat[9]* mat[6] -mat[8]*mat[13]*mat[6] -mat[12]*mat[5]*mat[10]+mat[4]*mat[13]*mat[10] +mat[8]*mat[5]*mat[14] -mat[4]*mat[9]* mat[14])/det;
    out[13]=(-mat[12]*mat[9]* mat[2] +mat[8]*mat[13]*mat[2] +mat[12]*mat[1]*mat[10]-mat[0]*mat[13]*mat[10] -mat[8]*mat[1]*mat[14] +mat[0]*mat[9]* mat[14])/det;
    out[14]=( mat[12]*mat[5]* mat[2] -mat[4]*mat[13]*mat[2] -mat[12]*mat[1]*mat[6]+mat[0]*mat[13]*mat[6] +mat[4]*mat[1]*mat[14] -mat[0]*mat[5]* mat[14])/det;
    out[15]=(-mat[8]* mat[5]* mat[2] +mat[4]*mat[9]* mat[2] +mat[8]* mat[1]*mat[6]-mat[0]*mat[9]* mat[6] -mat[4]*mat[1]*mat[10] +mat[0]*mat[5]* mat[10])/det;
    return 1;
}

/* Multiply the matrix a by matrix b */
void glmat_mult(GLdouble * a, GLdouble * b, GLdouble * out){

    out[0] =  a[0]*b[0]   +   a[4]*b[1]   +    a[8]*b[2]   +    a[12]*b[3];
    out[1] =  a[1]*b[0]   +   a[5]*b[1]   +    a[9]*b[2]   +    a[13]*b[3];
    out[2] =  a[2]*b[0]   +   a[6]*b[1]   +    a[10]*b[2]  +    a[14]*b[3];
    out[3] =  a[3]*b[0]   +   a[7]*b[1]   +    a[11]*b[2]  +    a[15]*b[3];

    out[4] =  a[0]*b[4]   +   a[4]*b[5]   +    a[8]*b[6]   +    a[12]*b[7];
    out[5] =  a[1]*b[4]   +   a[5]*b[5]   +    a[9]*b[6]   +    a[13]*b[7];
    out[6] =  a[2]*b[4]   +   a[6]*b[5]   +    a[10]*b[6]  +    a[14]*b[7];
    out[7] =  a[3]*b[4]   +   a[7]*b[5]   +    a[11]*b[6]  +    a[15]*b[7];

    out[8] =  a[0]*b[8]   +   a[4]*b[9]   +    a[8]*b[10]  +   a[12]*b[11];
    out[9] =  a[1]*b[8]   +   a[5]*b[9]   +    a[9]*b[10]  +   a[13]*b[11];
    out[10] = a[2]*b[8]   +   a[6]*b[9]   +    a[10]*b[10] +   a[14]*b[11];
    out[11] = a[3]*b[8]   +   a[7]*b[9]   +    a[11]*b[10] +   a[15]*b[11];

    out[12] = a[0]*b[12]   +   a[4]*b[13]   +    a[8]*b[14]  +   a[12]*b[15];
    out[13] = a[1]*b[12]   +   a[5]*b[13]   +    a[9]*b[14]  +   a[13]*b[15];
    out[14] = a[2]*b[12]   +   a[6]*b[13]   +    a[10]*b[14] +   a[14]*b[15];
    out[15] = a[3]*b[12]   +   a[7]*b[13]   +    a[11]*b[14] +   a[15]*b[15];
}

void glmat_multInplace(GLdouble * a, GLdouble * b){
    GLdouble out[16];

    out[0] =  a[0]*b[0]   +   a[4]*b[1]   +    a[8]*b[2]   +    a[12]*b[3];
    out[1] =  a[1]*b[0]   +   a[5]*b[1]   +    a[9]*b[2]   +    a[13]*b[3];
    out[2] =  a[2]*b[0]   +   a[6]*b[1]   +    a[10]*b[2]  +    a[14]*b[3];
    out[3] =  a[3]*b[0]   +   a[7]*b[1]   +    a[11]*b[2]  +    a[15]*b[3];

    out[4] =  a[0]*b[4]   +   a[4]*b[5]   +    a[8]*b[6]   +    a[12]*b[7];
    out[5] =  a[1]*b[4]   +   a[5]*b[5]   +    a[9]*b[6]   +    a[13]*b[7];
    out[6] =  a[2]*b[4]   +   a[6]*b[5]   +    a[10]*b[6]  +    a[14]*b[7];
    out[7] =  a[3]*b[4]   +   a[7]*b[5]   +    a[11]*b[6]  +    a[15]*b[7];

    out[8] =  a[0]*b[8]   +   a[4]*b[9]   +    a[8]*b[10]  +   a[12]*b[11];
    out[9] =  a[1]*b[8]   +   a[5]*b[9]   +    a[9]*b[10]  +   a[13]*b[11];
    out[10] = a[2]*b[8]   +   a[6]*b[9]   +    a[10]*b[10] +   a[14]*b[11];
    out[11] = a[3]*b[8]   +   a[7]*b[9]   +    a[11]*b[10] +   a[15]*b[11];

    out[12] = a[0]*b[12]   +   a[4]*b[13]   +    a[8]*b[14]  +   a[12]*b[15];
    out[13] = a[1]*b[12]   +   a[5]*b[13]   +    a[9]*b[14]  +   a[13]*b[15];
    out[14] = a[2]*b[12]   +   a[6]*b[13]   +    a[10]*b[14] +   a[14]*b[15];
    out[15] = a[3]*b[12]   +   a[7]*b[13]   +    a[11]*b[14] +   a[15]*b[15];

    a[0] = out[0]; a[1] = out[1]; a[2] = out[2]; a[3] = out[3];
    a[4] = out[4]; a[5] = out[5]; a[6] = out[6]; a[7] = out[7];
    a[8] = out[8]; a[9] = out[9]; a[10] = out[10]; a[11] = out[11];
    a[12] = out[12]; a[13] = out[13]; a[14] = out[14]; a[15] = out[15];
}





void glmat_scale(GLdouble * mat, GLdouble sx, GLdouble sy, GLdouble sz){
    static GLdouble scale_mat[] =  {1.0 ,0.0 ,0.0 ,0.0,
                        0.0 ,1.0 ,0.0 ,0.0 ,
                        0.0 ,0.0 ,1.0 ,0.0 ,
                        0.0 ,0.0 ,0.0 ,1.0 };

    scale_mat[0] =  sx;
    scale_mat[5] =  sy;
    scale_mat[10] = sz;
    glmat_multInplace(mat, scale_mat);
}

void glmat_translate(GLdouble * mat, GLdouble dx, GLdouble dy, GLdouble dz){
    static GLdouble translate_mat[] =  {
                        1.0 , 0.0 , 0.0 , 0.0,
                        0.0 , 1.0 , 0.0 , 0.0 ,
                        0.0 , 0.0 , 1.0 , 0.0 ,
                        0.0 , 0.0 , 0.0 , 1.0 };

    translate_mat[12] =  dx;
    translate_mat[13] =  dy;
    translate_mat[14] = dz;
    glmat_multInplace(mat, translate_mat);
}



void glmat_rotate(GLdouble * mat, GLdouble angle, GLdouble x,GLdouble y, GLdouble z){
    static GLdouble R[16] ;
    GLdouble c,s,t;
    GLdouble mag;

    mag = sqrt(x*x + y*y + z*z);
    if (mag == 0.0) return;

    x/=mag;
    y/=mag;
    z/=mag;

    angle=angle*(3.14159265359/180.0);
    c=cos(angle);
    s=sin(angle);
    t=1.0-c;

    R[0] = t*x*x +c;
    R[1] =t*x*y - s*z;
    R[2] =t*x*z +s*y;
    R[3]=0.0;
    R[4] =t*x*y+s*z;
    R[5] =t*y*y+c;
    R[6] =t*y*z-s*x;
    R[7] = 0.0;
    R[8] =t*x*z-s*y;
    R[9] =t*y*z+s*x;
    R[10] =t*z*z+c;
    R[11] =0.0 ;
    R[12] =0.0 ;
    R[13] =0.0 ;
    R[14] =0.0 ;
    R[15] =1.0 ;

    glmat_multInplace(mat, R);
    glmat_orthag(mat);
}





/* apply the transform to the vector [x,y,z] */
void glmat_apply3(GLdouble * transform, GLdouble * x, GLdouble *y, GLdouble *z){
    GLdouble vx = *x;
    GLdouble vy = *y;
    GLdouble vz = *z;

    *x =  transform[0]*vx   +   transform[4]*vy   +    transform[8]*vz   +    transform[12];
    *y =  transform[1]*vx   +   transform[5]*vy   +    transform[9]*vz   +    transform[13];
    *z =  transform[2]*vx   +   transform[6]*vy   +    transform[10]*vz  +    transform[14];
}

void glmat_apply3v(GLdouble * transform, GLdouble * vec){
    GLdouble vx = vec[0];
    GLdouble vy = vec[1];
    GLdouble vz = vec[2];

    vec[0] =  transform[0]*vx   +   transform[4]*vy   +    transform[8]*vz   +    transform[12];
    vec[1] =  transform[1]*vx   +   transform[5]*vy   +    transform[9]*vz   +    transform[13];
    vec[2] =  transform[2]*vx   +   transform[6]*vy   +    transform[10]*vz  +    transform[14];
}

void glmat_apply4(GLdouble * transform, GLdouble * x, GLdouble *y, GLdouble *z, GLdouble *w){
    GLdouble vx = *x;
    GLdouble vy = *y;
    GLdouble vz = *z;
    GLdouble vw = *w;

    *x =  transform[0]*vx   +   transform[4]*vy   +    transform[8]*vz   +    transform[12]*vw;
    *y =  transform[1]*vx   +   transform[5]*vy   +    transform[9]*vz   +    transform[13]*vw;
    *z =  transform[2]*vx   +   transform[6]*vy   +    transform[10]*vz  +    transform[14]*vw;
    *w = transform[3]*vx   +   transform[7]*vy   +    transform[11]*vz  +    transform[15]*vw;
}

void glmat_apply4v(GLdouble * transform, GLdouble * vec){
    GLdouble vx = vec[0];
    GLdouble vy = vec[1];
    GLdouble vz = vec[2];
    GLdouble vw = vec[3];

    vec[0] =  transform[0]*vx   +   transform[4]*vy   +    transform[8]*vz   +    transform[12]*vw;
    vec[1] =  transform[1]*vx   +   transform[5]*vy   +    transform[9]*vz   +    transform[13]*vw;
    vec[2] =  transform[2]*vx   +   transform[6]*vy   +    transform[10]*vz  +    transform[14]*vw;
    vec[3] = transform[3]*vx   +   transform[7]*vy   +    transform[11]*vz  +    transform[15]*vw;
}




/* pass in windows coordinates, get back object coordinates */
int glmat_win2obj(GLdouble win[4]){

    GLdouble trans_mat[16], mod_mat[16],proj_mat[16];
    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mod_mat);
    glGetDoublev(GL_PROJECTION_MATRIX, proj_mat);

    /* deal with the passed in coordinates */
    win[2] = -1.0;  // yes this is correct set the z coordinate to -1 (see the normalization to -1 to 1 below)
    win[3] = 1.0;  // set w to 1

    /* normalize x and y to ranges between -1 and 1 */    
    win[0] = ((win[0] - viewport[0])/viewport[2]) * 2 - 1;
    win[1] = ((win[1] - viewport[1])/viewport[3]) * 2 - 1;


    glmat_mult(mod_mat, proj_mat, trans_mat);
    
    /* reuse proj_mat
       from this point on the proj_mat is the inverse transform to go from win to obj coordinates
    */
    if (! glmat_inv(trans_mat, proj_mat)) return 0;

    glmat_apply4v(proj_mat, win);
    
    return 1;
}


int glmat_obj2win(GLdouble obj[4]){

    obj[3] =1.0;
    
    GLdouble mod_mat[16],proj_mat[16];
    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mod_mat);
    glGetDoublev(GL_PROJECTION_MATRIX, proj_mat);

    glmat_apply4v(mod_mat, obj);
    glmat_apply4v(proj_mat, obj);

    if (obj[3] != 1.0)return 0;
    
    obj[0] = (obj[0] * 0.5 + 0.5) * viewport[2] + viewport[0];
    obj[1] = (obj[1] * 0.5 + 0.5) * viewport[3] + viewport[1];
    obj[2] = 0.0;

    return 1;
}












