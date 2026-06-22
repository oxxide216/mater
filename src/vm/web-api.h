#ifndef WEB_API_H
#define WEB_API_H

#include "shl/shl-defs.h"

#define false 0
#define true  1

#define NULL (void *) 0

#define da_reserve_space(da) da_reserve_space_impl(da, sizeof((da)->items[0]))

typedef unsigned char bool;

extern void print_str(char *ptr, u32 len);
extern void print_int(i32 _int);
extern void print_float(f32 _float);
extern void print_bool(bool _bool);

extern void wprint_str(char *ptr, u32 len);
extern void wprint_int(i32 _int);
extern void wprint_float(f32 _float);
extern void wprint_bool(bool _bool);

extern void eprint_str(char *ptr, u32 len);
extern void eprint_int(i32 _int);
extern void eprint_float(f32 _float);
extern void eprint_bool(bool _bool);

extern f32 mod_float(f32 a, f32 b);

extern void alert(char *ptr, u32 len);

u32 str_len(char *str);

void *walloc(u32 size);
void wfree(void *ptr);

void da_reserve_space_impl(void *da, u32 element_size);

#endif // WEB_API_H
