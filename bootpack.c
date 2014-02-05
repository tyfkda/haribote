void io_hlt();

void HariMain(void) {
 fin:
  io_hlt();
  goto fin;
}
