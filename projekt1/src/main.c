/********************************************************************************
 * @brief Thread example by using pthread in C.
 * 
 * @note  Thread functions in C must have void* as return type and void* as
 *        only input argument, so multiple input arguments have to be packed
 *        in a struct.
 ********************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <gpiod_utils.h>
#include <semaphore.h>

#define SEMAPHORE_ID_PRINT 0

/********************************************************************************
 * @brief Struct for packing input arguments for thread functions.
 * 
 * @param id
 *        Thread ID stored as a positive integer.
 * @param delay_time_ms
 *         Delay time between prints in ms.
 ********************************************************************************/
struct thread_args {
    struct gpiod_line* led_a;
    struct gpiod_line* led_b;
    struct gpiod_line* button;
    uint16_t blink_speed_ms;
    bool blink_enabled;
    uint8_t previous_value;
};

/********************************************************************************
 * @brief Runs thread continuously and prints the thread ID with set frequency.
 * 
 * @param args
 *        Void pointer to the input arguments. This pointer must be converted 
 *        to the correct pointer type, i.e. struct thread_args*, before usage.
 * @return
 *        A nullptr, since a void pointer has to be returned, desipte it not
 *        being used in this program.
 ********************************************************************************/
void* run_thread(void* arg) {
    struct thread_args* self= (struct thread_args*)(arg);

    while (1)
    {
        if (gpiod_line_event_detected(self->button, GPIOD_LINE_EDGE_RISING, &self->previous_value)) {
            self->blink_enabled = !self->blink_enabled;
            semaphore_reserve(SEMAPHORE_ID_PRINT);
            if (self->blink_enabled == true) {
                printf("Leds connected to pin %hu and %hu now enabled!\n", 
                       gpiod_line_offset(self->led_a), gpiod_line_offset(self->led_b));
            } else {
                printf("Leds connected to pin %hu and %hu now disabled!\n", 
                       gpiod_line_offset(self->led_a), gpiod_line_offset(self->led_b));
            }
            delay_ms(10);
            semaphore_release(SEMAPHORE_ID_PRINT);
        }

        if (self->blink_enabled == true) { // Om blinkning är aktiverat:
            gpiod_line_set_value(self->led_a, 1); // Tänd LED1.
            gpiod_line_set_value(self->led_b, 0); // Släck LED2.
            delay_ms(self->blink_speed_ms); // delay_ms.

            gpiod_line_set_value(self->led_a, 0); // Släck LED1.
            gpiod_line_set_value(self->led_b, 1); // Tänd LED2.
            delay_ms(self->blink_speed_ms); // delay.
        } else { // Annars (om blinkning ej är aktiverat).
            gpiod_line_set_value(self->led_a, 0);// Släck LED1.
            gpiod_line_set_value(self->led_b, 0);// Släck LED2.
        }
    }
    return 0; 
}

/********************************************************************************
 * @brief Creates two threads running the run_thread function:
 * 
 *        - t1: Thread ID = 1, delay time between prints = 500 ms.
 *        - t2: Thread ID = 2, delay time between prints = 1000 ms.
 * 
 *        The thread arguments are packed in structs, due to thread functions
 *        only being able to receive one input argument (a void pointer).
 *        
 *        The threads are created by calling the pthread function. The second
 *        argument is set to nullptr to make the thread run in default mode.
 *        
 *        The threads are syncronized by calling the pthread join function.
 *        Since the return value of the thread functions isn't needed in this 
 *        program, a nullptr is passed as second argument.
 ********************************************************************************/


int main(void) {

    struct gpiod_line* led1 = gpiod_line_new(17, GPIOD_LINE_DIRECTION_OUT);
    struct gpiod_line* led2 = gpiod_line_new(22, GPIOD_LINE_DIRECTION_OUT);
    struct gpiod_line* led3 = gpiod_line_new(23 ,GPIOD_LINE_DIRECTION_OUT);
    struct gpiod_line* led4 = gpiod_line_new(24 ,GPIOD_LINE_DIRECTION_OUT);

    struct gpiod_line* button1 = gpiod_line_new(27, GPIOD_LINE_DIRECTION_IN);
    struct gpiod_line* button2 = gpiod_line_new(25, GPIOD_LINE_DIRECTION_IN);

    struct thread_args args1 = { led1, led2, button1, 100, false, 0 };
    struct thread_args args2 = { led3, led4, button2, 500, false, 0 };
    pthread_t t1, t2; 

    pthread_create(&t1, 0, run_thread, &args1);
    pthread_create(&t2, 0, run_thread, &args2);
    pthread_join(t1, 0);
    pthread_join(t2, 0);
   
    return 0;
}