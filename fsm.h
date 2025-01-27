/**========================================================================
 *
 *                 ______         _______     _______..___  ___.
 *                /      |       |   ____|   /       ||   \/   |
 *               |  ,----' ______|  |__     |   (----`|  \  /  |
 *               |  |     |______|   __|     \   \    |  |\/|  |
 *               |  `----.       |  |    .----)   |   |  |  |  |
 *                \______|       |__|    |_______/    |__|  |__|
 *
 *                                  c-fsm
 *        A single-header, easy-to-use, and lightweight FSM library in C
 *
 * ?                                ABOUT
 * @author         :  Evan Bertis-Sample
 * @email          :  esample21@gmail.com
 * @repo           :  https://github.com/evan-bertis-sample/c-fsm.git
 * @createdOn      :  1/26/2025
 * @description    :  A Finite State Machine (FSM) implementation in C
 *
 * ?                                USAGE
 * 1. Include "fsm.h" in your project
 * 2. If you want to use the default implementation, define FSM_IMPL before
 *    including "fsm.h"
 * 3. Create a new FSM using fsm_create
 * 4. Add states and transitions to the FSM using fsm_add_state and
 *    fsm_add_transition
 * 5. Run the FSM using fsm_run
 * 6. Stop the FSM using fsm_stop
 * 7. Destroy the FSM using fsm_destroy
 *
 * For exampe usage please refer to:
 * https://github.com/Evan-Bertis-Sample/c-fsm/tree/main/examples
 *
 *========================================================================**/

#ifndef __FSM_H__
#define __FSM_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Only used for bool & size_t
// If you don't have stdbool.h or stdint.h, you can define these yourself
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Enable/disable debug logging for the internal FSM implementation
#ifndef FSM_DEBUG
#define FSM_DEBUG 1
#endif  // FSM_DEBUG

/**========================================================================
 *                           Types and Functions
 *========================================================================**/

/// @brief typedef for size_t, just in case it's not defined
typedef size_t fsm_size_t;

/// @brief typedef for bool, just in case it's not defined
typedef bool fsm_bool;

/// @brief Forward declaration of the FSM structure
struct fsm;

/// @brief Function pointer types for the state functions
typedef void (*fsm_state_fn)(struct fsm *fsm, void *context);

/// @brief Function pointer type for the transition predicates, which take an fsm, a context, and return a fsm_bool
typedef fsm_bool (*fsm_transition_predicate_fn)(struct fsm *fsm, void *context);

// typedef allocator/deallocator functions
typedef void *(*fsm_alloc_fn)(size_t size);
typedef void (*fsm_dealloc_fn)(void *ptr);

/// @brief Describes a state in the FSM
typedef struct fsm_state {
    char *name;
    fsm_state_fn on_enter;
    fsm_state_fn on_update;
    fsm_state_fn on_exit;
} fsm_state_t;

/// @brief Describes a transition in the FSM
typedef struct fsm_predicate_group {
    fsm_transition_predicate_fn *predicates;
    fsm_size_t predicate_count;
} fsm_predicate_group_t;

#define FSM_PREDICATE_GROUP(...)                                                                                      \
    (fsm_predicate_group_t) {                                                                                         \
        .predicates = (fsm_transition_predicate_fn[]){__VA_ARGS__},                                                   \
        .predicate_count = sizeof((fsm_transition_predicate_fn[]){__VA_ARGS__}) / sizeof(fsm_transition_predicate_fn) \
    }

/// @brief Describes a transition in the FSM
/// @note This is an internal structure used to store transitions
///      in the FSM, do not use this directly
typedef struct __fsm_transition {
    fsm_state_t *from;
    fsm_state_t *to;
    fsm_predicate_group_t *predicates;
} __fsm_transition_t;

/// @brief Describes a Finite State Machine
/// @note This is the main structure used to store the FSM
/// @note Please interact with the FSM using the functions provided
typedef struct fsm {
    /// @brief Context passed to state functions, could be anything
    void *context;

    /// @brief Array of states in the FSM -- strong reference, we own this memory
    fsm_state_t *states;

    /// @brief Array of transitions in the FSM -- strong reference, we own this memory
    __fsm_transition_t *transitions;

    /// @brief Memory allocation function
    fsm_alloc_fn __alloc_fn;
    fsm_dealloc_fn __dealloc_fn;

    fsm_size_t __context_size;
    fsm_size_t __state_count;
    fsm_size_t __transition_count;
    fsm_size_t __current_state_idx;

    fsm_bool __is_running;
} fsm_t;

/// @brief Creates a new FSM, starting with no states or transitions
/// @param alloc_fn Memory allocation function
/// @param dealloc_fn Memory deallocation function
/// @param context Context passed to state functions
/// @param context_size Size of the context
/// @return A new FSM, allocated using alloc_fn
fsm_t *fsm_create(fsm_alloc_fn alloc_fn, fsm_dealloc_fn dealloc_fn, void *context, size_t context_size);

/// @brief Runs the FSM, starting from the first state
/// @param fsm The FSM to run
void fsm_run(fsm_t *fsm);

/// @brief Stops the FSM, preventing it from running
/// @param fsm The FSM to stop
void fsm_stop(fsm_t *fsm);

/// @brief Destroys the FSM, freeing all memory associated with it
/// @param fsm The FSM to destroy
void fsm_destroy(fsm_t *fsm);

/// @brief Sets the current state of the FSM
/// @param fsm The FSM to set the state of
/// @param state_name The name of the state to set
/// @note This shouldn't be used to change the state of the FSM, use transitions instead
///       This is mainly so you can set the initial state of the FSM, which defaults to the first state
void fsm_set_state(fsm_t *fsm, char *state_name);

/*
 * Note about the fsm_add_xxx functions:
 * Whenever you add a state or transition to the FSM, the FSM will take ownership of the memory
 * and will free it when the FSM is destroyed. This means that you should not free the memory.
 * Modifying whatever fsm_state_t or fsm_predicate_group_t you pass to the fsm_add_xxx functions
 * after you pass them to the FSM will not have any effect on the FSM.
 * Instead, a copy of the data will be made and stored in the FSM.
 */

/// @brief Adds a state to the FSM
/// @param fsm The FSM to add the state to
/// @param state The state to add
/// @note The FSM will take ownership of the memory of the state, making a copy of it
void fsm_add_state(fsm_t *fsm, fsm_state_t state);

/// @brief Adds a transition to the FSM
/// @param fsm The FSM to add the transition to
/// @param from The name of the state to transition from
/// @param to The name of the state to transition to
/// @param predicates The predicates that must be true for the transition to occur
/// @note The FSM will take ownership of the memory of the predicates, making a copy of it
void fsm_add_transition(fsm_t *fsm, char *from, char *to, fsm_predicate_group_t predicates);

/// @brief Adds a transition from all states to a specific state
/// @param fsm The FSM to add the transition to
/// @param to The name of the state to transition to
/// @param predicates The predicates that must be true for the transition to occur
/// @note The FSM will take ownership of the memory of the predicates, making a copy of it
void fsm_add_transition_from_all(fsm_t *fsm, char *to, fsm_predicate_group_t predicates);

/// @brief Adds a transition to all states from a specific state
/// @param fsm The FSM to add the transition to
/// @param from The name of the state to transition from
/// @param predicates The predicates that must be true for the transition to occur
/// @note The FSM will take ownership of the memory of the predicates, making a copy of it
void fsm_add_transition_to_all(fsm_t *fsm, char *from, fsm_predicate_group_t predicates);

/// @brief Gets the number of states in the FSM
/// @param fsm The FSM to get the state count of
inline fsm_size_t fsm_state_count(fsm_t *fsm) { return fsm->__state_count; }

/// @brief Gets the number of transitions in the FSM
/// @param fsm The FSM to get the transition count of
inline fsm_size_t fsm_transition_count(fsm_t *fsm) { return fsm->__transition_count; }

/// @brief Gets the name of the current state of the FSM
/// @param fsm The FSM to get the current state of
inline char *fsm_current_state(fsm_t *fsm) { return fsm->states[fsm->__current_state_idx].name; }

/// @brief Checks if the FSM is running
/// @param fsm The FSM to check if it is running
inline fsm_bool fsm_is_running(fsm_t *fsm) { return fsm->__is_running; }

/// @brief Creates a new FSM given a context, using malloc and free as alloc/dealloc functions
#define FSM_CREATE(context) fsm_create(malloc, free, context, sizeof(context))

/// @brief Gets the context of the FSM as a specific type
/// @param fsm The FSM to get the context of
/// @param type The type to cast the context to
#define FSM_GET_CONTEXT(fsm, type) (type *)fsm->context

/// @brief (private_fn) Gets the index of a state in the FSM given its name
/// @param fsm The FSM to get the state index from
/// @param name The name of the state to get the index of
/// @return The index of the state, or -1 if the state does not exist
fsm_size_t __fsm_state_index(fsm_t *fsm, char *name);

/// @brief (private_fn) Gets the index of a transition in the FSM given the names of the from and to states
/// @param fsm The FSM to get the transition index from
/// @param from The name of the state to transition from
/// @param to The name of the state to transition to
/// @return The index of the transition, or -1 if the transition does not exist
fsm_size_t __fsm_transition_index(fsm_t *fsm, char *from, char *to);

/**========================================================================
 *                           Macros and Logging
 *========================================================================**/

#define FSM_LINE __LINE__
#define FSM_STR_HELPER(x) #x
#define FSM_STR(x) FSM_STR_HELPER(x)

#define FSM_ERROR_COLOR "\033[0;31m"
#define FSM_RESET_COLOR "\033[0m"

#ifdef FSM_DEBUG
#define FSM_DEUBG_PREFIX "[fsm.h:" FSM_STR(FSM_LINE) "] "
#define FSM_LOG(fmt, ...) printf(FSM_DEUBG_PREFIX fmt, ##__VA_ARGS__)
#define FSM_LOG_ERROR(fmt, ...) fprintf(stderr, FSM_ERROR_COLOR FSM_DEUBG_PREFIX FSM_RESET_COLOR fmt, ##__VA_ARGS__)
#else
#define FSM_LOG(fmt, ...) void(0)
#define FSM_LOG_ERROR(fmt, ...) void(0)
#endif  // FSM_DEBUG

/**========================================================================
 *                           Implementation
 *========================================================================**/

#ifdef FSM_IMPL

#endif  // FSM_IMPL

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __FSM_H__