#ifndef __TFMAP_H__
#define __TFMAP_H__

#ifndef SINGLE
int start_tf_map(const char *file_path, int flag_daemon);
#endif

void *map_thread_process(void *arg);

#endif
