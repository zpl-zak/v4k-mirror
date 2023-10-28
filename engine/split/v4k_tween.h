typedef struct tween_keyframe_t {
	int easing_mode;
	float t;
	vec3 v;
} tween_keyframe_t;

typedef struct tween_t {
	array(tween_keyframe_t) keyframes;

	vec3 result;
	float time;
	float duration;
} tween_t;

API tween_t tween();
API float     tween_update(tween_t *tw, float dt);
API void      tween_reset(tween_t *tw);
API void    tween_destroy(tween_t *tw);

API void tween_keyframe_set(tween_t *tw, float t, int easing_mode, vec3 v);
API void tween_keyframe_unset(tween_t *tw, float t);
