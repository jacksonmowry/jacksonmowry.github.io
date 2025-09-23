void expr() {
  if (lookahead == EOF) {
    return;
  }

  match('i');
  cond();
  match('t');
  expr();
  puts("true");
  match('e');
  expr();
  puts("false");
}
