//
//  firmware.hpp
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2020/7/18.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#ifndef firmware_h
#define firmware_h

#include <string.h>

#include "firmwarevar.hpp"

struct firmware {
    const char *name;
    const unsigned char *data;
    const unsigned int size;
};

#define FIRMWARE(_name, _data, _size) \
    .name = _name, .data = _data, .size = _size

extern const struct firmware firmwares[];
extern const int firmwares_total;



#endif /* firmware_h */
