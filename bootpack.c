void io_hlt();

void HariMain(void) {
  unsigned char* p = (unsigned char*)0xa0000;
  for (int i = 0x0000; i <= 0xffff; ++i)
    p[i] = i & 0x0f;

  for (;;)
    io_hlt();
}
