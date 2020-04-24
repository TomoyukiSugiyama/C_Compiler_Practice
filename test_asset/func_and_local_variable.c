sum(m, n) {
  acc = 0;
  for (i = m; i <= n; i = i + 1)
    acc = acc + i;
  return acc;
}

main() { return sum(1, 10); }