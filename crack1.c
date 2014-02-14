void api_putchar(int c);
void api_end(void);

void HariMain(void) {
  //api_putchar('A');
  *((char*)0x00102600) = 0;
  api_end();
}
