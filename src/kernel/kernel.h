/*
	A simple structure to save some bootloader requests that might be useful later

	"Inspired" by https://github.com/malwarepad/cavOS/blob/master/src/kernel/include/bootloader.h
*/

#ifndef KERNEL_H
#define KERNEL_H 1

#include <limine.h>

struct parse_bootloader {
	uint64_t memory_total;
	uint64_t entry_count;

	LIMINE_PTR(struct limine_memmap_entry **) memory_entries;
};

static struct parse_bootloader parsed_limine_data;

#endif