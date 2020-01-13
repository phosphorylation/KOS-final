struct console_buf{
  int buffer[256];
  int now;
  int head;
  int written;
  int fromException;
  kt_sem nelem;
  kt_sem nslots;
  kt_sem consoleWait;
  };
struct console_buf *cBuffer;

  void initialize_cBuf();
  void console_buf_read();
