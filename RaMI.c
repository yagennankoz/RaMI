#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include <hardware/flash.h>

#define DBG

#define MIDI_IN             uart0
#define MIDI_OUT            uart1

#define UART_BAUD_RATE      31250

#define UART0_IN_RX_PIN     1
#define UART1_OUT_TX_PIN    4
#define WAIT_DOWN_PIN       6
#define WAIT_UP_PIN         10
#define LED_PIN             25

#define BUFFER_SIZE             2048 
#define FLASH_TARGET_OFFSET     0x1F0000
#define WAIT_TIME_DEFAULT       125500
#define WAIT_TIME_MAX           10000000
#define WAIT_DIFF               500

typedef struct {
    uint8_t data;
    uint32_t counter;
} note;

static uint32_t wait_time;
static note data_buffer[BUFFER_SIZE];
static uint16_t in_idx;
static uint16_t out_idx;

int main();
void core0_main();
void init();
static void midi_in_callback();
static void switch_callback(uint gpio, uint32_t events);
static void save_setting();
static void load_setting();


int main()
{
    stdio_init_all();
    init();

    load_setting();
    in_idx = 0;
    out_idx = 0;

    irq_set_enabled(UART0_IRQ, true);

    gpio_put(LED_PIN, 0);
    core0_main();
    return 0;
}

void core0_main() {
    uint32_t counter;
    uint32_t note_counter;
    uint32_t elapsed;
    uint32_t led_counter;
    uint32_t led_elapsed;
    while (true) {
        while (uart_is_writable(MIDI_OUT)) {
            if (in_idx == out_idx) {
                gpio_put(LED_PIN, 0);
                continue;
            }
            counter = time_us_32();
            note_counter = data_buffer[out_idx].counter;
            if (counter >= note_counter) {
                elapsed = counter - note_counter;
                led_elapsed = counter - led_counter;
            } else {
                elapsed = UINT32_MAX - note_counter + counter;
                led_elapsed = UINT32_MAX - led_counter + counter;
            }
            if (led_elapsed > 10000) {
                gpio_put(LED_PIN, 0);
            }
            if (elapsed < wait_time) {
                continue;
            }
#ifdef DBG
//            printf ("in_idx:%04d out_idx:%04d cnt:%08x  ncnt:%08x  elp:%u\n", in_idx, out_idx, counter, note_counter, elapsed);
#endif
            uart_putc_raw(MIDI_OUT, data_buffer[out_idx++].data);
            gpio_put(LED_PIN, 1);
            led_counter = counter;
            if (out_idx >= BUFFER_SIZE) {
                out_idx = 0;
            }
        }
        gpio_put(LED_PIN, 0);
    }
}

void init() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    gpio_set_function(UART0_IN_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_OUT_TX_PIN, GPIO_FUNC_UART);

    uart_init(MIDI_IN, UART_BAUD_RATE);
    uart_set_hw_flow(MIDI_IN, false, false);
    uart_set_format(MIDI_IN, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(MIDI_IN, false);
    int UART_IRQ = MIDI_IN == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, midi_in_callback);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(MIDI_IN, true, false);

    uart_init(MIDI_OUT, UART_BAUD_RATE);
    uart_set_hw_flow(MIDI_OUT, false, false);
    uart_set_format(MIDI_OUT, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(MIDI_OUT, false);

    gpio_init(WAIT_DOWN_PIN);
    gpio_set_dir(WAIT_DOWN_PIN, GPIO_IN);
    gpio_pull_up(WAIT_DOWN_PIN);
    gpio_set_irq_enabled_with_callback(WAIT_DOWN_PIN, 4u, true, switch_callback);
    gpio_init(WAIT_UP_PIN);
    gpio_set_dir(WAIT_UP_PIN, GPIO_IN);
    gpio_pull_up(WAIT_UP_PIN);
    gpio_set_irq_enabled_with_callback(WAIT_UP_PIN, 4u, true, switch_callback);
}

static void midi_in_callback() {
    uint8_t c;

    while (uart_is_readable(MIDI_IN)) {
        c = uart_getc(MIDI_IN);
        data_buffer[in_idx].data = c;
        data_buffer[in_idx++].counter = time_us_32();
        if (in_idx >= BUFFER_SIZE) {
            in_idx = 0;
        }
    }
}

static void switch_callback(uint gpio, uint32_t events)
{
    gpio_set_irq_enabled (WAIT_UP_PIN, 4u, false);
    gpio_set_irq_enabled (WAIT_DOWN_PIN, 4u, false);

    switch (gpio) {
        case WAIT_UP_PIN:
            wait_time += WAIT_DIFF;
            if (wait_time > WAIT_TIME_MAX) {
                wait_time = WAIT_TIME_MAX;
            }
#ifdef DBG
            printf("wait time up:%u  ", wait_time);
#endif
            break;

        case WAIT_DOWN_PIN:
            wait_time -= WAIT_DIFF;
            if (wait_time > WAIT_TIME_MAX) {
                wait_time = 0;
            }
#ifdef DBG
            printf("wait time down:%u  ", wait_time);
#endif
            break;

        default:
            break;
    }
    save_setting();
    gpio_set_irq_enabled (WAIT_UP_PIN, 4u, true);
    gpio_set_irq_enabled (WAIT_DOWN_PIN, 4u, true);
}

static void save_setting()
{
    uint8_t write_data[FLASH_PAGE_SIZE];

    memcpy(write_data, &wait_time, sizeof(wait_time));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, write_data, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
#ifdef DBG
    printf(" - saved:%u\n", wait_time);
#endif
}

static void load_setting()
{
    printf("load_setting\n");
    const uint8_t *saved_wait = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    
    uint32_t wait_time_tmp;
    memcpy(&wait_time_tmp, saved_wait, sizeof(wait_time_tmp));
    
    if (wait_time_tmp <= WAIT_TIME_MAX) {
        wait_time = wait_time_tmp;
    } else {
        wait_time = WAIT_TIME_DEFAULT;
    }
}