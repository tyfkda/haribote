#include "fd.h"
#include "bootpack.h"
#include "stdio.h"  // NULL
#include "string.h"  // memcpy, strncmp
#include "util.h"

#define MAX_CLUSTER  (2880)
#define CLUSTER_SIZE  (512)

#define FINFO_TOP  ((FDINFO*)(ADR_DISKIMG + 0x002600))
#define FINFO_MAX  (224)

#define DISK_FAT           ((unsigned char*)(ADR_DISKIMG + 0x000200))
#define DISK_CLUSTER_DATA  ((unsigned char*)(ADR_DISKIMG + 0x003e00))

short get_next_cluster(short cluster) {
  const unsigned char* p = DISK_FAT + (cluster >> 1) * 3;
  if ((cluster & 1) == 0) {
    return (p[0] | p[1] << 8) & 0xfff;
  } else {
    return (p[1] >> 4 | p[2] << 4) & 0xfff;
  }
}

static void set_next_cluster(short cluster, short next) {
  unsigned char* p = DISK_FAT + (cluster >> 1) * 3;
  if ((cluster & 1) == 0) {
    p[0] = next;
    p[1] = (p[1] & 0xf0) | ((next >> 8) & 0x0f);
  } else {
    p[1] = (p[1] & 0x0f) | ((next << 4) & 0xf0);
    p[2] = next >> 4;
  }
}

static void deleteFatClusters(short startCluster) {
  for (short cluster = startCluster; cluster < 0xff0; ) {
    short next = get_next_cluster(cluster);
    set_next_cluster(cluster, 0x000);  // Free
    cluster = next;
  }
}

void fd_close(FDHANDLE* fh) {
  if (fh->modified) {
    if (fh->cluster > 0) {
      deleteFatClusters(fh->cluster);
      set_next_cluster(fh->cluster, 0xfff);  // End mark.
    }
    fh->finfo->size = fh->pos;

    // Update timestamp.
    unsigned char t[5];
    int year = read_rtc(t);
    fh->finfo->date = ((year - 1980) << 9) | ((t[0] - 1) << 5) | (t[1] - 1);
    fh->finfo->time = (t[2] << 11) | (t[3] << 5) | (t[4] / 2);
  }
  fh->finfo = NULL;
}

static void make_file_name83(char s[12], const char* name) {
  memset(s, ' ', 11);
  int j = 0;
  for (int i = 0; j < 11 && name[i] != '\0'; ++i) {
    if (name[i] == '.') {
      j = 8;
    } else {
      s[j++] = name[i];
    }
  }
}

int fd_writeopen(FDHANDLE* fh, const char* filename) {
  fh->finfo = NULL;
  fh->pos = 0;
  fh->cluster = 0;
  fh->modified = FALSE;

  char s[12];
  make_file_name83(s, filename);
  FDINFO* finfo;
  for (int i = 0; i < FINFO_MAX; ++i) {
    finfo = &FINFO_TOP[i];
    if (finfo->name[0] == 0x00 || finfo->name[0] == 0xe5) {  // End of table, or deleted entry.
      memset(finfo, 0x00, sizeof(FDINFO));
      memcpy(finfo->name, s, 11);
      finfo->type = 0x20;  // Normal file.
      goto found;
    }
    if ((finfo->type & 0x18) == 0) {
      if (strncmp((char*)finfo->name, s, 11) == 0)
        goto found;
    }
  }
  return FALSE;

 found:
  fh->finfo = finfo;
  return TRUE;
}

static FDINFO* fd_search(const char* filename) {
  char s[12];
  make_file_name83(s, filename);
  for (int i = 0; i < FINFO_MAX; ++i) {
    FDINFO* finfo = &FINFO_TOP[i];
    if (finfo->name[0] == 0x00)  // End of table.
      return NULL;
    if ((finfo->type & 0x18) == 0) {
      if (strncmp((char*)finfo->name, s, 11) == 0)
        return finfo;
    }
  }
  return NULL;
}

int fd_delete(const char* filename) {
  FDINFO* finfo = fd_search(filename);
  if (finfo == NULL)
    return FALSE;

  finfo->name[0] = 0xe5;  // Delete mark.
  deleteFatClusters(finfo->clustno);
  return TRUE;
}

int fd_open(FDHANDLE* fh, const char* name) {
  fh->pos = 0;
  fh->cluster = 0;
  fh->modified = FALSE;

  fh->finfo = fd_search(name);
  if (fh->finfo == NULL)
    return FALSE;
  fh->cluster = fh->finfo->clustno;
  return TRUE;
}

static unsigned char* clusterData(int cluster) {
  return DISK_CLUSTER_DATA + cluster * CLUSTER_SIZE;
}

int fd_read(FDHANDLE* fh, void* dst, int requestSize) {
  int readSize = 0;
  unsigned char* p = dst;
  while (requestSize > 0) {
    if (fh->pos >= (int)fh->finfo->size)
      break;
    char forward = TRUE;
    int nextClusterPos = (fh->pos + CLUSTER_SIZE) & -CLUSTER_SIZE;
    if (nextClusterPos > (int)fh->finfo->size)
      nextClusterPos = fh->finfo->size;
    int blockBytes = nextClusterPos - fh->pos;
    if (blockBytes > requestSize) {
      blockBytes = requestSize;
      forward = FALSE;
    }
    const unsigned char* src = clusterData(fh->cluster) + (fh->pos % CLUSTER_SIZE);
    memcpy(p, src, blockBytes);
    p += blockBytes;
    fh->pos += blockBytes;
    if (forward)
      fh->cluster = get_next_cluster(fh->cluster);
    readSize += blockBytes;
    requestSize -= blockBytes;
  }
  return readSize;
}

short allocate_cluster(void) {
  for (int i = 0; i < MAX_CLUSTER; ++i) {
    if (get_next_cluster(i) == 0) {
      set_next_cluster(i, 0xfff);  // Write end mark.
      return i;
    }
  }
  return -1;
}

int fd_write(FDHANDLE* fh, const void* srcData, int requestSize) {
  if (requestSize <= 0)
    return 0;

  fh->modified = TRUE;
  int writeSize = 0;
  unsigned char* src = (unsigned char*)srcData;
  while (requestSize > 0) {
    if (fh->pos == 0) {  // First write.
      if (fh->finfo->clustno > 0) {  // Exist old file: overwrite.
        fh->cluster = fh->finfo->clustno;
      } else {  // Not exist: allocate new cluster.
        fh->cluster = fh->finfo->clustno = allocate_cluster();
        // TODO: Error check.
      }
    } else if ((fh->pos % CLUSTER_SIZE) == 0) {  // Forward next cluster.
      short nextCluster = get_next_cluster(fh->cluster);
      if (nextCluster < 0xff0) {  // Valid: use it.
        fh->cluster = nextCluster;
      } else {
        nextCluster = allocate_cluster();
        // TODO: Error check.
        set_next_cluster(fh->cluster, nextCluster);
        fh->cluster = nextCluster;
      }
    }

    int size = CLUSTER_SIZE - (fh->pos % CLUSTER_SIZE);
    if (requestSize < size)
      size = requestSize;
    unsigned char* dst = clusterData(fh->cluster) + (fh->pos % CLUSTER_SIZE);
    memcpy(dst, src, size);
    fh->pos += size;
    writeSize += size;
    dst += size;
    requestSize -= size;
  }
  return writeSize;
}

static int calc_cluster(FDHANDLE* fh, int newpos) {
  int clusterCount = newpos / CLUSTER_SIZE - fh->pos / CLUSTER_SIZE;
  int cluster = fh->cluster;
  if (newpos < fh->pos) {  // If the new position is backward,
    // Then search target cluster from the top.
    cluster = fh->finfo->clustno;
    clusterCount = newpos / CLUSTER_SIZE;
  }

  for (int i = 0; i < clusterCount; ++i)
    cluster = get_next_cluster(cluster);
  return cluster;
}

void fd_seek(FDHANDLE* fh, int offset, int origin) {
  int newpos = fh->pos;
  switch (origin) {
  case 0:  newpos = offset; break;
  case 1:  newpos += offset; break;
  case 2:  newpos = fh->finfo->size + offset; break;
  }
  if (newpos < 0)
    newpos = 0;
  else if (newpos > (int)fh->finfo->size)
    newpos = fh->finfo->size;
  fh->cluster = calc_cluster(fh, newpos);
  fh->pos = newpos;
}




#include "console.h"
#include "int.h"
#include "mtask.h"

typedef unsigned char  u_int8_t;
typedef unsigned int   u_int32_t;

static u_int32_t fdc_interrupt = 0; // for 0x26 FDC interrupt

static int fdc_chk_interrupt() {
  return fdc_interrupt;
}

static u_int32_t fdc_wait_interrupt() {
  while (!fdc_chk_interrupt());
  return TRUE;
}

static void fdc_clear_interrupt() {
  fdc_interrupt = 0;
}

// IRQ-06 : FDC
int* inthandler26(int *esp) {
  (void)esp;
  ++fdc_interrupt;
  io_out8(PIC0_OCW2, 0x66);  // Notify IRQ-06 recv finish to PIC0
  return 0;
}

static unsigned char in8(int adr)  { return io_in8(adr); }
static void out8(int adr, unsigned char dat)  { io_out8(adr, dat); }
static void disableInterrupt()  { io_cli(); }
static void enableInterrupt()  { io_sti(); }

static void sysPrintc(int c) {
  CONSOLE* cons = task_now()->cons;
  char s[] = { c, '\0' };
  cons_putstr0(cons, s);
}

static void sysPrints(const char* str) {
  CONSOLE* cons = task_now()->cons;
  cons_putstr0(cons, str);
}

static void sysPrintBin(int x) {
  CONSOLE* cons = task_now()->cons;
  char s[30];
  sprintf(s, "%02x", x);
  cons_putstr0(cons, s);
}

#define DMA_DATABUF     1024
#define FDC_RESULT_MAXCOUNT 0x10

#define DMA_ADD_SEC     0x04  //channel2 low address
#define DMA_CNT_SEC     0x05  //channel2 count address
#define DMA_TOP         0x81  //channel2 high address

#define DMA_CMD_PRI     0xD0
#define DMA_CMD_SEC     0x08
#define DMA_REQ_PRI     0xD2
#define DMA_REQ_SEC     0x09
#define DMA_SGL_MSK_PRI 0xD4
#define DMA_SGL_MSK_SEC 0x0A
#define DMA_MOD_PRI     0xD6
#define DMA_MOD_SEC     0x0B
#define DMA_CLR_FLP_PRI 0x0C
#define DMA_CLR_FLP_SEC 0xD8
#define DMA_MSR_CLR_PRI 0xDA
#define DMA_MSR_CLR_SEC 0x0D
#define DMA_CLR_MSK_PRI 0xDC
#define DMA_CLR_MSK_SEC 0x0E
#define DMA_ALL_MSK_PRI 0xDE
#define DMA_ALL_MSK_SEC 0x0F


#define FDC_SRA  0x3f0// FDC status registerA (R)
#define FDC_SRB  0x3f1// FDC status registerB (R) 
#define FDC_DOR  0x3f2// FDC Control register (R/W)
#define FDC_MSR  0x3f4// FDC Status register (R)
#define FDC_DSR  0x3f4// FDC data rate select register (W)
#define FDC_DAT  0x3f5// FDC Data(R/W)
#define FDC_DIR  0x3f7// FDC digital input register (R)
#define FDC_CCR  0x3f7// FDC configuration control register (W)

#define MSR_RQM    0x80
#define MSR_DIO    0x40
#define MSR_BUSY   0x10
#define MSR_READY  0

/* FDC CMD */
#define CMD_SPECIFY         0x03
#define CMD_RECALIBRATE     0x07
#define CMD_SENSE_INT_STS   0x08
#define CMD_SEEK            0x0f
#define CMD_READ            0x46  //MT=0,MF=1,SK=0
#define CMD_WRITE           0x45 //MT=0,MF=1,SK=0

/*
  == FDC_CMD_SUB format ==
  x x x x x HD US1 US0
  x is anyone.
  HD is head number.
  US1 and US0 are drive number of FD.

  This cmd is used as the second byte of almost all command.
*/
#define CMD_SUB 0x00 //HD=0, US1 & US0 = 0

static u_int8_t dma_databuf[DMA_DATABUF];

static struct _dma_trans {
  u_int32_t count;
  u_int32_t addr;
} dma_trans;

static struct FDC_RESULTS {
  u_int8_t gets;
  u_int8_t req_sense;
  u_int32_t status_count;
  u_int8_t status[10];
} fdc_results;


static void fdc_dma_start() {
  out8(DMA_SGL_MSK_SEC, 0x02);
}

static void fdc_dma_stop() {
  out8(DMA_SGL_MSK_SEC, 0x06);
}

void init_dma() {
  // DMAC reset
  out8(DMA_MSR_CLR_PRI, 0x00);
  out8(DMA_MSR_CLR_SEC, 0x00);

  out8(DMA_CMD_PRI, 0x00);
  out8(DMA_CMD_SEC, 0x00);

  // DMAC mode register setting
  out8(DMA_MOD_PRI, 0xc0);
  out8(DMA_MOD_SEC, 0x46);

  out8(DMA_SGL_MSK_PRI, 0x00);
}

static void init_dma_r() {
  fdc_dma_stop();

  out8(DMA_MSR_CLR_SEC, 0x00);
  out8(DMA_CLR_FLP_SEC, 0);

  out8(DMA_MOD_SEC, 0x46);
  disableInterrupt();
  out8(DMA_ADD_SEC, dma_trans.addr >> 0);
  out8(DMA_ADD_SEC, dma_trans.addr >> 8);
  out8(DMA_TOP, dma_trans.addr >> 16);
  out8(DMA_CNT_SEC, dma_trans.count >> 0);
  out8(DMA_CNT_SEC, dma_trans.count >> 8);
  enableInterrupt();
  fdc_dma_start();
}

static void init_dma_w() {
  fdc_dma_stop();

  out8(DMA_MSR_CLR_SEC, 0x00);
  out8(DMA_CLR_FLP_SEC, 0);

  out8(DMA_MOD_SEC, 0x4a);  // memory >> I/O
  disableInterrupt();
  out8(DMA_ADD_SEC, dma_trans.addr >> 0);
  out8(DMA_ADD_SEC, dma_trans.addr >> 8);
  out8(DMA_TOP, dma_trans.addr >> 16);
  out8(DMA_CNT_SEC, dma_trans.count >> 0);
  out8(DMA_CNT_SEC, dma_trans.count >> 8);
  enableInterrupt();
  fdc_dma_start();
}

// @return < 0 : Retry fault.
static int fdc_wait_msrStatus(u_int8_t mask, u_int8_t expected) {
  for (int count = 0; count < FDC_RESULT_MAXCOUNT; ++count) {
    u_int8_t status = in8(FDC_MSR);
    if ((status & mask) == expected)
      return status;
  }
  return -1;
}

static int fdc_cmd(const u_int8_t* cmd, const int length) {
  //sysPrints("[FDC] cmd busy check.\n");
  if (!fdc_wait_msrStatus(MSR_BUSY, MSR_READY)) {
    sysPrints("[FDC] cmd busy check error.\n");
    return FALSE;
  }
  //sysPrints("[FDC] cmd busy check [OK]\n");

  //sysPrints("[FDC] cmd out and msr check.\n");
  for (int i = 0; i < length; ++i) {
    if (!fdc_wait_msrStatus(MSR_RQM | MSR_DIO, MSR_RQM)) {
      sysPrints("[FDC] msr RQM|DIO error\n");
      return FALSE;
    }
    out8(FDC_DAT, cmd[i]);
  }
  //sysPrints("[FDC] cmd out and msr check [OK]\n");

  return TRUE;
}

// FDC Read Result Phase
static int fdc_read_results() {
  fdc_results.status_count = 0;

  if (fdc_wait_msrStatus(MSR_RQM | MSR_DIO, MSR_RQM | MSR_DIO) < 0) {
    sysPrints("fdc result phase error 1\n");
    return FALSE;
  }

  u_int8_t* msr = &fdc_results.status[0];
  for (int i = 0; ; ++i) {
    *msr++ = in8(FDC_DAT);
    ++fdc_results.status_count;

    int status = fdc_wait_msrStatus(MSR_RQM, MSR_RQM);
    if (status < 0) {
      sysPrints("fdc result phase error 2\n");
      return FALSE;
    }
    if (!(status & MSR_DIO)) {
      char s[30];
      sprintf(s, "count: %d\n", i);
CONSOLE* cons = task_now()->cons;
      cons_putstr0(cons, s);
      return TRUE;
    }
  }
}

static int fdc_sense_interrupt() {
  static const u_int8_t cmd[] = { CMD_SENSE_INT_STS };

  fdc_clear_interrupt();
  if (fdc_cmd(cmd, sizeof(cmd)) != TRUE) {
    sysPrints("[FDC] sense interrupt status cmd error\n");
    return FALSE;
  }
  fdc_read_results();
  return TRUE;
}

static void fdc_motor_on() {
  out8(FDC_DOR, 0x1c);
}

static void fdc_motor_off() {
  out8(FDC_DOR, 0x0c);
}

static int fdc_recalibrate() {
  static const u_int8_t cmd[] = { CMD_RECALIBRATE, CMD_SUB };

  fdc_clear_interrupt();

  //sysPrints("[FDC] Recalibrate Cmd.\n");
  if (!fdc_cmd(cmd, sizeof(cmd))) {
    sysPrints("[FDC] Recalibrate Cmd error\n");
    return FALSE;
  }
  //sysPrints("[FDC] Recalibrate Cmd [OK]\n");

  if (!fdc_wait_interrupt())
    sysPrints("[FDC] wait interrupt error\n");

  /* get result */
  fdc_sense_interrupt();

  if (!fdc_wait_msrStatus(MSR_BUSY, MSR_READY)) {
    sysPrints("[FDC] Recalibrate  wait fail\n");
    return FALSE;
  }
  return TRUE;
}

static void fdc_specify() {
  fdc_motor_on();

  static const u_int8_t specify_cmd[] = {
    CMD_SPECIFY,
    0xc1,  // ?
    0x10   // ?
  };
  fdc_clear_interrupt();
  fdc_cmd(specify_cmd, sizeof(specify_cmd));

  fdc_motor_off();
}

void init_fdc() {
  init_dma();
  dma_trans.addr = (u_int32_t)&dma_databuf[0];
  dma_trans.count = 512;
  init_dma_r();

  out8(FDC_DOR, 0x0);
  out8(FDC_CCR, 0x0);
  out8(FDC_DOR, 0xc);

  fdc_motor_on();

  fdc_specify();
}

static int fdc_seek(u_int8_t track) {
  u_int8_t cmd[] = {
    CMD_SEEK,       // 0x0f
    0,
    track
  };

  fdc_clear_interrupt();

  //sysPrints("[FDC] seek cmd check.\n");
  if (!fdc_cmd(cmd, sizeof(cmd))) {
    sysPrints("[FDC] seek cmd error\n");
    return FALSE;
  }
  //sysPrints("[FDC] seek cmd check [OK]\n");

  if (!fdc_wait_interrupt()) {
    sysPrints("[FDC][SEEK] wait interrupt error\n");
    return FALSE;
  }

  /* get result */
  if (!fdc_sense_interrupt()) {
    sysPrints("[FDC][SEEK] SIS error\n");
    return FALSE;
  }

  return TRUE;
}

void* fdc_read(int head, int track, int sector) {
CONSOLE* cons = task_now()->cons;
cons_putstr0(cons, "recalibrate\n");
  if (!fdc_recalibrate()) {
    sysPrints("[FDC][READ] recalibrate error\n");
    return NULL;
  }
  if (!fdc_seek(track)) {
    sysPrints("[FDC][READ] seek error\n");
    return NULL;
  }

  init_dma_r();

  u_int8_t cmd[] = {
    CMD_READ,
    head << 2,   // head
    track,       // track
    head,        // head
    sector,      // sector
    0x2,         // sector length (0x2 = 512byte)
    0x12,        // end of track (EOT)
    0x1b,        // dummy GSR
    0            // dummy STP
  };

cons_putstr0(cons, "move\n");
  fdc_cmd(cmd, sizeof(cmd));
  fdc_dma_stop();
  fdc_read_results();

  fdc_motor_off();

  // write the binary which we get from DMA
  sysPrints("READ DATA:\n");
  for (int j = 0; j < 1; ++j) {
    for (int i = 0; i < 16; ++i) {
      sysPrintBin(dma_databuf[j * 16 + i]);
      sysPrintc(' ');
    }
    sysPrintc('\n');
  }

  return NULL;
}

int fdc_write(void* buf, int head, int track, int sector) {
  sysPrints("FDC_WRITE\n");
  if (!fdc_recalibrate()) {
    sysPrints("[FDC][WRITE] recalibrate error\n");
    return FALSE;
  }

  sysPrints("SEEK\n");
  if (!fdc_seek(track)) {
    sysPrints("[FDC][WRITE] seek error\n");
    return FALSE;
  }

  //for (int i = 0; buf[i] != '\0' || i < 512; ++i)
  //  dma_databuf[i] = buf[i];
  memcpy(dma_databuf, buf, 512);

  init_dma_w();

  u_int8_t cmd[] = {
    CMD_WRITE,
    head << 2,      // head
    track,          // track
    head,           // head
    sector,         // sector
    0x2,            // sector length (0x2 = 512byte)
    0x12,           // end of track (EOT)
    0x1b,           // dummy GSR
    0               // dummy STP
  };

  fdc_clear_interrupt();

  sysPrints("WRITE\n");
  if (!fdc_cmd(cmd, sizeof(cmd))) {
    sysPrints("[FDC][WRITE] cmd error\n");
    return FALSE;
  }

  if (!fdc_wait_interrupt()) {
    sysPrints("[FDC][WRITE] wait interrupt error\n");
    return FALSE;
  }

  if (!fdc_read_results()) {
    sysPrints("[FDC][WRITE] read result error\n");
    return FALSE;
  }
  sysPrints("WRITE OK\n");

  return TRUE;
}
