#!/usr/bin/python
#coding: utf8

import struct

def btea(v, n, k): #xxtea加密算法
    MX = lambda: ((z>>5)^(y<<2)) + ((y>>3)^(z<<4))^(sum^y) + (k[(p & 3)^e]^z)
    u32 = lambda x: x & 0xffffffff

    y = v[0]
    sum = 0
    DELTA = 0x9e3779b9
    if n > 1: 
        z = v[n-1]
        q = 6 + 52 / n
        while q > 0:
            q -= 1
            sum = u32(sum + DELTA)
            e = u32(sum >> 2) & 3
            p = 0
            while p < n - 1:
                y = v[p+1]
                z = v[p] = u32(v[p] + MX())
                p += 1
            y = v[0]
            z = v[n-1] = u32(v[n-1] + MX())
        return True
    elif n < -1:
        n = -n
        q = 6 + 52 / n
        sum = u32(q * DELTA)
        while sum != 0:
            e = u32(sum >> 2) & 3
            p = n - 1
            while p > 0:
                z = v[p-1]
                y = v[p] = u32(v[p] - MX())
                p -= 1
            z = v[n-1]
            y = v[0] = u32(v[0] - MX())
            sum = u32(sum - DELTA)
        return True
    return False
    
if __name__ == '__main__': #简单试验
    print "key", "1234567890123456"
    key = struct.unpack("=iiii", "1234567890123456")
    v = [110, 111, 112, 113]
    print v
    btea(v, 4, key)
    print v
    btea(v, -4, key)
    print v
    
