def fibo(n)
   n<2 ? 1 : {
              var f1 = fibo(n-1);
              var f2 = fibo(n-2);
              f1+f2
             };
