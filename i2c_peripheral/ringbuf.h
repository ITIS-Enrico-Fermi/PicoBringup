#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t *   mem;
    int         size;
    int         cursor;
    int         elements;
} ringbuffer_t;

ringbuffer_t ringbuffer_create(uint8_t * memory_pool, const int pool_size);
bool ringbuffer_is_empty(ringbuffer_t * rb);
bool ringbuffer_is_full(ringbuffer_t * rb);
uint8_t ringbuffer_peek(ringbuffer_t * rb);
uint8_t ringbuffer_pop(ringbuffer_t * rb);
void ringbuffer_push(ringbuffer_t * rb, uint8_t e);
