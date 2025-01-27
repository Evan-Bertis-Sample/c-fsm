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

// Enable/disable debug logging for the internal FSM implementation
#define FSM_DEBUG 1

/**========================================================================
 *                           Types and Functions
 *========================================================================**/

struct fsm;
typedef void (*fsm_state_fn)(struct fsm *fsm, void *context);
typedef bool (*fsm_transition_predicate_fn)(struct fsm *fsm, void *context);
typedef size_t fsm_size_t;

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

/// @brief Describes a transition in the FSM
/// @note This is an internal structure used to store transitions
///      in the FSM, do not use this directly
typedef struct __fsm_transition {
    fsm_state_t *from;
    fsm_state_t *to;
    fsm_predicate_group_t *predicates;
} __fsm_transition_t;

typedef struct fsm {
    /// @brief Context passed to state functions, could be anything
    void *context;
    /// @brief Array of states in the FSM -- strong reference, we own this memory
    fsm_state_t *states;
    /// @brief Array of transitions in the FSM -- strong reference, we own this memory
    __fsm_transition_t *transitions;

    fsm_alloc_fn __alloc_fn;
    fsm_dealloc_fn __dealloc_fn;

    size_t __context_size;
    fsm_size_t __state_count;
    fsm_size_t __transition_count;
    fsm_size_t __current_state_idx;
    bool __is_running;
} fsm_t;

fsm_t *fsm_create(fsm_alloc_fn alloc_fn, fsm_dealloc_fn dealloc_fn);
void fsm_run(fsm_t *fsm);
void fsm_stop(fsm_t *fsm);
void fsm_destroy(fsm_t *fsm);

/*
 * Note about the fsm_add_xxx functions:
 * Whenever you add a state or transition to the FSM, the FSM will take ownership of the memory
 * and will free it when the FSM is destroyed. This means that you should not free the memory.
 * Modifying whatever fsm_state_t or fsm_predicate_group_t you pass to the fsm_add_xxx functions
 * after you pass them to the FSM will not have any effect on the FSM.
 * Instead, a copy of the data will be made and stored in the FSM.
 */

void fsm_add_state(fsm_t *fsm, fsm_state_t state);
void fsm_add_transition(fsm_t *fsm, char *from, char *to, fsm_predicate_group_t predicates);
void fsm_add_transition_from_all(fsm_t *fsm, char *to, fsm_predicate_group_t predicates);
void fsm_add_transition_to_all(fsm_t *fsm, char *from, fsm_predicate_group_t predicates);

inline fsm_size_t fsm_state_count(fsm_t *fsm) { return fsm->__state_count; }
inline fsm_size_t fsm_transition_count(fsm_t *fsm) { return fsm->__transition_count; }
inline char *fsm_current_state(fsm_t *fsm) { return fsm->states[fsm->__current_state_idx].name; }
inline bool fsm_is_running(fsm_t *fsm) { return fsm->__is_running; }

#define FSM_CREATE() fsm_create(malloc, free)
#define FSM_GET_CONTEXT(fsm, type) (type *)fsm->context

fsm_size_t __fsm_state_index(fsm_t *fsm, char *name);
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