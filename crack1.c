void api_putchar(int c);

void HariMain(void) {
  //api_putchar('A');
  *((char*)0x00102600) = 0;
}
