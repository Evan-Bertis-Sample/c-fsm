/**========================================================================
 * ?                                ABOUT
 * @author         :  Evan Bertis-Sample
 * @email          :  esample21@gmail.com
 * @repo           :  https://github.com/evan-bertis-sample/c-fsm.git
 * @createdOn      :  1/26/2025
 * @description    :  A Finite State Machine (FSM) implementation in C
 *========================================================================**/

#ifndef __FSM_H__
#define __FSM_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>

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
    void *context;

    fsm_state_fn on_enter;
    fsm_state_fn on_exit;
} fsm_state_t;

fsm_state_t *fsm_state_create(fsm_alloc_fn alloc_fn, char *name, void *context, fsm_state_fn on_enter, fsm_state_fn on_exit);
void fsm_state_destroy(fsm_dealloc_fn dealloc_fn, fsm_state_t *state);

#define FSM_STATE_CREATE(...) fsm_state_create(malloc, __VA_ARGS__)
#define FSM_STATE_DESTROY(state) fsm_state_destroy(free, state)

/// @brief Describes a transition in the FSM
typedef struct fsm_predicate_group {
    fsm_transition_predicate_fn *predicates;
    fsm_size_t predicate_count;
} fsm_predicate_group_t;

fsm_predicate_group_t *fsm_predicate_group_create(fsm_alloc_fn alloc_fn, fsm_transition_predicate_fn *predicates, fsm_size_t predicate_count);
void fsm_predicate_group_destroy(fsm_dealloc_fn dealloc_fn, fsm_predicate_group_t *group);

#define FSM_PREDICATE_GROUP_CREATE(...) fsm_predicate_group_create(malloc, __VA_ARGS__, sizeof(__VA_ARGS__) / sizeof(fsm_transition_predicate_fn))
#define FSM_PREDICATE_GROUP_DESTROY(group) fsm_predicate_group_destroy(free, group)

/// @brief Describes a transition in the FSM
/// @note This is an internal structure used to store transitions
///      in the FSM, do not use this directly
typedef struct __fsm_transition {
    fsm_state *from;
    fsm_state *to;
    fsm_predicate_group_t *predicates;
} __fsm_transition_t;

typedef struct fsm {
    fsm_state *states;
    fsm_size_t __state_count;

    __fsm_transition_t *transitions;
    fsm_size_t __transition_count;

    fsm_size_t __current_state_idx;

    bool is_running;
} fsm_t;

void fsm_init(fsm_t *fsm, fsm_state *states, fsm_size_t state_count, fsm_predicate_group_t *transitions, fsm_size_t transition_count);
void fsm_run(fsm_t *fsm);
void fsm_stop(fsm_t *fsm);

void fsm_add_state(fsm_t *fsm, fsm_state *state);
void fsm_add_transition(fsm_t *fsm, fsm_predicate_group_t *transition);
void fsm_add_transition(fsm_t *fsm, char *from, char *to, fsm_transition_predicate_fn *predicates, fsm_size_t predicate_count);

inline fsm_size_t fsm_state_count(fsm_t *fsm) { return fsm->__state_count; }
inline fsm_size_t fsm_transition_count(fsm_t *fsm) { return fsm->__transition_count; }

fsm_size_t __fsm_state_index(fsm_t *fsm, char *name);
fsm_size_t __fsm_transition_index(fsm_t *fsm, char *from, char *to);

#ifdef FSM_IMPL

#endif  // FSM_IMPL

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __FSM_H__