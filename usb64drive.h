//
// usb64drive.h
//
// usb64drive; A open-source 64drive library.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef USB64DRIVE_H
#define USB64DRIVE_H
#include <stdio.h>

// Closes the 64drive connection handle.
void usb64drive_close(int fd);

// Opens a 64drive connection and returns a file descriptor.
// If the file descriptor is < 0, the function failed.
int usb64drive_open(const char *path);

// Queries the device for the current firmware version.
// Returns zero on success, and non-zero on error.
int usb64drive_get_version(int fd, unsigned *version);

int usb64drive_write_rom(int fd, FILE *f);

#endif

