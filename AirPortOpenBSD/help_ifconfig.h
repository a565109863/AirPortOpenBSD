//
//  help_ifconfig.hpp
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2019/7/29.
//  Copyright © 2019 Zhong-Mac. All rights reserved.
//

#ifndef help_ifconfig_hpp
#define help_ifconfig_hpp

#include "compat.h"

void    ifconfig(const char **argv, int argc);
void    ifconfig(const char *);

void    setifflags(const char *, int);
void    setifnwid(const char *, int);
void    setifjoin( const char *, int);
void    delifjoin(const char *, int);
void    showjoin(const char *, int);
void    setifbssid(const char *, int);
void    setifnwkey(const char *, int);
void    setifwpa(const char *, int);
void    setifwpaprotos(const char *, int);
void    setifwpaakms(const char *, int);
void    setifwpaciphers(const char *, int);
void    setifwpagroupcipher(const char *, int);
void    setifwpakey(const char *, int);
void    setifchan(const char *, int);

void    process_join_commands();

int
pkcs5_pbkdf2(const char *pass, size_t pass_len, const uint8_t *salt,
             size_t salt_len, uint8_t *key, size_t key_len, unsigned int rounds);

#endif /* help_ifconfig_hpp */
