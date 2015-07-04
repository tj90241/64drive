//
// 64drive_writerom.c
//
// usb64drive; A open-source 64drive library.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "usb64drive.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
  const char *file, *device;
  unsigned version;
  FILE *f;
  int fd;

  if (argc < 2 || argc > 3) {
    printf("Usage: %s [device path] <rom path>\n", argv[0]);
    return EXIT_SUCCESS;
  }

  file = argc > 2 ? argv[2] : argv[1];
  device = argc > 2 ? argv[1] : "/dev/ttyUSB0";

  if ((f = fopen(file, "r")) == NULL) {
    printf("Failed to open: %s\n", file);
    return EXIT_FAILURE;
  }

  if ((fd = usb64drive_open(device)) < 0) {
    printf("Failed to open: %s\n", device);

    fclose(f);
    return EXIT_FAILURE;
  }

  if (usb64drive_get_version(fd, &version)) {
    printf("Failed to query version.\n");

    fclose(f);
    usb64drive_close(fd);
    return EXIT_FAILURE;
  }

  if (usb64drive_write_rom(fd, f)) {
    printf("Failed to write the ROM image.\n");

    fclose(f);
    usb64drive_close(fd);
    return EXIT_FAILURE;
  }

  fclose(f);
  usb64drive_close(fd);
  return EXIT_SUCCESS;
}

