tween_t tween() {
	tween_t tw = {0};
	return tw;
}

float tween_update(tween_t *tw, float dt) {
	if (!array_count(tw->keyframes)) return 0.0f;

	for (size_t i = 0; i < array_count(tw->keyframes) - 1; ++i) {
        tween_keyframe_t *kf1 = &tw->keyframes[i];
        tween_keyframe_t *kf2 = &tw->keyframes[i + 1];
        if (tw->time >= kf1->t && tw->time <= kf2->t) {
            float localT = (tw->time - kf1->t) / (kf2->t - kf1->t);
            float easedT = ease(localT, kf1->easing_mode);
            tw->result = mix3(kf1->v, kf2->v, easedT);
            break;
        }
    }

	float done = (tw->time / tw->duration);
	tw->time += dt;
	return clampf(done, 0.0f, 1.0f);
}

void tween_reset(tween_t *tw) {
	tw->time = 0.0f;
}

void tween_destroy(tween_t *tw) {
	tween_t tw_ = {0};
	array_free(tw->keyframes);
	*tw = tw_;
}

static inline
int tween_comp_keyframes(const void *a, const void *b) {
    float t1 = ((const tween_keyframe_t*)a)->t;
    float t2 = ((const tween_keyframe_t*)b)->t;
    return (t1 > t2) - (t1 < t2);
}

void tween_keyframe_set(tween_t *tw, float t, int mode, vec3 v) {
	tween_keyframe_t keyframe = { mode, t, v };
	array_push(tw->keyframes, keyframe);
	array_sort(tw->keyframes, tween_comp_keyframes);
	tw->duration = array_back(tw->keyframes)->t;
}

void tween_keyframe_unset(tween_t *tw, float t) { /*@todo: untested*/
	int id = -1;
	for (int i = 0; i < array_count(tw->keyframes); i++) {
		if (tw->keyframes[i].t == t) {
			id = i;
			break;
		}
	}

	if (id == -1) return;
	array_erase_slow(tw->keyframes, id);
	tw->duration = array_back(tw->keyframes)->t;
}
