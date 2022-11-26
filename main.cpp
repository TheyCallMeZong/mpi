
#include "/opt/homebrew/Cellar/open-mpi/4.1.4_2/include/mpi.h"
#include <iostream>
#include <chrono>

using namespace std;

double y(double x)
{
    return 10-x;
}

double trapezoidalIntegral(double a, double b, int start, int finish, double width) {
    double trapezoidal_integral = 0;
    for(int step = start; step < finish; step++) {
        const double x1 = a + step*width;
        const double x2 = a + (step+1)*width;

        trapezoidal_integral += 0.5*(x2-x1)*(y(x1) +y(x2));
    }

    return trapezoidal_integral;
}

int main(int argc, char **argv) {
    int num_procs, my_id, thread_size;
    MPI_Status status;
    double a = 1, b = 5;
    int n = 1000000000;
    double interval[3];
    unsigned int start_time =  clock(); // начальное время

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    // Получаем количество запущенных процессов
    MPI_Comm_size(MPI_COMM_WORLD, &thread_size);
    double resulti = 0;
    double result = 0;
    if (thread_size == 1){
        resulti = trapezoidalIntegral(a, b, 0, n, (b-a)/n);
    }
    if (my_id == 0){
        for (int i = 1; i < thread_size; i++){
            int start = (i - 1) * (n / (thread_size - 1));
            int finish = (i - 1) * (n / (thread_size - 1)) + (n / (thread_size - 1));
            //запихиваем начальную и конечную точки в массив
            interval[0] = start;
            interval[1] = finish;
            interval[2] = (b - a) / n;

            MPI_Send(&interval, 3, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
    } else{
        MPI_Recv(&interval, 3, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        resulti = trapezoidalIntegral(a, b, interval[0], interval[1], interval[2]);
    }

    MPI_Reduce(&resulti, &result, thread_size, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    unsigned int end_time = clock(); // конечное время
    unsigned int search_time = end_time - start_time; // искомое время
    if (my_id == 0){
        printf("Result - %f\n", result);
        printf("Time - %umc\n", search_time / 1000);
    }
    MPI_Finalize();
}