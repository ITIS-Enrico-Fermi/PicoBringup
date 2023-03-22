#include "ringbuf.h"
#include <stdint.h>
#include <stdbool.h>

ringbuffer_t ringbuffer_create(uint8_t * memory_pool, const int pool_size) {
    ringbuffer_t r = {
        .mem = memory_pool,
        .size = pool_size,
        .cursor = 0,
        .elements = 0
    };

    return r;
}

bool ringbuffer_is_empty(ringbuffer_t * rb) {
    return rb->elements > 0;
}

bool ringbuffer_is_full(ringbuffer_t * rb) {
    return rb->elements == rb->size;
}

uint8_t ringbuffer_peek(ringbuffer_t * rb) {
    return rb->mem[rb->cursor];
}

uint8_t ringbuffer_pop(ringbuffer_t * rb) {
    if(ringbuffer_is_empty(rb))
        return 0;
    
    uint8_t e = ringbuffer_peek(rb);

    rb->elements--;
    rb->cursor--;

    if(rb->cursor < 0)
        rb->cursor = rb->size - 1;

    return e;
}

void ringbuffer_push(ringbuffer_t * rb, uint8_t e) {
    if(ringbuffer_is_full(rb))
        return;

    rb->cursor++;
    if(rb->cursor >= rb->size)
        rb->cursor = 0;

    rb->elements++;
    
    rb->mem[rb->cursor] = e;
}
