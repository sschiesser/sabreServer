/*
 * Simple Raw HID functions for Linux - for use with Teensy RawHID example
 * http://www.pjrc.com/teensy/rawhid.html
 * Copyright (c) 2009 PJRC.COM, LLC
 *
 *  rawhid_open - open 1 or more devices
 *  rawhid_recv - receive a packet
 *  rawhid_send - send a packet
 *  rawhid_close - close a device
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above description, website URL and copyright notice and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Version 1.0: Initial Release
 *
 * Version 1.1: adapted to C++ and provide device listing on OS X
 * @author: jasch www.jasch.ch
 * @date: 20140727
 *
 */
#ifndef RAW_HID_H
#define RAW_HID_H

#ifdef __cplusplus
extern "C" {
#endif
    
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <unistd.h>
    #include <IOKit/IOKitLib.h>
    #include <IOKit/hid/IOHIDLib.h>
        
    #define BUFFER_SIZE 64
    #define MAX_NUM_DEVICES 32

    // a linked list of all opened HID devices, so the caller can simply refer to them by number
    typedef struct hid_struct hid_t;
    typedef struct buffer_struct buffer_t;
    static hid_t *first_hid = NULL;
    static hid_t *last_hid = NULL;

    static IOHIDDeviceRef * device_array;

    struct hid_struct {
        IOHIDDeviceRef ref;
        int open;
        uint8_t buffer[BUFFER_SIZE];
        buffer_t *first_buffer;
        buffer_t *last_buffer;
        struct hid_struct *prev;
        struct hid_struct *next;
    };

    struct buffer_struct {
        struct buffer_struct *next;
        uint32_t len;
        uint8_t buf[BUFFER_SIZE];
    };

    int rawhid_open(int max, int vid, int pid, int usage_page, int usage);
    int rawhid_recv(int num, void *buf, int len, int timeout);
    int rawhid_send(int num, void *buf, int len, int timeout);
    void rawhid_close(int num);
    int rawhid_isOpen(int num);
    int rawhid_listdevices(int * index, long * vid, long * pid, char ** manufacturer_name, char ** product_name);

    // private functions, not intended to be used from outside this file
    static void add_hid(hid_t *);
    static hid_t * get_hid(int);
    static void free_all_hid(void);
    static void hid_close(hid_t *);
    static void attach_callback(void *, IOReturn, void *, IOHIDDeviceRef);
    static void detach_callback(void *, IOReturn, void *hid_mgr, IOHIDDeviceRef dev);
    static void timeout_callback(CFRunLoopTimerRef, void *);
    static void input_callback(void *, IOReturn, void *, IOHIDReportType,
                               uint32_t, uint8_t *, CFIndex);
    
    // public functions are defined in "raw_hid.h"
    
#ifdef __cplusplus
}
#endif

#endif /* RAW_HID_H */
