fibonacci(n) {
  if (n == 0)
    return 3;
  if (n == 1)
    return 2;
  if (n == 2)
    return fibonacci(n - 1) + fibonacci(n - 2);
}

main() { return fibonacci(2); }