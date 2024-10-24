#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define DEVICE_NAME "/dev/video0"
#define BUFFER_COUNT 4

struct buffer {
  void *start;
  size_t length;
};

int main() {
  int fd = open(DEVICE_NAME, O_RDWR);
  if (fd == -1) {
    perror("Opening video device");
    return EXIT_FAILURE;
  }

  struct v4l2_capability cap;
  if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
    perror("Querying Capabilities");
    close(fd);
    return EXIT_FAILURE;
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf(stderr, "The device does not support video capture.\n");
    close(fd);
    return EXIT_FAILURE;
  }

  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = 640;
  fmt.fmt.pix.height = 480;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

  if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
    perror("Setting Pixel Format");
    close(fd);
    return EXIT_FAILURE;
  }

  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));
  req.count = BUFFER_COUNT;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
    perror("Requesting Buffer");
    close(fd);
    return EXIT_FAILURE;
  }

  struct buffer buffers[BUFFER_COUNT];
  for (int i = 0; i < BUFFER_COUNT; ++i) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
      perror("Querying Buffer");
      close(fd);
      return EXIT_FAILURE;
    }

    buffers[i].length = buf.length;
    buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, buf.m.offset);
    if (buffers[i].start == MAP_FAILED) {
      perror("Mapping Buffer");
      close(fd);
      return EXIT_FAILURE;
    }
  }

  for (int i = 0; i < BUFFER_COUNT; ++i) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
      perror("Queueing Buffer");
      close(fd);
      return EXIT_FAILURE;
    }
  }

  if (ioctl(fd, VIDIOC_STREAMON, &buf.type) == -1) {
    perror("Starting Capture");
    close(fd);
    return EXIT_FAILURE;
  }

  struct v4l2_buffer buf;
  memset(&buf, 0, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
    perror("Dequeuing Buffer");
    close(fd);
    return EXIT_FAILURE;
  }

  // Here you can process the image in buffers[buf.index].start
  printf("Captured frame size: %d bytes\n", buf.bytesused);

  if (ioctl(fd, VIDIOC_STREAMOFF, &buf.type) == -1) {
    perror("Stopping Capture");
  }

  for (int i = 0; i < BUFFER_COUNT; ++i) {
    munmap(buffers[i].start, buffers[i].length);
  }

  close(fd);
  return EXIT_SUCCESS;
}
