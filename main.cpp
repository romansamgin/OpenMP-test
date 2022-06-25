#include <iostream>
#include <vector>
#include <random>

#include <omp.h>
#include <time.h>
#include <sys/time.h>


using namespace std;

const int64_t arrsize = 10000000;

const int64_t s4 = arrsize / 4;

const int num_threads = 4;

int getRandomNumber(int min, int max)
{
    static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0); 
    return static_cast<int>(rand() * fraction * (max - min + 1) + min);
}

int main() {
    srand(time(0));
    omp_set_dynamic(0);
    omp_set_num_threads(num_threads);

    struct timeval start[3], end[3];
    double Time_end[3] = {0};
    
    // BLOCK 1
    vector<int64_t> a(arrsize), b(arrsize), x1(arrsize), x2(arrsize);
    for(int64_t i = 0; i < arrsize; ++i) {
        a[i] = getRandomNumber(-10, 10);
        b[i] = getRandomNumber(-10, 10);
    }

    x1[0] = getRandomNumber(-10, 10);
    x2[0] = x1[0];
    a[0] = 1;
    b[0] = 0;
    
    
    //BLOCK 2
    gettimeofday(&start[1], NULL);
    for(int64_t i = 1; i < arrsize; ++i) {
        x1[i] = a[i] * x1[i - 1] + b[i];
    }
    gettimeofday(&end[1], NULL);
   
    Time_end[1] = fabs((end[1].tv_usec - start[1].tv_usec)) / 1000.0; 
    cout << "Последовательно: " << Time_end[1] << endl;
    

    //BLOCK 3    
    gettimeofday(&start[2], NULL);
    
    int64_t a1 = 0, b1 = 0, a2 = 0, b2 = 0, a3 = 0, b3 = 0;
    
    #pragma omp parallel for
    for(int n = s4; n <= 3 * s4; n += s4) 
    {
        int64_t ax = 1, bx = 0;
        
        if (n == s4)
        {
            for(int64_t i = n; i > 1; --i) 
            {
                ax *= a[i];
                bx += ax * b[i - 1];
            }
            bx += b[n];
            ax *= a[1];
            
            a1 = ax;
            b1 = bx;
        }
        
        if (n == 2 * s4)
        {
            for(int64_t i = n; i > s4; --i) 
            {
                ax *= a[i];
                bx += ax * b[i - 1];
            }
            bx += b[n];
            ax *= a[s4];
            
            a2 = ax;
            b2 = bx;
        }
        
        if (n == 3 * s4)
        {
            for(int64_t i = n; i > 2 * s4; --i) 
            {
                ax *= a[i];
                bx += ax * b[i - 1];
            }
            bx += b[n];
            ax *= a[2 * s4];
            
            a3 = ax;
            b3 = bx;
        }
    }
    
    
    //BLOCK 4
    x2[s4] = a1 * x2[0] + b1;
    x2[2 * s4] = a2 * x2[s4] + b2;
    x2[3 * s4] = a3 * x2[2 * s4] + b3;

    
    //BLOCK 5 
    #pragma omp parallel for
    for(int j = 0; j < 4; ++j) 
    {
        int64_t k = j * s4;
        for(int64_t i = k + 1; i < k + s4; ++i) {
            x2[i] = a[i] * x2[i - 1] + b[i];
        }
        
    }
        
    gettimeofday(&end[2], NULL);
    Time_end[2] = fabs((end[2].tv_usec - start[2].tv_usec)) / 1000.0; 
    
    cout << "Параллельно: " << Time_end[2] << endl;   
    
    
    //BLOCK 6
    for(int64_t i = 0; i < x1.size(); ++i) {
        if (x1[i] != x2[i]) {
            cout<<"Результат сравнения результатов: Error"  <<endl;
            return 0;
        }
    }
   
    cout<<"Результат сравнения результатов: OK" <<endl;
    
    
    return 0;
}
