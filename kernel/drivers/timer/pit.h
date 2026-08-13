#ifndef KERNEL_DRIVERS_TIMER_PIT_H
#define KERNEL_DRIVERS_TIMER_PIT_H

#include <stdint.h>

#define PIT_FREQUENCY 1000  // in Hz

/// @brief Initializes the Programmable Interval Timer (PIT) with the desired frequency.
///
/// The PIT operates at a base frequency of 1,193,182 Hz. Since the PIT uses an
/// integer divisor, the requested frequency cannot always be represented exactly.
/// The divisor is therefore rounded down, making the actual interrupt frequency
/// an approximation of the requested frequency.
///
/// @param frequency The desired timer interrupt frequency in Hz.
void pit_init(uint32_t frequency);

/// @brief Increments the number of timer ticks.
void pit_tick(void);

/// @brief Returns the number of timer ticks since the PIT was initialized.
///
/// @return The current timer tick count.
uint32_t pit_get_ticks(void);

/// @brief Returns the elapsed time in milliseconds based on PIT ticks.
///
/// Converts the number of PIT ticks returned by pit_get_ticks() to milliseconds using PIT_FREQUENCY.
///
/// @return Elapsed time in milliseconds.
uint64_t timer_ms(void);

#endif  // KERNEL_DRIVERS_TIMER_PIT_H
