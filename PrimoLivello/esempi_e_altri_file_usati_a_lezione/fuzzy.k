def fibo(n)
   n<2 ? 1 : {
              var n = n<4 ? fibo(n-1) : n;
              var f2 = fibo(n-2);
              n+f2
             }+n;
