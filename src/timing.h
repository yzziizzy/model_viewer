



typedef struct QueryQueue {
	GLuint qids[6];
	int head, used;
} QueryQueue;




double getCurrentTime();


double timeSince(double past);

void query_queue_init(QueryQueue* q);


void query_queue_start(QueryQueue* q);


void query_queue_stop(QueryQueue* q);


int query_queue_try_result(QueryQueue* q, uint64_t* time);


int tryQueryTimer(GLuint id, uint64_t* time);

 
