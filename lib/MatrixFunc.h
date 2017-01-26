#pragma once

// C(m x l) = A(m × n) x B(n × l)
static void MultiMatrix( const float *A, const float *B, const int& m, const int& n, const int& l, float *C )
{
    int i, j, k;

    for( i=0; i<m; i++ ){
        for( j=0; j<l; j++ ){
            *( C+l*i+j ) = 0.0;
            for( k=0; k<n; k++ ){
                *( C+l*i+j ) += ( *( A+n*i+k ) )*( *( B+l*k+j ) );
            }
        }
    }
}


// cross product : X(3 x 1) = A(3×1) [cross] B(3×1)
void CrossMatrix( float A[3], float B[3], float X[3] )
{
    X[0] = A[1]*B[2] - A[2]*B[1];
    X[1] = A[2]*B[0] - A[0]*B[2];
    X[2] = A[0]*B[1] - A[1]*B[0];
}


// transpose : A(m × n)
void TransMatrix( float *A, int m, int n, float *A_trans )
{
    int i,j;
    for( i=0; i<m; i++ ){
        for( j=0; j<n; j++ ){
            *( A_trans+m*j+i ) = *( A+n*i+j );
        }
    }
}


// inverse : A (n x n)
void InvMatrix( float *A, int n, float *A_inv )
{
    int i, j, k;
    float L[n][n], U[n][n], buf[n][n];

    // initialize
    for( i=0; i<n; i++ ){
        for( j=0; j<n; j++ ){
            U[i][j] = 0.0;
            if( i == j )	buf[i][j] = L[i][j] = 1.0;
            else			buf[i][j] = L[i][j] = 0.0;
        }
    }
    // LU decomposition
    for( i=0; i<n; i++ ){
        for( j=i; j<n; j++ ){
            U[i][j] = *( A+n*i+j );
            for( k=0; k<i; k++ ){
                U[i][j] -= L[i][k]*U[k][j]; // U = c[i][j]=a[i][j]-sigma(k=1,i-1)b[i][k]c[k][j] (i<=j)
            }
        }
        for( j=i+1; j<n; j++ ){
            L[j][i] = *( A+n*j+i );
            for( k=0; k<i; k++ ){
                L[j][i] -= L[j][k]*U[k][i]; // L = b[i][j]={a[i][j]-sigma(k=1,j-1)b[i][k]c[k][j]}/c[j][j] (i>j)
            }
            L[j][i] /= U[i][i];
        }
    }
    // calculate inverse : Ax = v
    for( k=0; k<n; k++ ){
        for( i=0; i<n; i++ ){
            for( j=0; j<i; j++ ){
                buf[i][k] -= L[i][j]*buf[j][k];
            }
        }
        for( i=n-1; i>=0; i-- ){
            *( A_inv+n*i+k ) = buf[i][k];
            for( j=n-1; j>i; j-- ){
                *( A_inv+n*i+k ) -= U[i][j]*( *( A_inv+n*j+k ) );
            }
            *( A_inv+n*i+k ) /= U[i][i];
        }
    }
}


// pseudo inverse : A (m x n)
void PInvMatrix( float *A, int m, int n, float *A_pseudo )
{
    int i, k;
    if( m < n ) k = m;
    else        k = n;
    float A_trans[n][m], **AA_trans, **AA_inv, *buffer;
    
    AA_trans = (float **)malloc(sizeof(float *) * k);
    AA_inv = (float **)malloc(sizeof(float *) * k);
    buffer = (float *)malloc(sizeof(float) * k * k * 2);
    for( i=0; i<k; i += k ) {
        AA_trans[i] = AA_trans_ + i * k;
        AA_inv[i] = AA_inv_ + (k + i) * k;
    }

    TransMatrix( A, m, n, *A_trans );
    if( m < n ){ // rank = m : A+ = A_trans*(A*A_trans)_inv
        MultiMatrix( A, *A_trans, m, n, m, *AA_trans );
        InvMatrix( *AA_trans, m, *AA_inv );
        MultiMatrix( *A_trans, *AA_inv, n, m, m, A_pseudo );
    }else{ // rank = n : A+ = (A_trans*A)_inv*A_trans
        MultiMatrix( *A_trans, A, n, m, n, *AA_trans );
        InvMatrix( *AA_trans, n, *AA_inv );
        MultiMatrix( *AA_inv, *A_trans, n, n, m, A_pseudo );
    }

    free(AA_trans);
    free(AA_inv);
    free(buffer);
}


// weighted pseudo inverse : A(m x n) & W(n x l)
void WPInvMatrix( float *A, float *W, int m, int n, int l, float *A_wp )
{
    float A_trans[n][m], W_inv[l][l];
    float AW[m][l], AWA[m][m], AWA_inv[m][m], WA[l][m];

    TransMatrix( A, m, n, *A_trans );
    InvMatrix( W, l, *W_inv );

    MultiMatrix( A, *W_inv, m, n, l, *AW );
    MultiMatrix( *AW, *A_trans, m, l, m, *AWA );
    InvMatrix( *AWA, m, *AWA_inv );

    MultiMatrix( *W_inv, *A_trans, l, l, m ,*WA );

    MultiMatrix( *WA, *AWA_inv, l, m, m, A_wp );

}
