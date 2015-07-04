//
// usb64drive.c
//
// usb64drive; A open-source 64drive library.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "usb64drive.h"
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef __USE_MISC
#define __USE_MISC
#include <termios.h>
#undef __USE_MISC
#endif

#define DEV_CMD_LOADRAM       0x20
#define DEV_CMD_DUMPRAM       0x30
#define DEV_CMD_SETSAVE       0x70
#define DEV_CMD_GETVER        0x80
#define DEV_CMD_UPGRADE       0x84
#define DEV_CMD_UPGREPORT     0x85
#define DEV_CMD_PI_RD_32      0x90
#define DEV_CMD_PI_WR_32      0x91
#define DEV_CMD_PI_WR_BL      0x94
#define DEV_CMD_PI_WR_BL_LONG 0x95

#define BANK_INVALID          0
#define BANK_CARTROM          1
#define BANK_SRAM256          2
#define BANK_SRAM768          3
#define BANK_FLASHRAM1M       4
#define BANK_FLASHPKM1M       5
#define BANK_EEPROM16         6
#define BANK_LAST             7

static ssize_t usb64drive_send_rw_cmd(int fd, int cmd,
  int bank, size_t addr, uint8_t *host_buf, size_t size);

static const uint8_t usb64drive_dev_magic[] = {0x55, 0x44, 0x45, 0x56};

void usb64drive_close(int fd) {
  close(fd);
}

int usb64drive_open(const char *path) {
  struct termios tty;
  int fd;

  if ((fd = open(path, O_RDWR | O_NOCTTY | O_SYNC)) < 0)
    return -1;

  memset(&tty, 0, sizeof(tty));
  if (tcgetattr(fd, &tty) != 0)
    return -1;

  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);

  // 8-bit chars.
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;

  // Ignore break signal.
  tty.c_iflag &= ~IGNBRK;

  // No signaling chars.
  tty.c_lflag = 0;

  // No remapping, no delays.
  tty.c_oflag = 0;

  // No blocking reads, 0.5 sec read timeout.
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 5;

  // Shut off XON/XOFF.
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);

  // Ignore modem controls, enable reading.
  tty.c_cflag |= (CLOCAL | CREAD);

  // Shut off parity.
  tty.c_cflag &= ~(PARENB | PARODD | CMSPAR);
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  // Gimme my CR.
  tty.c_iflag &= ~(ICRNL | INLCR);

  if (tcsetattr(fd, TCSANOW, &tty) != 0)
    return -1;

  return fd;
}

int usb64drive_get_version(int fd, unsigned *version) {
  uint8_t tx[4] = {DEV_CMD_GETVER, 0x43, 0x4D, 0x44};
  uint8_t rx[8];

  if (write(fd, tx, sizeof(tx)) != sizeof(tx))
    return -1;

  if (read(fd, rx, sizeof(rx)) != sizeof(rx))
    return -1;

  // Check magic.
  if (memcmp(rx + 4, usb64drive_dev_magic, sizeof(usb64drive_dev_magic))) {
    errno = EPROTO;
    return -1;
  }

  *version =
    (rx[0] << 24) |
    (rx[1] << 16) |
    (rx[2] << 8 ) |
    (rx[3] << 0 );

  return 0;
}

ssize_t usb64drive_send_rw_cmd(int fd, int cmd,
  int bank, size_t addr, uint8_t *host_buf, size_t size) {
  ssize_t i, chk;

  uint8_t tx[12] = {
    cmd,        0x43,       0x4D,      0x44,
    addr >> 24, addr >> 16, addr >> 8, addr >> 0,
    bank,       size >> 16, size >> 8, size
  };

  if (write(fd, tx, sizeof(tx)) != sizeof(tx))
    return -1;

  size &= 0x00FFFFFFU;

  for (i = 0; i < (ssize_t) size; i += chk) {
    if ((chk = (cmd == DEV_CMD_DUMPRAM)
      ? read(fd, host_buf + i, size - i)
      : write(fd, host_buf + i, size - i)
    ) < 0) {
      return -1;
    }
  }

  return size;
}

int usb64drive_write_rom(int fd, FILE *f) {
  uint8_t buf[32768];
  size_t addr;

  for (addr = 0; !feof(f); addr += sizeof(buf)) {
    while (fread(buf, sizeof(buf), 1, f) != 1) {
      if (feof(f))
        break;

      if (ferror(f))
        return -1;
    }

    if (usb64drive_send_rw_cmd(fd, DEV_CMD_LOADRAM,
      BANK_CARTROM, addr, buf, sizeof(buf)) < 0)
      return 0;
  }

  return 0;
}

