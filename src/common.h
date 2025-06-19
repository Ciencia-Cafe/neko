// Copyright 2025 Elloramir.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#ifndef NEKO_COMMON_H
#define NEKO_COMMON_H

#define STR2(x) #x
#define STR(x) STR2(x)

#ifdef _WIN32
#define INCBIN_SECTION ".rdata, \"dr\""
#else
#define INCBIN_SECTION ".rodata"
#endif

#define INCBIN(name, file) \
	__asm__(".section " INCBIN_SECTION "\n" \
			".global incbin_" STR(name) "_start\n" \
			".balign 16\n" \
			"incbin_" STR(name) "_start:\n" \
			".incbin \"" file "\"\n" \
			\
			".global incbin_" STR(name) "_end\n" \
			".balign 1\n" \
			"incbin_" STR(name) "_end:\n" \
			".byte 0\n" \
	); \
	extern __attribute__((aligned(16))) const char incbin_ ## name ## _start[]; \
	extern                              const char incbin_ ## name ## _end[]

#endif