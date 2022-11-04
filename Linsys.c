#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

//Whats been added: 
    //Both with and without multi-threading can be run in the same program and solutions compared
    //Error can be calculated between A and LU

const int RAND_LIMIT = 100; //When we generate random integers i put cap on 0 to 100

double randomize_double() //Randomize doubles from 0.0 and 10.00. Inspiration from lab 6 - task 7
{
    double d;
    d = (double)(rand()%RAND_LIMIT) + 10.0; //Only generate numbers between 10 and 100. Not close to zero to avoid infinite solution
    return d;
}

void create_random_matrix(double*** array, int n)
{
    double elem=0;
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {  
            while(elem==0) //We want nonzero numbers
            { 
                elem = randomize_double();
            }
            //elem = rand() %100;
            (*array)[i][j] = elem;
            elem=0;
        }
    }
}

void create_random_vector(double** vec, int n)
{
    double elem;
    for(int i=0; i<n; i++)
    {
        elem = randomize_double();
        (*vec)[i] = elem;
    }
}

void matrix_memory_alloc(double*** array, int n) //Inspiration from HPP-lab6-assignment7-matmul.c
{
    *array = (double**)malloc((n+2)*sizeof(double*));
    for(int i=0; i<n; i++)
    {
        (*array)[i] = (double*)malloc((n+2)*sizeof(double*));
    }
}

void vector_memory_alloc(double** vec, int n) //Inspiration from HPP-lab6-assignment7-matmul.c
{
    *vec = (double*)malloc((n+2)*sizeof(double*));
}

void matrix_memory_free(double** array, int n) //Free memory of matrix
{
    for(int i=0; i<n; i++)
    {
        free(array[i]);
    }
    free(array);
}

void print_matrix(double** array, int n) //Print matrix
{
    for(int i=0;i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            printf("%lf \t", array[i][j]);
            if(j==n-1)
            {
                printf("\n");
            }
        }
    }
    printf("\n");
}

void print_vector(double* vec, int n) //Print vector
{
    for(int i=0;i<n; i++)
    {
        printf("%lf \t", vec[i]);
    }
    printf("\n");
}

int compare_solutions(double *x_no_thread, double *x_thread, int n){ //Compare solution between parallel and serial calculations
    double x_error=0.0;
    double diff=0.0;
    for(int i = 0; i<n; n++){
        diff = abs(x_no_thread[i]-x_thread[i]);
        x_error += diff;
    }
    return x_error;
}

//Code structure taken from lab 06 in the course
void mat_mult(double **L, double **U, double **LU, int n)
{
  for (int k=0; k<n; k++) 
  {
    for (int i=0; i<n; i++) 
    {
      double L_ik = L[i][k];
      for (int j=0; j<n; j++)
      {
	    LU[i][j] += L_ik * U[k][j]; 
      }  
    }
  }
}

//Error between A and LU to see if solutions is correct
int A_ComparedWith_LU(double **A, double **L, double **U, int n){
    double error = 0.0;
    double **LU; 
    double diff =0.0;
    matrix_memory_alloc(&LU, n);
    mat_mult(L, U, LU, n);
    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            diff = abs(A[i][j]-LU[i][j]);
            error += diff;
        }
    }
    //printf("LU is: \n");
    //print_matrix(LU, n);
    matrix_memory_free(LU, n); //Free memory of LU
    return error;
}

//Program doing the LU solver in parallel
void LU_With_Threading(double **A, double *b, double **L, double **U, double *x, double *y, int n, int n_threads, int print_matrices){
    //----------------------With OpenMP------------------------------------
    //We want to decompose A = L*U where L is lower triangular matrix and U is upper triangular matrix 

    //Setting all solution vectors and matrices to zero
    for(int i=0; i<n; i++)
    {
        x[i]=0.0;
        y[i] =0.0;
        
        for(int j=0; j<n; j++)
        {
            U[i][j] = 0.0;
            L[i][j] = 0.0;
        }
        L[i][i] = 1.0;
    }

    //Initalizing variables
    double start_time = omp_get_wtime();
    double end_time;

    double sum_over_k_1;
    double sum_over_k_2;
    double u_divider;

    //jik loop order
    for(int j=0; j<n; j++) 
    {
        //Calculate U
        #pragma omp parallel for reduction(+:sum_over_k_1) num_threads(n_threads)
        for(int i=j; i<n; i++) 
        {
            sum_over_k_1 = 0;
            for(int k=0; k<j; k++)
            {
                sum_over_k_1 += L[j][k]*U[k][i];
            }
            U[j][i] = A[j][i] - sum_over_k_1;
        }

        //Calculate L
        #pragma omp parallel for reduction(+:sum_over_k_2) num_threads(n_threads)
        for(int i=j; i<n; i++) 
        {
            sum_over_k_2 =0;
            for(int k=0; k<j; k++)
            {
                sum_over_k_2 += L[i][k]*U[k][j];
            }
            
            if (U[j][j] == 0) {
				printf("Division by zero impossible!\n");
				exit(EXIT_FAILURE);
            }
            u_divider = (double)1/U[j][j];
            L[i][j] = (double)(A[i][j] - sum_over_k_2)*u_divider;
        }
    }
    
    //Solving Ly=b with forward substitution
    double sum_over_j;
    double l_divide;
    double u_divide;
    for(int j=0; j<n; j++)
    {
        sum_over_j=0.0;

        #pragma omp parallel for reduction(+:sum_over_j) num_threads(n_threads)
        for(int i=0; i<j; i++)
        {
            sum_over_j +=L[j][i]*y[i];
        }
        l_divide = (double)1/L[j][j];
        if (L[j][j] == 0) 
        {
			printf("Division by zero impossible!\n");
			exit(EXIT_FAILURE);
        }
        y[j] = (double)(b[j] - sum_over_j)*l_divide;
    }

    //Now solve UX = Y with back substitution
    for(int i=n-1; i>=0; i--) //Go backwards from n-1 to 0
    {
        double sum_over_j=0.0;

        #pragma omp parallel for reduction(+:sum_over_j) num_threads(n_threads)
        for(int j=i+1; j<n; j++)
        {
            sum_over_j += U[i][j]*x[j];
        }
        u_divide = (double)1/U[i][i];
        if (U[i][i] == 0) 
        {
			printf("Division by zero impossible!\n");
			exit(EXIT_FAILURE);
        }
        x[i] = (double)(y[i] - sum_over_j)*u_divide;
    }
    
    end_time = omp_get_wtime();
    printf("Time taken in parallell is: %lf \n",end_time-start_time);

    //printf("500th x value = %lf \n", x[500]);

}

//Program doing the LU solver in series
void LU_Without_Threading(double **A, double *b, double **L, double **U, double *x, double *y, int n, int n_threads, int print_matrices){
    //----------------------With OpenMP------------------------------------
    //We want to decompose A = L*U where L is lower triangular matrix and U is upper triangular matrix 

    //Setting all solution vectors and matrices to zero
    for(int i=0; i<n; i++)
    {
        x[i]=0.0;
        y[i] =0.0;
        
        for(int j=0; j<n; j++)
        {
            U[i][j] = 0.0;
            L[i][j] = 0.0;
        }
        L[i][i] = 1.0;
    }

    //Initalizing variables
    double start_time = omp_get_wtime();
    double end_time;

    double sum_over_k_1;
    double sum_over_k_2;
    double u_divider;
    for(int j=0; j<n; j++) 
    {
        for(int i=j; i<n; i++) 
        {
            sum_over_k_1 = 0;
            for(int k=0; k<j; k++)
            {
                sum_over_k_1 += L[j][k]*U[k][i];
            }
            U[j][i] = A[j][i] - sum_over_k_1;
        }
        for(int i=j; i<n; i++) 
        {
            sum_over_k_2 =0;
            for(int k=0; k<j; k++)
            {
                sum_over_k_2 += L[i][k]*U[k][j];
            }
            
            if (U[j][j] == 0) {
				printf("Division by zero impossible!\n");
				exit(EXIT_FAILURE);
            }
            u_divider = (double)1/U[j][j];
            L[i][j] = (double)(A[i][j] - sum_over_k_2)*u_divider;
        }
    }
    
    //Solving Ly=b with forward substitution
    double sum_over_j;
    double l_divide;
    double u_divide;
    for(int j=0; j<n; j++)
    {
        sum_over_j=0.0;

        for(int i=0; i<j; i++)
        {
            sum_over_j +=L[j][i]*y[i];
        }
        l_divide = (double)1/L[j][j];
        if (L[j][j] == 0) 
        {
			printf("Division by zero impossible!\n");
			exit(EXIT_FAILURE);
        }
        y[j] = (double)(b[j] - sum_over_j)*l_divide;
    }

    //Now solve UX = Y with back substitution
    for(int i=n-1; i>=0; i--) //Go backwards from n-1 to 0
    {
        double sum_over_j=0.0;

        for(int j=i+1; j<n; j++)
        {
            sum_over_j += U[i][j]*x[j];
        }
        u_divide = (double)1/U[i][i];
        if (U[i][i] == 0) 
        {
			printf("Division by zero impossible!\n");
			exit(EXIT_FAILURE);
        }
        x[i] = (double)(y[i] - sum_over_j)*u_divide;
    }

    end_time = omp_get_wtime();

    printf("Time taken in series is: %lf \n",end_time-start_time);
}

int main(int argc, char* argv[])
{
    if(argc != 7) //Check if correct number of inputs are given
    {
        printf("Give 6 arguments: n = matrix size, n_threads = number of threads, method=0 ,1 or 2, randomly_generated = 0 or 1, print_matrices = 0 or 1 and calculate_error = 0 or 1.\n");
        return -1;
    }

    int n = atoi(argv[1]);
    int n_threads = atoi(argv[2]);
    int method = atoi(argv[3]);
    int randomly_generated = atoi(argv[4]);
    int print_matrices = atoi(argv[5]);
    int calculate_error = atoi(argv[6]);

    if(n < 1) //Check that correct input is given
    {
        printf("Error: Positive inputs for matrix size only \n");
        return -1;
    }

    if(n_threads<1) //Check that correct input is given
    {
        printf("Error: Positive inputs for number of threads only \n");
        return -1;
    }

    if(method <0 || method >2) //Check that correct input is given
    {
        printf("Error: Give one of the following method numbers: \n");
        printf("0: Run program with in series \n");
        printf("1: Run program in parallel \n");
        printf("2: Run both with and without and compare results");
        return -1;
    }

    if(randomly_generated<0 || randomly_generated>1){
        printf("Error: Give either 0 for not random or 1 for random A and b \n");
        return -1;
    }

    if(print_matrices<0 || print_matrices>1){
        printf("Error: Give either 0 for no printing or 1 for printing matrices \n");
        return -1;
    }

    if(calculate_error<0 || calculate_error>1){
        printf("Error: Give either 0 or 1 for calculating error between A and LU\n");
        return -1;
    }

    //srand(time(NULL)); //Include to make sure new random numbers are generated each run

    //Initialize matrices and vector
    double **A; 
    double *b; 
    double **L;
    double **U;
    double *y; //Intermediate solution vector
    double *x; //Final solution vector

    //Allocate memory
    matrix_memory_alloc(&A,n);
    vector_memory_alloc(&b, n);
    matrix_memory_alloc(&L,n);
    matrix_memory_alloc(&U,n);
    vector_memory_alloc(&x,n);
    vector_memory_alloc(&y,n);

    //Store data in A and b
    if(randomly_generated){
        create_random_matrix(&A,n);
        create_random_vector(&b, n);
    }
    else{
        A[0][0] = 93.0; A[0][1] = 96.0; A[0][2] = 87.0;
        A[1][0] = 25.0; A[1][1] = 103.0; A[1][2] = 45.0;
        A[2][0] = 96.0; A[2][1] = 102.0; A[2][2] = 59.0;

        b[0] = 31.0; b[1] = 72.0; b[2] = 37.0;

        n = 3; //Override so that n = whatever your decided system is
    }

    //Different cases depending on chosen method
    switch(method){
        case 0:
            LU_Without_Threading(A, b, L, U, x, y, n, n_threads, print_matrices);
            break;
        case 1:
            LU_With_Threading(A, b, L, U, x, y, n, n_threads, print_matrices);
            break;
        case 2:
            LU_Without_Threading(A, b, L, U, x, y, n, n_threads, print_matrices);
            double *x_intermediate;
            vector_memory_alloc(&x,n);
            for(int i =0; i<n; i++){
                x_intermediate[i] = x[i];
            }
            LU_With_Threading(A, b, L, U, x, y, n, n_threads, print_matrices);
            double x_error = compare_solutions(x_intermediate, x, n);
            printf("Error between solutions = %lf \n", x_error);
            free(x_intermediate);
            break;
    }

    //If we want to calculate the error between A and L*U
    if(calculate_error){
        double error; 
        error = A_ComparedWith_LU(A, L, U, n);
        printf("Error From ||A-LU|| = %lf \n", error);
    }

    //If we want to print the matrices
    if(print_matrices==1 && method!=2){
        printf("Matrix A is: \n");
        print_matrix(A,n);

        printf("b is: \n");
        print_vector(b,n);
        
        printf("L is: \n");
        print_matrix(L,n);

        printf("U is: \n");
        print_matrix(U,n);

        printf("x is: \n");
        print_vector(x,n);
    }

    //Free all memory 
    matrix_memory_free(L,n);
    matrix_memory_free(U,n);
    matrix_memory_free(A,n);
    free(b);
    free(x);
    free(y);
}