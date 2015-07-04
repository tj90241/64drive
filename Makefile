#
# Makefile
#
# usb64drive; A open-source 64drive library.
# Copyright (C) 2014, Tyler J. Stachecki.
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

CFLAGS := -Wall -Wextra -std=c99 -pedantic -Os

all: 64drive_writerom libusb64drive.a

libusb64drive.a: usb64drive.o
	@echo "Linking: $@"
	@$(AR) rcs $@ $^

usb64drive.o: usb64drive.c usb64drive.h
	@echo "Compiling: $@"
	@$(CC) $(CFLAGS) $< -c -o $@

64drive_writerom: 64drive_writerom.c libusb64drive.a
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -L. $< -o $@ -lusb64drive

test: test.c libusb64drive.a
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -L. $< -o $@ -lusb64drive

.PHONY: clean
clean:
	@echo "Cleaning..."
	@$(RM) *.o *.a 64drive_writerom test

