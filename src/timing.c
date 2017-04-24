
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <stdint.h>

#include "timing.h" 

double getCurrentTime() {
	double now;
	struct timespec ts;
	static double offset = 0;
	
	// CLOCK_MONOTONIC_RAW is linux-specific.
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	
	now = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
	if(offset == 0) offset = now;
	
	return now - offset;
}

double timeSince(double past) {
	double now = getCurrentTime();
	return now - past;
}

void query_queue_init(QueryQueue* q) {
	glGenQueries(6, q->qids);
	q->head = 0;
	q->used = 0;
}

void query_queue_start(QueryQueue* q) {
	if(q->used < 6) {
		glBeginQuery(GL_TIME_ELAPSED, q->qids[q->head]);
		q->head = (q->head + 1) % 6;
		q->used++;
	}
	else {
		fprintf(stderr, "query queue exhausted \n");
	}
}

void query_queue_stop(QueryQueue* q) {
	glEndQuery(GL_TIME_ELAPSED);
}

int query_queue_try_result(QueryQueue* q, uint64_t* time) {
	uint64_t p;
	int tail;
	
	if(q->used == 0) {
		return 2;
	}
	
	tail = (q->head - q->used + 6) % 6; 
	
	glGetQueryObjectui64v(q->qids[tail], GL_QUERY_RESULT_AVAILABLE, &p);
	if(GL_FALSE == p) {
		return 1; // the query isn't ready yet
	}
	
	glGetQueryObjectui64v(q->qids[tail], GL_QUERY_RESULT, time); 
	q->used--;
	
	return 0;
}

int tryQueryTimer(GLuint id, uint64_t* time) {
	uint64_t p;
	
	glGetQueryObjectui64v(id, GL_QUERY_RESULT_AVAILABLE, &p);
	if(GL_TRUE == p) { 
		glGetQueryObjectui64v(id, GL_QUERY_RESULT, time); 
		return 0;
	}
	
	return 1;
}
