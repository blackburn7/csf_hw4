
/*
Assignment 4

Atticus Colwell
acolwel2@jh.edu

Matthew Blackburn
mblackb8@jh.edu
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int compare_i64(const void *left_, const void *right_) {
  int64_t left = *(int64_t *)left_;
  int64_t right = *(int64_t *)right_;
  if (left < right) return -1;
  if (left > right) return 1;
  return 0;
}

void seq_sort(int64_t *arr, size_t begin, size_t end) {
  size_t num_elements = end - begin;
  qsort(arr + (begin), num_elements, sizeof(int64_t), compare_i64);
}

// Merge the elements in the sorted ranges [begin, mid) and [mid, end),
// copying the result into temparr.
void merge(int64_t *arr, size_t begin, size_t mid, size_t end, int64_t *temparr) {
  int64_t *endl = arr + mid;
  int64_t *endr = arr + end;
  int64_t *left = arr + begin, *right = arr + mid, *dst = temparr;

  for (;;) {
    int at_end_l = left >= endl;
    int at_end_r = right >= endr;

    if (at_end_l && at_end_r) break;

    if (at_end_l)
      *dst++ = *right++;
    else if (at_end_r)
      *dst++ = *left++;
    else {
      int cmp = compare_i64(left, right);
      if (cmp <= 0)
        *dst++ = *left++;
      else
        *dst++ = *right++;
    }
  }
}

void fatal(const char *msg) __attribute__ ((noreturn));

void fatal(const char *msg) {
  fprintf(stderr, "Error: %s\n", msg);
  exit(1);
}

void merge_sort(int64_t *arr, size_t begin, size_t end, size_t threshold) {
  assert(end >= begin);
  size_t size = end - begin;

  if (size <= threshold) {
    seq_sort(arr, begin, end);
    return;
  }

  // recursively sort halves in parallel
  size_t mid = begin + size/2;

  int wstatus1;
  int wstatus2;

  pid_t pid1 = fork();
  if (pid1 == -1) {
    // handle pid error
    fatal("error");
  } else if (pid1 == 0) {
    // child process
    merge_sort(arr, begin, mid, threshold);
    exit(0);
  }

  pid_t pid2 = fork();
  if (pid2 == -1) {
    // wait for child 1 to finish if child 2 fails
    waitpid(pid2, &wstatus1, 0);
    fatal("error");
  } else if (pid2 == 0) {
    merge_sort(arr, mid, end, threshold);
    exit(0);
  }

  // have parent wait for child completion
  pid_t actual_pid1 = waitpid(pid1, &wstatus1, 0);
  pid_t actual_pid2 = waitpid(pid2, &wstatus2, 0);

  if (actual_pid1 == -1) {
    // handle waitpid failure
    fatal("error");
  }
  if (actual_pid2 == -1) {
    fatal("error");
  }

  // subprocess error handling
  if (!WIFEXITED(wstatus1)) {
    // handle subprocess crash
    fatal("error");
  }
  if (WEXITSTATUS(wstatus1) != 0) {
    // handle subprocess returns non-zero exit code
    fatal("error");
  }

    if (!WIFEXITED(wstatus2)) {
    // handle subprocess crash
    fatal("error");
  }
  if (WEXITSTATUS(wstatus2) != 0) {
    // handle subprocess returns non-zero exit code
    fatal("error");
  }










  // allocate temp array now, so we can avoid unnecessary work
  // if the malloc fails
  int64_t *temp_arr = (int64_t *) malloc(size * sizeof(int64_t));
  if (temp_arr == NULL)
    fatal("malloc() failed");

  // child processes completed successfully, so in theory
  // we should be able to merge their results
  merge(arr, begin, mid, end, temp_arr);

  // copy data back to main array
  for (size_t i = 0; i < size; i++)
    arr[begin + i] = temp_arr[i];

  // now we can free the temp array
  free(temp_arr);

  // success!
}

int main(int argc, char **argv) {
  // check for correct number of command line arguments
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <filename> <sequential threshold>\n", argv[0]);
    fatal("error");
  }

  // process command line arguments
  const char *filename = argv[1];
  char *end;
  size_t threshold = (size_t) strtoul(argv[2], &end, 10);
  if (end != argv[2] + strlen(argv[2])) {
    fprintf(stderr, "Error: threshold is invalid");
    fatal("error");
  }

  // open the file
  int fd = open(filename, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Error: failed to open file");
    fatal("error");
  }

  // use fstat to determine the size of the file
  struct stat statbuf;
  int rc = fstat(fd, &statbuf);
  if (rc != 0) {
    fprintf(stderr, "Error: no stat");
    fatal("error");
  }
  size_t f_size_in_bytes = statbuf.st_size;

  // map the file into memory using mmap
  int64_t *data = mmap(NULL, f_size_in_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
  if (data == MAP_FAILED) {
    fatal("error");
  }

  // sort the data!
  merge_sort(data, 0, f_size_in_bytes / sizeof(int64_t), threshold);

  // unmap and close the file
  if (munmap(data, f_size_in_bytes) == -1) {
    // handle unmapping error
    fatal("error");
  }

  // TODO: exit with a 0 exit code if sort was successful
  return 0;
}
