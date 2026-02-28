/**
 * @file Scheduler.hpp
 * @author Planeson, Red Bird Racing
 * @brief Declaration of the Scheduler class template, for scheduling tasks on multiple MCP2515 instances
 * @version 1.2
 * @date 2026-02-28
 * @see Scheduler.tpp
 * @dir Scheduler @brief The Scheduler library contains the Scheduler class template, which manages the scheduling of tasks for multiple MCP2515 instances, allowing for periodic execution of functions based on a specified time interval and spin-wait threshold.
 */

#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "Enums.hpp"

/**
 * @brief Scheduler class template for scheduling tasks on multiple MCP2515 instances
 * Takes in function pointers to member functions of MCP2515 class,
 * and calls them at specified intervals, using a unified central ticker.
 *
 * @details The Scheduler class holds function pointers to tasks to be run on multiple MCP2515 instances.
 * Because how most MCP2515 are dedicated to a specific function (e.g., motor control, BMS, datalogger),
 * instead of the scheduler holding a queue of messages to send and a queue of read messages,
 * it holds function pointers to tasks that handle sending and receiving messages for each MCP2515.
 * The object wishing to send/receive messages holds their own MCP2515 object (reference).
 *
 * The Scheduler runs on a fixed period. When Scheduler.update() is called, it checks if a period has already passed.
 * If not, it checks if it is almost passed. If the time to the next cycle is less than SPIN_US, it spin-waits until the period is reached.
 * Otherwise, it returns immediately, allowing other non-scheduler tasks to run.
 * Once the time to fire is reached, it runs the tasks via the pointers in a round-robin fashion, going from one MCP2515 to another.
 *
 * With a modified mcp2515.h that doesn't directly use SPI.h, we can skip waiting for the SPI transaction to complete,
 * and instead work on compiling the next message while the previous SPI transaction is still ongoing.
 * Although it would be easier to only have a single queue (instead NUM_MCP2515 queues),
 * we give room for the CAN-bus to be busy on one MCP2515, while another MCP2515 can still send/receive messages,
 * i.e. we distribute the load across multiple CAN buses more evenly, instead of having one bus burst at one time
 *
 * In the rare case where the system is busy and misses more than one period, the scheduler will skip to the next period, preventing bursts.
 *
 * @tparam NUM_TASKS Number of tasks per MCP2515, choose highest of all, but keep as low as possible
 * @tparam NUM_MCP2515 Number of MCP2515 instances
 */
template <uint8_t NUM_TASKS, uint8_t NUM_MCP2515>
class Scheduler
{
    using TaskFn = void (*)();

public:
    Scheduler() = delete; /**< all arguments must be provided */
    Scheduler(uint32_t period_us_, uint32_t spin_threshold_us_, unsigned long (*const current_time_us)());
    // no need destructor, since no dynamic memory allocation, and won't destruct in the middle of the program anyway

    void update();
    void synchronize(unsigned long (*const current_time_us)());
    bool addTask(const McpIndex mcp_index, const TaskFn task, const uint8_t tick_interval);
    bool removeTask(const McpIndex mcp_index, const TaskFn task);

    /**
     * @brief Returns the period of the scheduler in microseconds.
     * @return The period in microseconds.
     */
    constexpr uint32_t getPeriodUs() const { return PERIOD_US; }

    /**
     * @brief Returns the number of cycles needed for a given interval in microseconds.
     * @param[in] interval_us The interval in microseconds.
     * @return The number of cycles needed.
     */
    constexpr uint32_t cyclesNeeded(const uint32_t interval_us) const { return interval_us / PERIOD_US; }

private:
    TaskFn tasks[NUM_MCP2515][NUM_TASKS];          /**< Array of tasks, sorted by each MCP2515. */
    uint8_t task_ticks[NUM_MCP2515][NUM_TASKS];    /**< Period (in ticks) of each function, 1 is fire every tick, 0 is disabled. */
    uint8_t task_counters[NUM_MCP2515][NUM_TASKS]; /**< Counter to hold firing for n ticks, "how many ticks left before firing?". */
    uint8_t task_cnt[NUM_MCP2515];                 /**< Array of number of tasks per MCP2515. */
    const uint32_t PERIOD_US;                      /**< Period (tick length). */
    const uint32_t SPIN_US;                        /**< Threshold to switch from letting non-scheduler task in loop() run, to spin-locking (to ensure on time firing). */
    uint32_t last_fire_us;                         /**< Last time scheduler fired, overridden if missed more than one period. */
    unsigned long (*const CURRENT_TIME_US)();      /**< Function pointer to a function returning the current time in microseconds. */

    inline void runTasks();
};

#include "Scheduler.tpp"

#endif // SCHEDULER_HPP