fibonacci(n) {
  if (n == 0)
    return 0;
  if (n == 1)
    return 1;
  if (n >= 2) {
    x = fibonacci(n - 1);
    y = fibonacci(n - 2);
    return x + y;
  }
}

main() { return fibonacci(10); }