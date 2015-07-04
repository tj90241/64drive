//
// test.c
//
// usb64drive; A open-source 64drive library.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "usb64drive.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char *argv[]) {
  const char *device;
  unsigned version;
  int fd;

  if (argc > 2) {
    printf("Usage: %s <device path>\n", argv[0]);
    return EXIT_SUCCESS;
  }

  device = argc > 1 ? argv[1] : "/dev/ttyUSB0";

  if ((fd = usb64drive_open(device)) < 0) {
    printf("Failed to open: %s\n", device);
    return EXIT_FAILURE;
  }

  printf("usb64drive_open: pass\n");

  if (usb64drive_get_version(fd, &version)) {
    printf("Failed to query version.\n");
    usb64drive_close(fd);
    return EXIT_FAILURE;
  }

  printf("usb64drive_get_version: pass [0x%.8X]\n", version);

  usb64drive_close(fd);
  return EXIT_SUCCESS;
}

